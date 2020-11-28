// steam_cpp_wrapper_custom.cpp : Defines the exported functions for the DLL.
//

#include "pch.h"
#include "framework.h"

#include <unordered_set>
#include <memory>
#include <string>
#include <mutex>

#include <sdk/public/steam/steam_api.h>
#include <sdk/public/steam/steam_gameserver.h>

extern "C" {
#include "steam_cpp_wrapper_custom.h"
}



namespace util {


    template<typename... Arguments>
    inline void dbgf(const char* s, Arguments&&... args)
    {
        std::string str(1, '\0');

        int n = snprintf(&str[0], 0, s, std::forward<Arguments>(args)...);

        n += 1;
        str.resize(n);
        snprintf(&str[0], n, s, std::forward<Arguments>(args)...);
        OutputDebugStringA(str.c_str());
    }

}













struct Achievement_t
{
    int m_eAchievementID;
    const char* m_pchAchievementID;
    char m_rgchName[128];
    char m_rgchDescription[256];
    bool m_bAchieved;
    int m_iIconImage;
};


class CSteamAchievements;

// globals
// auto destroyed when dll unloads 
std::unique_ptr<uint64_t>           g_currentAppID;
std::unique_ptr<CSteamAchievements> g_steamAchievements;




class CSteamAchievements
{


private:
    int64 m_iAppID; // Our current AppID
    Achievement_t* m_pAchievements; // Achievements data
    int m_iNumAchievements; // The number of Achievements
    bool m_bInitialized; // Have we called Request stats and received the callback?

    std::recursive_mutex            pendings_mtx;
    std::unordered_set<const char*> pendings;


    void setPendingAchievements()
    {
        std::unique_lock<decltype(pendings_mtx)> lock(pendings_mtx);

        if (pendings.empty())
            return;

        for (auto id : pendings)
            SetAchievement(id);
    }

public:

    CSteamAchievements(/*Achievement_t* Achievements, int NumAchievements*/) :
        m_iAppID(0),
        m_bInitialized(false),
        m_CallbackUserStatsReceived(this, &CSteamAchievements::OnUserStatsReceived),
        m_CallbackUserStatsStored(this, &CSteamAchievements::OnUserStatsStored),
        m_CallbackAchievementStored(this, &CSteamAchievements::OnAchievementStored)
    {
        m_iAppID = SteamUtils()->GetAppID();
        //m_pAchievements = Achievements;
        //m_iNumAchievements = NumAchievements;
        RequestStats();
        //MessageBoxA(0, 0, 0, 0);
    }

    ~CSteamAchievements() {}


    bool RequestStats()
    {
        // Is Steam loaded? If not we can't get stats.
        if (NULL == SteamUserStats() || NULL == SteamUser())
        {
            OutputDebugStringA("## RequestStats... steam not loaded\n");
            return false;
        }
        // Is the user logged on?  If not we can't get stats.
        if (!SteamUser()->BLoggedOn())
        {
            OutputDebugStringA("## RequestStats... steam user not logged on\n");
            return false;
        }

        auto ret = SteamUserStats()->RequestCurrentStats();
        
        util::dbgf("## RequestStats, ret: %d... ok\n", ret);

        // Request user stats.
        return ret;
    }

    bool SetAchievement(const char* ID)
    {
        // Have we received a call back from Steam yet?
        if (!m_bInitialized) {
            {
                std::unique_lock<decltype(pendings_mtx)> lock(pendings_mtx);
                pendings.emplace(ID);
            }
            OutputDebugStringA("## SetAchievement pending push front in progress...not received a callback from Steam\n");
            return false; 
        }        

        auto ret = SteamUserStats()->SetAchievement(ID);
        util::dbgf("## SetAchievementp.. received a call back from Steam, achievement set? : %d\n", ret);
        return SteamUserStats()->StoreStats();
    }


