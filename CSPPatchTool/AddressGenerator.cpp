#include"DllInject.h"
#include "AddressGenerator.h"
#include"Util/Util.h"
#include <cstdint>
#include<SDL3/SDL.h>






bool AddressGenerator::PushAddressData()
{
    if (!PushCspVersion())
    {
        return false;
    }
    PushCSPHelperAddr();
    PushToneAdjustmentAddr();
    PushCSPDataAddr();
    PushTimeLapseExportAddr();
    PushLayerObjectAddr();


    PushTimerPointAddr();
    PushUDMPluginUnlockAddr();
    return true;
}








bool AddressGenerator::_MatchFeatureCode(uint8_t* dest, uint8_t* fetureCode, size_t codeSize)
{
    for (size_t i = 0; i < codeSize; i++)
    {
        if (dest[i] == fetureCode[i] || fetureCode[i] == 0xcc)
        {
            continue;
        }
        else
        {
            return false;
        }
    }
    return true;
}







void AddressGenerator::SetUp(DllInject* parent)
{
    _parent = parent;
    for (auto& x : parent->sectionheads)
    {
        if (0 == strcmp(x.name, ".text"))
        {
            _codeMem = parent->newExeFileMem.data() + x.pointerToRawData;
            _codeMemSize = x.virtualSize;
            _VA = x.virtualAddress;
            return;
        }
    }
    _codeMem = parent->newExeFileMem.data();
    _codeMemSize = parent->newExeFileMem.size();
    _VA = 0;
}



bool AddressGenerator::PushCspVersion()
{
    
    {
        //wchar_t str1[] = L"ProductName\0\0CLIP STUDIO PAINT\0000\0\0ProductVersion";
        wchar_t str1[] = L"ProductName\0\0CLIP STUDIO PAINT\0\0\0\0ProductVersion";//后面有一段不知道是什么内容
        uint8_t* mem = (uint8_t*)str1;
        for (int i = 0; i < 6; i++)
        {
            *(mem + sizeof(L"ProductName\0\0CLIP STUDIO PAINT") + i) = 0XCC;
        }



        for (size_t i = 0; i < _parent->newExeFileMem.size() - sizeof(str1); i++)
        {
            if (_MatchFeatureCode(_parent->_exeFileMem + i, (uint8_t*)str1, sizeof(str1)))
            {
                wchar_t* getPos = (wchar_t*)(_parent->newExeFileMem.data() + i + sizeof(str1));
                char buffer[256];
                size_t receiveSize;
                auto err = wcstombs_s(&receiveSize, buffer, getPos, sizeof(buffer));
                addrJson["CspVersion"] = buffer;
                addrJson["CspAddressRVA"]["CspVersion"] = _parent->FOA2RVA(i) + sizeof(str1);


                printf("CSP Version: %s\n", buffer);

                return true;
            }
        }
    }
    //未找到CSP 寻找UDM
    {
        wchar_t str1[] = L"PAINT\0\0\0\0ProductVersion";
        uint8_t* mem = (uint8_t*)str1;
        for (int i = 0; i < 6; i++)
        {
            *(mem + sizeof(L"PAINT") + i) = 0XCC;
        }


        for (size_t i = 0; i < _parent->newExeFileMem.size() - sizeof(str1); i++)
        {
            if (_MatchFeatureCode(_parent->newExeFileMem.data() + i, (uint8_t*)str1, sizeof(str1)))
            {
                wchar_t* getPos = (wchar_t*)(_parent->newExeFileMem.data() + i + sizeof(str1));
                char buffer[256];
                size_t receiveSize;
                auto err = wcstombs_s(&receiveSize, buffer, getPos, sizeof(buffer));
                addrJson["CspVersion"] = buffer;
                addrJson["IsUDM"] = true;
                addrJson["CspAddressRVA"]["CspVersion"] = _parent->FOA2RVA(i) + sizeof(str1);


                printf("UDM Version: %s\n", buffer);

                return true;
            }
        }
    }
    //4.0版本寻找UDM版本号码
    {
        //wchar_t str1[] = L"PAINT\0\0\0\0ProductVersion";
        wchar_t str1[] = L"ProductVersion";
        wchar_t str2[] = L"VarFileInfo";



        for (size_t i = 0; i < _parent->newExeFileMem.size() - sizeof(str1); i++)
        {
            if (_MatchFeatureCode(_parent->newExeFileMem.data() + i, (uint8_t*)str1, sizeof(str1)))
            {
                //匹配成功后在后方小段字节内寻找VarFileInfo
                //成功则视为有效
                for (size_t j = i+sizeof(str1); j < _parent->newExeFileMem.size() - sizeof(str2)
                    && j < i + sizeof(str1)+32; j++)
                {
                    if (_MatchFeatureCode(_parent->newExeFileMem.data() + j, (uint8_t*)str2, sizeof(str2)))
                    {
                        wchar_t* getPos = (wchar_t*)(_parent->newExeFileMem.data() + i + sizeof(str1));
                        char buffer[256];
                        size_t receiveSize;
                        auto err = wcstombs_s(&receiveSize, buffer, getPos, sizeof(buffer));
                        addrJson["CspVersion"] = buffer;
                        addrJson["IsUDM"] = true;
                        addrJson["CspAddressRVA"]["CspVersion"] = _parent->FOA2RVA(i) + sizeof(str1);


                        printf("UDM Version: %s\n", buffer);

                        return true;
                    }
                }
            }
        }
    }



    SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_ERROR,"Can not fine CSP/UDM Version Info!");
    return false;
}

