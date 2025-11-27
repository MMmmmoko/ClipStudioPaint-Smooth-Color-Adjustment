// dllmain.cpp : 定义 DLL 应用程序的入口点。
//使用SDL3重新写一个跨平台的？

#include "pch.h"




BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:

        //MessageBox(nullptr,L"test",L"test",MB_OK);
        //利用CSP和优动漫会无差别加载plugin中插件的特性
        //当此插件被读取时，便可以注入CSPMOD.dll
		LoadLibraryA("CSPMOD.dll");
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

