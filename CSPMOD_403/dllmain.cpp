// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include"ProjConfig.h"
#include"CSPHook.h"

extern "C" __declspec(dllexport) int FakeFuction(void)
{
    ////do anything here////
    return 0;
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CSPHook::Init((uintptr_t)hModule);
        CSPHook::SetUpHook();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}



//CSP PE node
//导入表0x218 ：04BC122C  FOA0x04BC022C.  SIZE 0X550    修改后size：0x550+0x14=0X564