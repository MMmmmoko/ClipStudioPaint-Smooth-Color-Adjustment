#include<Windows.h>
#include<iostream>
#include"DllInject.h"
#include<SDL3/SDL.h>



void ProcessCSP(uint8_t* mem,size_t size,DllInject::EXETYPE exeType)
{
    DllInject cspExeInject;

    cspExeInject.SetUpPE(mem, size, exeType);



    printf("Start Patch...\n");
    cspExeInject.Patch();
}

void SetUpLogFunc()
{
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

    SDL_LogOutputFunction logFunc = [](void* userdata, int category, SDL_LogPriority priority, const char* message)
        {
            HANDLE h = (HANDLE)userdata;
            if (category == SDL_LogCategory::SDL_LOG_CATEGORY_ERROR)
            {
                CONSOLE_SCREEN_BUFFER_INFO info;
                GetConsoleScreenBufferInfo(h, &info);
                WORD oldAttr = info.wAttributes;
                SetConsoleTextAttribute(h, FOREGROUND_RED | FOREGROUND_INTENSITY);

                SDL_GetDefaultLogOutputFunction()(userdata, category, priority, message);

                SetConsoleTextAttribute(h, oldAttr);
                return;
            }
            if (priority== SDL_LogPriority::SDL_LOG_PRIORITY_ERROR&&category == SDL_LogCategory::SDL_LOG_CATEGORY_APPLICATION)
            {
                CONSOLE_SCREEN_BUFFER_INFO info;
                GetConsoleScreenBufferInfo(h, &info);
                WORD oldAttr = info.wAttributes;
                SetConsoleTextAttribute(h, FOREGROUND_RED| FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                SDL_GetDefaultLogOutputFunction()(userdata, category, priority, message);
                SetConsoleTextAttribute(h, oldAttr);
                return;

            }


            SDL_GetDefaultLogOutputFunction()(userdata, category, priority, message);


        };




    SDL_SetLogOutputFunction(logFunc, h);








}































int main()
{
    SetUpLogFunc();
	const char* CSP_Path = "CLIPStudioPaint.exe";
    const char* UDM_Path = "UDMPaintPRO.exe";
    const char* UDMEX_Path = "UDMPaintEX.exe";
    FILE* fp = nullptr;
    errno_t err= fopen_s(&fp, CSP_Path, "rb");
    bool openSuccess = false;

    DllInject::EXETYPE exetype = DllInject::EXETYPE::CSP;
    const char* openfileName = CSP_Path;
    if (err!=0||fp==nullptr)
    {
        err = fopen_s(&fp, UDM_Path, "rb");
        openfileName = UDM_Path;
        exetype=DllInject::EXETYPE::UDMPRO;
    }
    if (err!=0||fp==nullptr)
    {
        err = fopen_s(&fp, UDMEX_Path, "rb");
        openfileName = UDMEX_Path;
        exetype= DllInject::EXETYPE::UDMEX;
    }
    if (err!=0||fp==nullptr)
    {
        SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_ERROR,"Open CLIPStudioPaint.exe/UDMPaintPRO.exe/UDMPaintEX.exe File Failed.\n");
        system("pause");
        return 1;
    }

    printf("Open CSP File Success: %s\n", openfileName);
    //if(!isUDM)
    //    printf("Open CSP File Success: %s\n", CSP_Path);
    //else
    //    printf("Open UDM File Success: %s\n", UDM_Path);



    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);   // 文件大小
    rewind(fp);

    uint8_t* buffer = (uint8_t*)malloc(size);
    if (!buffer)
    {
        fclose(fp);
        printf("?\n");//幽默提示buffer可能为空的警告
        system("pause");
        return 0;
    }

    fread(buffer, 1, size, fp);
    fclose(fp);

    ProcessCSP(buffer,size, exetype);

    // buffer 里就是文件内容
    free(buffer);

    system("pause");
    return 0;
}

