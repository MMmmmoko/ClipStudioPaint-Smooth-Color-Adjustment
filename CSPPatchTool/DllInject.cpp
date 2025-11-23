#include<iostream>
#include<fstream>
#include<filesystem>
#include"CSPHacker.h"
#include "DllInject.h"
#include"AddressGenerator.h"
#include<SDL3/SDL.h>
bool CheckExeHead(const uint8_t* exeFileMem, size_t exeFileSize)
{
    if (exeFileSize < 0x40)
    {
        printf("Not Exe File!");
        return false;
    }

    uint8_t exePEHead[] = {
        0x4D,0x5A,0x90,0x00,    0x03,0x00,0x00,0x00,    0x04,0x00,0x00,0x00,    0xFF,0xFF,0x00,0x00,
        0xB8,0x00,0x00,0x00,    0x00,0x00,0x00,0x00,    0x40,0x00,0x00,0x00,    0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,    0x00,0x00,0x00,0x00,    0x00,0x00,0x00,0x00,    0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,    0x00,0x00,0x00,0x00,    0x00,0x00,0x00,0x00
    };


    return(0 == memcmp(exePEHead, exeFileMem, sizeof(exePEHead)));

}





//https://www.cnblogs.com/zheh/p/4008268.html

bool DllInject::SetUpPE(uint8_t* exeFileMem, size_t exeFileSize,bool isUDM)
{
    if (!CheckExeHead(exeFileMem, exeFileSize))
    {
        printf("Not Exe File!");
        return false;
    }
    _exeFileMem = exeFileMem;
    _exeFileSize= exeFileSize;
    _isUDM = isUDM;

    //读取节数量


    PEHeadPos = *(uint32_t*)(_exeFileMem + 0x3c);
    NT_Part1StartPos = PEHeadPos + 4;
    NT_Part23StartPos = NT_Part1StartPos + 20;//64bit csp
    imageBase = *(uint64_t*)(_exeFileMem + NT_Part23StartPos + 24);
    fileAlignment = *(uint32_t*)(_exeFileMem + NT_Part23StartPos + 36);
    sectionAlignment = *(uint32_t*)(_exeFileMem + NT_Part23StartPos + 32);




    uint16_t sectionsNum= *(uint16_t*)(exeFileMem + NT_Part1StartPos+2);
    uint8_t* sectionHeadReadPtr = exeFileMem + NT_Part1StartPos + 260;
    for (int i = 0; i < sectionsNum; i++)
    {
        uint8_t* curSection = sectionHeadReadPtr + i * 40;
        //判断这个内存是否全0
        bool isEnd = true;
        for (int byte = 0; byte < 40; byte++)
        {
            if (curSection[byte] != 0)
            {
                isEnd = false;
                break;
            }
        }
        if (isEnd)break;



        PESectiuonHead sectionhead = {};
        memcpy(sectionhead.name, curSection,8);
        sectionhead.virtualAddress = *(uint32_t*)(curSection + 12);
        sectionhead.virtualSize = *(uint32_t*)(curSection + 8);
        sectionhead.sizeOfRawData = *(uint32_t*)(curSection + 16);
        sectionhead.pointerToRawData = *(uint32_t*)(curSection + 20);
        sectionheads.push_back(sectionhead);
    }

    return true;
}

size_t DllInject::RVA2FOA(size_t RVA)
{
    size_t result = 0;

    int sectionIndex=-1;
    for (int i = 0; i < sectionheads.size(); i++)
    {
        if (RVA > sectionheads[i].virtualAddress)
        {
            sectionIndex = i;
        }
        else
        {
            break;
        }
    }
    result = sectionheads[sectionIndex].pointerToRawData + RVA - sectionheads[sectionIndex].virtualAddress;
    return result;
}

size_t DllInject::FOA2RVA(size_t FOA)
{
    size_t result = 0;

    int sectionIndex = -1;
    for (int i = 0; i < sectionheads.size(); i++)
    {
        if (FOA > sectionheads[i].pointerToRawData)
        {
            sectionIndex = i;
        }
        else
        {
            break;
        }
    }

    result = sectionheads[sectionIndex].virtualAddress + FOA - sectionheads[sectionIndex].pointerToRawData;
    return result;
}



