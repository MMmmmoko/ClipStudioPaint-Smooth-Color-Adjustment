// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include<iostream>
#include"CSPMOD.h"
#include"CspData.h"
#include"duilib/duilib.h"
extern "C" __declspec(dllexport) void CELSYS_PleaseOptmizeTheBasicExperience_onegai() {
    printf("CELSYS,Please Optmize the Basic Drawing Experience, onegai.");
}

extern "C" __declspec(dllexport) void CSPMOD_SetStrData(const char* key,const char* value) {
    CspData::SetStrData(key, value);
}

extern "C" __declspec(dllexport) bool CSPMOD_GetStrFromID(wchar_t* buffer,const wchar_t* stringID, uint32_t* strSize) {
    std::wstring str=ui::GlobalManager::Instance().Lang().GetStringViaID(stringID);
    memcpy(buffer, str.c_str(), str.size()*2);
    buffer[str.size()] = 0;
    //*strSize = static_cast<uint32_t>(str.size()+1);
    *strSize = static_cast<uint32_t>(str.size());
    return !str.empty();
}





BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
#ifdef _DEBUG
        //优动漫可能和控制台窗口有冲突，将这段代码移动到AddressTable中
        //AllocConsole(); // 创建控制台窗口
        //// 重定向标准输出到控制台
        //FILE* stream;
        //freopen_s(&stream,"CONOUT$", "w", stdout);
        //freopen_s(&stream, "CONOUT$", "w", stderr);
        //freopen_s(&stream, "CONIN$", "r", stdin);
#endif // DEBUG
        


        CSPMOD::OnAttachCSP((uintptr_t)hModule);

        //利用CSP和优动漫会无差别加载plugin中插件的特性
        LoadLibraryA("CSPMOD.dll");
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

//nimdui
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Comctl32.lib")

#ifdef _DEBUG
#pragma comment(lib, "cximage_d.lib")
#pragma comment(lib, "duilib_d.lib")
#pragma comment(lib, "libpng_d.lib")
#pragma comment(lib, "libwebp_d.lib")
#pragma comment(lib, "zlib_d.lib")
#else
#pragma comment(lib, "cximage_s.lib")
#pragma comment(lib, "duilib.lib")
#pragma comment(lib, "libpng_s.lib")
#pragma comment(lib, "libwebp_s.lib")
#pragma comment(lib, "zlib_s.lib")
#endif // _DEBUG


//SKIA
#ifdef _DEBUG
#pragma comment(lib, "msvc.x64.MDd/skia.lib") 
#pragma comment(lib, "msvc.x64.MDd/bentleyottmann.lib")
#pragma comment(lib, "msvc.x64.MDd/expat.lib")
#pragma comment(lib, "msvc.x64.MDd/jsonreader.lib")
#pragma comment(lib, "msvc.x64.MDd/pathkit.lib")
#pragma comment(lib, "msvc.x64.MDd/skcms.lib")
#pragma comment(lib, "msvc.x64.MDd/skottie.lib")
#pragma comment(lib, "msvc.x64.MDd/skresources.lib")
#pragma comment(lib, "msvc.x64.MDd/skshaper.lib")
#pragma comment(lib, "msvc.x64.MDd/svg.lib")
#else
#pragma comment(lib, "msvc.x64.MD/skia.lib") 
#pragma comment(lib, "msvc.x64.MD/bentleyottmann.lib")
#pragma comment(lib, "msvc.x64.MD/expat.lib")
#pragma comment(lib, "msvc.x64.MD/jsonreader.lib")
#pragma comment(lib, "msvc.x64.MD/pathkit.lib")
#pragma comment(lib, "msvc.x64.MD/skcms.lib")
#pragma comment(lib, "msvc.x64.MD/skottie.lib")
#pragma comment(lib, "msvc.x64.MD/skresources.lib")
#pragma comment(lib, "msvc.x64.MD/skshaper.lib")
#pragma comment(lib, "msvc.x64.MD/svg.lib")
#endif // _DEBUG