void AddressGenerator::PushToneAdjustmentAddr()
{
    _PushHsvAddr();
    _PushColorBalanceAddr();
    _PushToneCurveAddr();
}


void AddressGenerator::GenerateOutputFile()
{
    if (!addrJson.empty())
    {
        if (!util::SaveJsonToFile(addrJson, "CSPAddrTable.json"))
        {
            SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_ERROR, "写文件失败，请尝试右键以管理员身份运行此程序");
            SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_ERROR, "The program does not have write permission for this folder. Please try running this program as administrator.");
        }
    }


}











void AddressGenerator::PushTimerPointAddr()
{
    uint8_t timerMsToNsFeature[] = {
0x48,0x69,0xC2,0x40,0x42,0x0F,0x00,
0x48,0x89,0x41,0x08,
0xC3
    };


    uint8_t timerFeature[] = {
0x8B ,0x15   ,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
,0x48 ,0x8D ,0x4C ,0x24 ,0x20

,0xE8 ,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
,0x48 ,0x8D ,0x05  ,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
,0x48 ,0x89 ,0x44 ,0x24 ,0x30
//,0x48 ,0x89 ,0x5C ,0x24 ,0x38
//,0x0F ,0x28 ,0x44 ,0x24 ,0x30
//,0x66 ,0x0F ,0x7F ,0x44 ,0x24 ,0x30
//,0x48 ,0x8D ,0x4C ,0x24 ,0x30
//削短？？

//8B ,15   , ??, ??, ??, ??,
//,48 ,8D ,4C ,24 ,20
//,E8 , ??, ??, ??, ??,
//,48 ,8D ,05  , ??, ??, ??, ??,
//,48 ,89 ,44 ,24 ,30
//,48 ,89 ,5C ,24 ,38
//,0F ,28 ,44 ,24 ,30
//,66 ,0F ,7F ,44 ,24 ,30
//,48 ,8D ,4C ,24 ,30
    };


    SDL_Log("Start Find TimerPoint Addresses...");
	bool anyCodeFinded = false;
    int j = 0;
    for (uint32_t i = 0; i < _codeMemSize - sizeof(timerMsToNsFeature); i++)
    {
        if (_MatchFeatureCode(_codeMem + i, timerMsToNsFeature, sizeof(timerMsToNsFeature)))
        {

            //先找毫秒转纳秒的函数地址以适当减少应用范围
            uint32_t timerMsToNsFuncAddr = i;
            for (i = 0; i < _codeMemSize - sizeof(timerFeature); i++)
            {


                if (_MatchFeatureCode(_codeMem + i, timerFeature, sizeof(timerFeature)))
                {
                    uint32_t callAddr = *(uint32_t*)(_codeMem + i + 12) + i + 12 + 4;
                    if (callAddr != timerMsToNsFuncAddr)continue;


                    addrJson["CspAddressRVA"]["TimerAddrs"][j] = (_VA + i);
                    j++;
                    anyCodeFinded = true;
                }


            }


            break;



            //uint8_t cmd_mov_edx_0_nop[6] = { 0xba,0x01,0x00 ,0x00 ,0x00 ,0x90 };
            //memcpy(_codeMem + i, cmd_mov_edx_0_nop, sizeof(cmd_mov_edx_0_nop));

        }
    }

    if (anyCodeFinded)
    {
        SDL_Log("TimerPoint Addresses Finded.");
    }
    else
    {
        SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_ERROR, "TimerPoint Addresses Not Found!");
    }


}