std::vector<std::string> DllInject::GetDllList()
{


    uint32_t PEHeadPos=*(uint32_t*)(_exeFileMem + 0x3c);




    uint32_t part1StartPos = PEHeadPos + 4;
    uint32_t part23StartPos = part1StartPos+20;//64bit csp

    uint64_t imageBase = *(uint64_t*)(_exeFileMem + part23StartPos + 24);
    




    uint64_t importTableRVAPos = *(uint32_t*)(_exeFileMem + part23StartPos + 120);
    //importTableRVAPos -= imageBase;

    uint32_t importTableSize = *(uint32_t*)(_exeFileMem + part23StartPos + 124);
    uint32_t importTableCount = importTableSize / 20;

    std::vector<std::string> strVec;

    auto foa = RVA2FOA(importTableRVAPos);
    for (uint32_t i = 0; i < importTableCount; i++)
    {
        auto curTablePtr = _exeFileMem+foa + i * 20;

        bool isEnd = true;
        for (int byte = 0; byte < 20; byte++)
        {
            if (curTablePtr[byte] != 0)
            {
                isEnd = false;
                break;
            }
        }
        if (isEnd)break;



        uint32_t dllNameRVA = *(uint32_t*)(curTablePtr + 12);
        auto dllnameFoa = RVA2FOA(dllNameRVA);


        strVec.push_back((char*)_exeFileMem + dllnameFoa);
    }
    
    





    


    return strVec;
}


bool DllInject::Patch()
{
    if (newExeFileMem.empty())
    {
        newExeFileMem.reserve(_exeFileSize + 1024 * 1024);
        newExeFileMem.resize(_exeFileSize);
        memcpy(newExeFileMem.data(), _exeFileMem, _exeFileSize);
    }

    SetRandomAddrDisable();


    if (!InjectDll())return false;




    CSPHacker hacker;
    AddressGenerator addrGenerator;
    //获取代码段
    size_t sumData = 0;
    for (auto& x : sectionheads)
    {
        if (0 == strcmp(x.name, ".text"))
        {
            hacker.SetUp(newExeFileMem.data()+x.pointerToRawData,x.virtualSize,x.virtualAddress);
            break;
        }
    }


    bool baseHack = false;
    if (std::filesystem::exists("hack.txt"))
    {
        SDL_Log("Try Crack...");
        do
        {
            if (!hacker.DoBaseHack())
            {
                SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_ERROR,"Base Hack Failed");
                break;
            }
            baseHack = true;

            if (!hacker.DoJumpStartWindowHack())
            {
                SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_ERROR, "Hack Jump Start Window Failed\n");
                //break;
            }
            if (!hacker.DoJumpHideTrialText())
            {
                SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_ERROR, "Hack Jump HideTrialText Failed\n");
                //break;
            }
            SDL_Log("Crack Success");
        } while (false);
    }
    //草，好像因为DoBaseHack的模糊检查，这里插件的判断条件已经被自动跳过了，所以不用改了
    //但是还是要过一轮的，因为需要发布无破解的版本
    if (!baseHack)
    {
        if (!hacker.DoPluginUnlock())
        {
            SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_APPLICATION,"Do Plugin Function Unlock Failed");
        }
    }

    //cps某些共同特征计时器一起改
    //基础功能修改
    hacker.DoPatchTimerPoint();



    SDL_Log("Generating Address Table..");
    addrGenerator.SetUp(this);
    if (!addrGenerator.PushAddressData())
    {
        return false;
    }






    addrGenerator.GenerateOutputFile();
    //return true;
    //printf("Generating Address Table Success!");















    const char* outFileName = "CLIPStudioPaint_Patched.exe";
    if (_isUDM)outFileName = "UDMPaintPRO_Patched.exe";
    
    std::ofstream out(outFileName, std::ios::binary | std::ios::trunc);
    if (!out) {
        std::cerr << "Error When Write File"<< outFileName <<"\n";
        return false;
    }

    out.write(reinterpret_cast<const char*>(newExeFileMem.data()),
        static_cast<std::streamsize>(newExeFileMem.size()));
    out.close();

    //如果成功则重命名原CSP进程写入新文件
    SDL_Log("Patch Success!");

    return true;

}

