#include "pch.h"
#include "AddressTable.h"
#include"CSPMOD.h"
#include"json/json.h"

#include"Util/Util.h"
#include<SDL3/SDL.h>

static AddressTable ins;

bool AddressTable::LoadFromFile()
{
    //以绝对路劲的方式加载json
    
    std::string basePath=SDL_GetBasePath();
    basePath += "CSPAddrTable.json";
    ins.addressMap= util::BuildJsonFromFile(basePath.c_str());
    if (ins.addressMap.empty())
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"CSPMOD Error","Can not read file: CSPAddressTable.json",nullptr);
        return false;
    }


    //return true;


    //测试启动UDM
    bool isUDM = (ins.addressMap.isMember("IsUDM") && ins.addressMap["IsUDM"].isBool() && ins.addressMap["IsUDM"].asBool());
    if (isUDM)
    {
        //WinExec("UDMPaintPRO.exe",SW_HIDE);
        //WinExec("UDMPaintEX.exe", SW_HIDE);

    }


#ifdef _DEBUG
    if (!isUDM)
    {
        //优动漫和控制台窗口有冲突，仅对CSP启用控制台窗口
        AllocConsole(); // 创建控制台窗口
        // 重定向标准输出到控制台
        FILE* stream;
        freopen_s(&stream, "CONOUT$", "w", stdout);
        freopen_s(&stream, "CONOUT$", "w", stderr);
        freopen_s(&stream, "CONIN$", "r", stdin);
    }
#endif
   

    //读取版本
    
    if (ins.addressMap.isMember("CspVersion") && ins.addressMap["CspVersion"].isString()
        && ins.addressMap.isMember("CspAddressRVA")
        && ins.addressMap["CspAddressRVA"].isMember("CspVersion")&& ins.addressMap["CspAddressRVA"]["CspVersion"].isUInt64()
        )
    {
        std::string verStr= ins.addressMap["CspVersion"].asString();
        wchar_t* strptr =(wchar_t*) (ins.addressMap["CspAddressRVA"]["CspVersion"].asUInt64()+ CSPMOD::GetBaseAddr());


        

        char buffer[256];
        size_t receiveSize;
        auto err = wcstombs_s(&receiveSize, buffer, strptr, sizeof(buffer));
        buffer[255] = 0;
        if (verStr == buffer)
        {
            ins.cspVersion = verStr;
            DoStaticPatch();
            return true;
        }
        else
        {
            char outbuffer[512];
            SDL_snprintf(outbuffer, sizeof(outbuffer), "AddrTable Version Mismatch:CSP: %s, CSPAddrTable.json: %s", buffer, verStr.c_str());
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "CSPMOD Error", "AddrTable Version Mismatch: CSPAddressTable.json", nullptr);
            return false;
        }
    }
    




    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "CSPMOD Error", "No Version Info in CSPAddressTable.json", nullptr);
    return false;
}









void AddressTable::DoStaticPatch()
{
    //TimerPoints
    


    if (ins.addressMap["CspAddressRVA"].isMember("TimerAddrs") && ins.addressMap["CspAddressRVA"]["TimerAddrs"].isArray())
    {
        for (auto& x : ins.addressMap["CspAddressRVA"]["TimerAddrs"])
        {
            if (x.isUInt64())
            {
				void* addr = (void*)(x.asUInt64() + CSPMOD::GetBaseAddr());

                uint8_t cmd_mov_edx_0_nop[6] = { 0xba,0x01,0x00 ,0x00 ,0x00 ,0x90 };
				CSPMOD::CodePatch(addr, cmd_mov_edx_0_nop, sizeof(cmd_mov_edx_0_nop));
                //memcpy(_codeMem + i, cmd_mov_edx_0_nop, sizeof(cmd_mov_edx_0_nop));
            }
        }
    }


    //UDMPlugin
    {
		void* pUDMPluginCheckFunc = GetAddress("UDMPluginUnlockAddr");
        if (pUDMPluginCheckFunc)
        {
            uint8_t cmd_nop[6] = { 0x90,0x90,0x90 ,0x90 ,0x90 ,0x90 };
			CSPMOD::CodePatch(pUDMPluginCheckFunc, cmd_nop, sizeof(cmd_nop));
        }
    }




    //return false;
}










void* AddressTable::GetAddress(const char* addrName)
{

    if (ins.addressMap["CspAddressRVA"].isMember(addrName) && ins.addressMap["CspAddressRVA"][addrName].isUInt64())
    {
        return (void*)(ins.addressMap["CspAddressRVA"][addrName].asUInt64()+ CSPMOD::GetBaseAddr());
    }

    return nullptr;
}

std::vector<int> AddressTable::GetOffsetList(const char* offsetName)
{
    std::vector<int> result;
    if (ins.addressMap.isMember(offsetName) && ins.addressMap[offsetName].isArray())
    {
        for (auto& x : addressMap[offsetName])
        {
            if (x.isInt())
            {
                result.push_back(x.asInt());
            }
        }
    }
    return result;
}