    STEAM_CALLBACK(CSteamAchievements, OnUserStatsReceived, UserStatsReceived_t,
        m_CallbackUserStatsReceived);
    STEAM_CALLBACK(CSteamAchievements, OnUserStatsStored, UserStatsStored_t,
        m_CallbackUserStatsStored);
    STEAM_CALLBACK(CSteamAchievements, OnAchievementStored,
        UserAchievementStored_t, m_CallbackAchievementStored);

};


void CSteamAchievements::OnUserStatsReceived(UserStatsReceived_t* pCallback)
{
    // we may get callbacks for other games' stats arriving, ignore them
    if (m_iAppID != pCallback->m_nGameID) {
        OutputDebugStringA("## OnUserStatsReceived, app id not good\n");
        return;
    }

    if (k_EResultOK != pCallback->m_eResult)
    {
        char buffer[128];
        _snprintf(buffer, 128, "## RequestStats - failed, %d\n", pCallback->m_eResult);
        OutputDebugStringA(buffer);
        return;
    }


    OutputDebugStringA("## Received stats and achievements from Steam\n");
    m_bInitialized = true;

    setPendingAchievements();

    // load achievements
    /*for (int iAch = 0; iAch < m_iNumAchievements; ++iAch)
    {
        Achievement_t& ach = m_pAchievements[iAch];

        SteamUserStats()->GetAchievement(ach.m_pchAchievementID, &ach.m_bAchieved);
        _snprintf(ach.m_rgchName, sizeof(ach.m_rgchName), "%s",
            SteamUserStats()->GetAchievementDisplayAttribute(ach.m_pchAchievementID,
                "name"));
        _snprintf(ach.m_rgchDescription, sizeof(ach.m_rgchDescription), "%s",
            SteamUserStats()->GetAchievementDisplayAttribute(ach.m_pchAchievementID,
                "desc"));
    }*/
}

void CSteamAchievements::OnUserStatsStored(UserStatsStored_t* pCallback)
{
    // we may get callbacks for other games' stats arriving, ignore them
    if (m_iAppID == pCallback->m_nGameID)
    {
        if (k_EResultOK == pCallback->m_eResult)
        {
            OutputDebugStringA("## Stored stats for Steam\n");
        }
        else
        {
            char buffer[128];
            _snprintf(buffer, 128, "## StatsStored - failed, %d\n", pCallback->m_eResult);
            OutputDebugStringA(buffer);
        }
    }
}

void CSteamAchievements::OnAchievementStored(UserAchievementStored_t* pCallback)
{
    // we may get callbacks for other games' stats arriving, ignore them
    if (m_iAppID == pCallback->m_nGameID)
    {
        OutputDebugStringA("## Stored Achievement for Steam\n");
    }
}











extern "C" {
    __declspec(dllexport)
    bool c_SteamAPI_Init(void)
    {
        bool ret =
            SteamAPI_Init();

        return ret;
    }
}

extern "C" {
    __declspec(dllexport)
        void c_SteamAPI_Run(void)
    {
        SteamAPI_RunCallbacks();
    }
}

extern "C" {
    __declspec(dllexport)
        bool c_SteamAPI_RestartAppIfNecessary(uint32_t id)
    {
        //return SteamAPI_RestartAppIfNecessary(id);

        bool ret =
            SteamAPI_RestartAppIfNecessary(id);

        /*if (ret)
            OutputDebugStringA("steam wrapper, ret true");
        else
            OutputDebugStringA("steam wrapper, ret false");*/

        return ret;
    }
}



extern "C" {
    __declspec(dllexport)
        bool c_SteamAPI_SetAchievement(const char* id)
    {
        /*if (!g_currentAppID) {
            OutputDebugStringA("steam_cpp_wrapper_custom, c_SteamAPI_SetAchievement: app id not set. Refused request.");
            return false;
        }*/

        if (!g_steamAchievements)
            g_steamAchievements = std::make_unique<CSteamAchievements>();


        return g_steamAchievements->SetAchievement(id);
    }
}
