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
        return
            SteamAPI_Init();
    }
}

extern "C" {
    __declspec(dllexport)
        bool c_SteamAPI_RestartAppIfNecessary(uint32_t id)
    {
        return
            SteamAPI_RestartAppIfNecessary(id);
    }
}