void AddressGenerator::PushUDMPluginUnlockAddr()
{

    uint8_t pluginFeature[] = {
    0x0F,0x84,BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD,0x48,0x8D,0x8C,0x24,BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD,
        0xE8, BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD,
        0x90,0x48,0x8B,0x54,0x24,BYTEWILDCARD,0x48,0x81,0xC2,
        0x88,0x00,0x00,0x00,0x48,0x8D,0x8C,0x24,BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD,
        0xE8, BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD,
        0x48,0x8D,0x8C,0x24,BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD,
        0xE8, BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD,
        0x85,0xC0,0x0F,0x8E
		//4.2.0失效了，这里减少特征码长度并将rsp+X也改为通配符



    //0F,84,??, ??, ??, ??,48,8D,8C,24,??, ??, ??, ??,
    //    E8, ??, ??, ??, ??,
    //    90,48,8B,54,24,??,48,81,C2,
    //    88,00,00,00,48,8D,8C,24,??, ??, ??, ??,
    //    E8, ??, ??, ??, ??,
    //    48,8D,8C,24,??, ??, ??, ??,
    //    E8, ??, ??, ??, ??,
    //    85,C0,0F,8E



    //0F,84,68,01,00,00,48,8D,8C,24,E8,00,00,00,
    //    E8, ??, ??, ??, ??,
    //    90,48,8B,54,24,30,48,81,C2,
    //    88,00,00,00,48,8D,8C,24,E8,00,00,00,
    //    E8, ? ? , ? ? , ? ? , ? ? ,
    //    48,8D,8C,24,E8,00,00,00,
    //    E8, ? ? , ? ? , ? ? , ? ? ,
    //    85,C0,0F,8E,1E,01,00,00,
    //    48,8D,8C,24,E8,00,00,00,
    //    E8, ? ? , ? ? , ? ? , ? ? ,
    };


    SDL_Log("Start Find Plugin Unlock Address...");
    bool anyCodeFinded = false;
    for (size_t i = 0; i < _codeMemSize - sizeof(pluginFeature); i++)
    {
        if (_MatchFeatureCode(_codeMem + i, pluginFeature, sizeof(pluginFeature)))
        {

            //第三方插件和官方插件在同一个地方加载，
            //我们寄宿到的插件是高斯模糊，首字母G会比流畅调色插件首字母S早
            //所以高斯模糊加载后，后面的第三方插件是能有效加载的
            //这个会找到2个地址，在跨越版本时难以找到具体单个的特征，全部改了
            addrJson["CspAddressRVA"]["UDMPluginUnlockAddrVec"].append(_VA + i);
            anyCodeFinded = true;
            //je指令改为nop
            //uint8_t cmd_nop[6] = { 0x90,0x90,0x90 ,0x90 ,0x90 ,0x90 };
            //memcpy(_codeMem + i, cmd_nop, sizeof(cmd_nop));

        }
    }

    if (anyCodeFinded)
    {
        SDL_Log("Plugin Unlock Address Finded.");
    }
    else
    {
		SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_ERROR, "Plugin Unlock Address Not Found!");
    }


}