void DllInject::SetRandomAddrDisable()
{
    uint8_t* windowSubSystemDataPos = (newExeFileMem.data() + NT_Part23StartPos + 70);
    //第7个bit表示可以重定位，将它设置为0;
    *windowSubSystemDataPos &= 0xBF;
}

bool DllInject::InjectDll()
{


    const char* dllName = "CSPMOD.dll";
    //避免重复注入CSPMODE.dll
    auto dlllist=GetDllList();
    for (auto& x : dlllist)
    {
        if (x == dllName)
            return true;
    }





    //计算新的RVA结构所需长度
    //void

    //计算节头到节表之间的冗余
    //如果足够，则创建新节，在节尾部添加信息

    size_t remainBytes= sectionheads[0].pointerToRawData-(NT_Part1StartPos + 260+ (sectionheads.size()+1)*40);
    //remainBytes = 0;
    if (remainBytes<40)
    {
        //printf("No Enough Space before SectionData!");
        //创建新的空间

        std::vector<uint8_t>temBuffer;
        temBuffer.resize(newExeFileMem.size()-sectionheads[0].pointerToRawData);
        memcpy(temBuffer.data(),newExeFileMem.data()+ sectionheads[0].pointerToRawData, temBuffer.size());

        newExeFileMem.resize(newExeFileMem.size()+fileAlignment);
        memcpy(newExeFileMem.data()+sectionheads[0].pointerToRawData + fileAlignment, temBuffer.data(), temBuffer.size());
        memset(newExeFileMem.data() + sectionheads[0].pointerToRawData,0, fileAlignment);
        memset(newExeFileMem.data() + NT_Part1StartPos + 260 + (sectionheads.size() + 1) * 40,0, remainBytes);
        //将所有pointerToRawDada加上fileAlignment
        sectionheads.clear();
        {
            uint16_t sectionsNum = *(uint16_t*)(newExeFileMem.data() + NT_Part1StartPos + 2);
            uint8_t* sectionHeadReadPtr = newExeFileMem.data() + NT_Part1StartPos + 260;
            for (int i = 0; i < sectionsNum; i++)
            {
                uint8_t* curSection = sectionHeadReadPtr + i * 40;
                //判断这个内存是否全0
                bool isEnd = true;
                for (int byte = 0; byte < 40; byte++)
                {
                    if (curSection[byte] != 0)
                    {
                        isEnd = false;
                        break;
                    }
                }
                if (isEnd)break;



                PESectiuonHead sectionhead = {};
                memcpy(sectionhead.name, curSection, 8);
                sectionhead.virtualAddress = *(uint32_t*)(curSection + 12);
                sectionhead.virtualSize = *(uint32_t*)(curSection + 8);
                sectionhead.sizeOfRawData = *(uint32_t*)(curSection + 16);
                *(uint32_t*)(curSection + 20) += fileAlignment;
                sectionhead.pointerToRawData = *(uint32_t*)(curSection + 20);
                sectionheads.push_back(sectionhead);
            }
        }
    }




    //有足够空间时，添加新的头部数据,在文件末尾添加新节
    auto pSectionNum = (uint16_t*)(newExeFileMem.data() + NT_Part1StartPos + 2);
    (*(uint16_t*)(newExeFileMem.data() + NT_Part1StartPos + 2))++;//将节section数量+1

    uint8_t* newSection=newExeFileMem.data() + NT_Part1StartPos + 260 +sectionheads.size()*40;
    memcpy(newSection,".CMOD",sizeof(".CMOD"));
    //

    uint32_t* pImportTableRVAPos=(uint32_t*)(newExeFileMem.data() + NT_Part23StartPos + 120);
    uint32_t* pImportTableSize = (uint32_t*)(newExeFileMem.data() + NT_Part23StartPos + 124);

    int oldSectionIndex = -1;
    for (int i = 0; i < sectionheads.size(); i++)
    {
        if (*pImportTableRVAPos > sectionheads[i].virtualAddress)
        {
            oldSectionIndex = i;
        }
        else
        {
            break;
        }
    }


    //flag
    uint32_t  flag= *(uint32_t*)(newExeFileMem.data() + NT_Part1StartPos + 260 + oldSectionIndex * 40 + 36);
    //给flag设置可写
    flag |= (1<<31);
     *(uint32_t*)(newSection + 36) = flag;




    uint32_t oldImportTableSize = *(uint32_t*)(_exeFileMem + NT_Part23StartPos + 124);
    uint32_t spaceReq = static_cast<uint32_t>( oldImportTableSize + 20 + strlen(dllName)+1 + 8*4+100);//8*4：导入查找表
    //  VirtualSize
    *(uint32_t*)(newSection + 8) = spaceReq;






   // VirtualAddress;
    //查看最后一个section的数据
    uint32_t lastVirtualEnd=sectionheads.back().virtualAddress + sectionheads.back().virtualSize;
    uint32_t  newVirtualAddr= lastVirtualEnd+(sectionAlignment-(lastVirtualEnd % sectionAlignment));
    *(uint32_t*)(newSection + 12) = newVirtualAddr;
    //sizeof rawData
    uint32_t sizeOfRawData= spaceReq + (fileAlignment - (spaceReq % fileAlignment));
    *(uint32_t*)(newSection + 16) = sizeOfRawData;
    //pointer to rawData
    uint32_t pointerOfRawData=  static_cast<uint32_t>(newExeFileMem.size() + (fileAlignment - (newExeFileMem.size() % fileAlignment)));
    *(uint32_t*)(newSection + 20) = pointerOfRawData;


    newExeFileMem.resize(pointerOfRawData+ sizeOfRawData,0);

    //修改image大小后程序才能正常运行
    uint32_t* pSizeOfImage = (uint32_t*)(newExeFileMem.data() + NT_Part23StartPos + 56);
    (*pSizeOfImage) += ((spaceReq + sectionAlignment - 1) / sectionAlignment)* sectionAlignment;








    uint8_t* newSectionMemStart = newExeFileMem.data() + pointerOfRawData;

    //拷贝原导入表信息
    memcpy(newSectionMemStart,newExeFileMem.data()+RVA2FOA(*pImportTableRVAPos), oldImportTableSize);
    //拷贝DLL Name
    size_t dllNameFilePos= pointerOfRawData + oldImportTableSize + 20;
    memcpy(newExeFileMem.data()+ dllNameFilePos, dllName, strlen(dllName));

    //构建导入查找表
    uint64_t importAddrTable = 0; 
    importAddrTable |= (1llu << 63);
    importAddrTable += 1;

    //importAddrTable= newVirtualAddr + oldImportTableSize + 20 + strlen(dllName) + 1+32;
    *(uint64_t*)(newExeFileMem.data() + dllNameFilePos + strlen(dllName) + 1)= importAddrTable;
    *(uint64_t*)(newExeFileMem.data() + dllNameFilePos + strlen(dllName) + 17)= importAddrTable;

    //memcpy(newExeFileMem.data() + dllNameFilePos + strlen(dllName) + 1 + 32+2, "CELSYS_PleaseOptmizeTheBasicExperience_onegai", strlen("CELSYS_PleaseOptmizeTheBasicExperience_onegai"));



    //填写导入DLL的RVA
    *(uint32_t*)(newSectionMemStart+ oldImportTableSize -20 +0)= static_cast<uint32_t>( newVirtualAddr + oldImportTableSize + 20+ strlen(dllName) + 1 );
    *(uint32_t*)(newSectionMemStart+ oldImportTableSize -20 +12)= newVirtualAddr+ oldImportTableSize + 20;
    *(uint32_t*)(newSectionMemStart+ oldImportTableSize -20 +16)= *(uint32_t*)(newSectionMemStart + oldImportTableSize - 20 + 0)+16;







    //回填导入表的地址和大小
    *pImportTableRVAPos = newVirtualAddr;
    *pImportTableSize = oldImportTableSize+20;

    return true;



}

