
// DuilibLearn.cpp : 定义应用程序的入口点。
#include <memory>
#include"CatDui.h"
#include "CatDuiThread.h"
#include "basic_form.h"
#include"CSPMOD.h"


#include"resource.h"




void UIMiscThread::OnInit()
{

}

void UIMiscThread::OnCleanup()
{

}







UIMainThread* pUiThread=nullptr;
void UIMainThread::Start()
{
    if (!pUiThread)
    {
        pUiThread = new UIMainThread;
        pUiThread->StartWithoutLoop();
    }
}

void UIMainThread::OnInit()
{
    static bool init = false;
    if (init)return;
    init = true;

    // 启动杂事处理线程
    misc_thread_.reset(new UIMiscThread(ui::kThreadMisc, L"Global Misc Thread"));
    misc_thread_->Start();

    // 获取资源路径，初始化全局参数
    // 默认皮肤使用 resources\\themes\\default
    // 默认语言使用 resources\\lang\\zh_CN
    // 如需修改请指定 Startup 最后两个参数
   // std::wstring theme_dir = QPath::GetAppPath();
    //ui::GlobalManager::Startup(theme_dir + L"resources\\", ui::CreateControlCallback(), false);




    //获取系统语言
    int count;
    SDL_Locale** localeList = SDL_GetPreferredLocales(&count);
    const wchar_t* langstr;
    if (NULL == localeList||0== count)
    {
        SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_APPLICATION, "Can not get locale info! %s", SDL_GetError());
        //TODO/FIXME加入对steam语言的检测
        langstr= L"schinese.ini";
    }
    else
    {
        if (0 == SDL_strcmp(localeList[0]->language, "zh"))
        {
            langstr = L"schinese.ini";
        }
        else if(0 == SDL_strcmp(localeList[0]->language, "ja"))
        {
            langstr=L"japanese.ini";
        }
        else
        {
            langstr=L"english.ini";
        }
    }






#ifdef _DEBUG

    ui::FilePath resourcePath = ui::FilePathUtil::GetCurrentModuleDirectory();
    resourcePath += L"DuiResource\\";
    auto resParam = ui::LocalFilesResParam(resourcePath);


    resParam.languageFileName = L"schinese.ini";
    resParam.languageFileName = L"japanese.ini";
    //resParam.languageFileName = L"english.ini";
    //resParam.languageFileName = langstr;

    ui::GlobalManager::Instance().Startup(resParam);



#else
    //windows 发布release版本时将zip放入程序资源中


    //原库没有正确处理dll模块句柄
    //先hook以暂时修复这个问题

    //auto pTargetFunc=  (&ui::ZipManager::OpenResZip);
    //void** ppTargetFunc = (void**)&pTargetFunc;
    //void* pFunc = *ppTargetFunc;
    //CatHook::AutoHook(pFunc,(void*)Hook_ZipManager_OpenResZip,(void**) & orig_ZipManager_OpenResZip);
    



    


    //使用exe资源文件中的zip压缩包
    ui::ResZipFileResParam resParam;
    resParam.resourcePath = _T("DuiResource\\");
    //我们是dll不能空着，空着表示exe的资源，我要读的是dll资源
    resParam.hResModule = (HMODULE)CSPMOD::GetDllModule() ;
    resParam.resourceName = MAKEINTRESOURCE(IDR_THEME1);
    resParam.resourceType = _T("Theme");
    resParam.zipPassword = _T("");
    resParam.languageFileName = langstr;
    ui::GlobalManager::Instance().Startup(resParam);
    ////我超这个ui库有bug，对加载dll中资源没有进行测试.
    

#endif // _DEBUG










    //创建窗口
    //BasicForm* window = new BasicForm();
    //window->CreateWnd(nullptr,ui::WindowCreateParam(L"CSP Helper", true));
    ////正常应用主窗口被关闭时会退出，但我们是寄生在CSP主线程的，不能让CSP退出
    ////window->PostQuitMsgWhenClosed(true);
    //window->ShowWindow(ui::kSW_SHOW_NORMAL);




}

void UIMainThread::OnCleanup()
{


    misc_thread_->Stop();
    misc_thread_.reset(nullptr);
    ui::GlobalManager::Instance().Shutdown();



}