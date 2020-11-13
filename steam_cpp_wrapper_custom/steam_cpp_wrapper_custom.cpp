// steam_cpp_wrapper_custom.cpp : Defines the exported functions for the DLL.
//

#include "pch.h"
#include "framework.h"



#include <sdk/public/steam/steam_api.h>
#include <sdk/public/steam/steam_gameserver.h>

extern "C" {
#include "steam_cpp_wrapper_custom.h"
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
        bool c_SteamAPI_RestartAppIfNecessary(uint32_t id)
    {
        //return SteamAPI_RestartAppIfNecessary(id);

        bool ret =
            SteamAPI_RestartAppIfNecessary(id);

        if (ret)
            OutputDebugStringA("steam wrapper, ret true");
        else
            OutputDebugStringA("steam wrapper, ret false");

        return ret;
    }
}
