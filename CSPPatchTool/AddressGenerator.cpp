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
        util::SaveJsonToFile(addrJson,"CSPAddrTable.json");
    }


}
