
// DuilibLearn.cpp : ����Ӧ�ó������ڵ㡣
#include <memory>
#include"CatDui.h"
#include "CatDuiThread.h"
#include "basic_form.h"
#include"resource.h"
//#include"NormalDialog.h"
#include"HookTool.h"




//����hook
#include"HookTool.h"
//#include "duilib/Core/ZipStreamIO.h"


//#include"duilib"

void UIMiscThread::OnInit()
{
//#if defined(DUILIB_BUILD_FOR_WIN)&&!defined (DUILIB_BUILD_FOR_SDL)
//    ::OleInitialize(nullptr);
//#endif
}

void UIMiscThread::OnCleanup()
{
//#if defined(DUILIB_BUILD_FOR_WIN)&&!defined (DUILIB_BUILD_FOR_SDL)
//    ::OleUninitialize();
//#endif
}






//Hook���޸�dll��Դ��������
//zipManager.h��private�ĳ���public��
// ����δ������nimdui��ʱ��public�����ǻ�privateҲ��ͨ��������ٶ�λ����
//��Ϊͨ��hook�Ŀ��߼��Ǽ���������Ϊ�����Կ��Խ��ܡ�
//����д����۸ľͿ�������������ͻ��private���ƣ�c++ģ��ע��freand����
// 
// ����ȥ�Ŀ�Դ��
// 
//extern "C" void* unzOpen2(const char*, zlib_filefunc_def*);
//static bool (*orig_ZipManager_OpenResZip)(ui::ZipManager* pthis, HMODULE hModule, LPCTSTR resourceName, LPCTSTR resourceType, const DString& password) = NULL;
//static bool Hook_ZipManager_OpenResZip(ui::ZipManager* pthis, HMODULE hModule, LPCTSTR resourceName, LPCTSTR resourceType, const DString& password)
//{
//    HRSRC rsc = ::FindResource(hModule, resourceName, resourceType);
//    ASSERT(rsc != nullptr);
//    if (rsc == nullptr) {
//        return false;
//    }
//    //uint8_t* pData = (uint8_t*)::LoadResource(nullptr, rsc);
//    //uint32_t nDataSize = ::SizeofResource(nullptr, rsc);
//    uint8_t* pData = (uint8_t*)::LoadResource(hModule, rsc);
//    uint32_t nDataSize = ::SizeofResource(hModule, rsc);
//    ASSERT((pData != nullptr) && (nDataSize > 0));
//    if ((pData == nullptr) || (nDataSize == 0)) {
//        return false;
//    }
//    pthis->CloseResZip();
//    pthis->m_password = password;
//
//
//
//    pthis->m_pZipStreamIO = std::make_unique<ui::ZipStreamIO>(pData, nDataSize);
//    zlib_filefunc_def pzlib_filefunc_def;
//    pthis->m_pZipStreamIO->FillFopenFileFunc(&pzlib_filefunc_def);
//    pthis->m_hzip = ::unzOpen2(nullptr, &pzlib_filefunc_def);
//    return  pthis->m_hzip != nullptr;
//
//}






void UIMainThread::OnInit()
{
    static bool init = false;
    if (init)return;
    init = true;

    // �������´����߳�
    misc_thread_.reset(new UIMiscThread(ui::kThreadMisc, L"Global Misc Thread"));
    misc_thread_->Start();

    // ��ȡ��Դ·������ʼ��ȫ�ֲ���
    // Ĭ��Ƥ��ʹ�� resources\\themes\\default
    // Ĭ������ʹ�� resources\\lang\\zh_CN
    // �����޸���ָ�� Startup �����������
   // std::wstring theme_dir = QPath::GetAppPath();
    //ui::GlobalManager::Startup(theme_dir + L"resources\\", ui::CreateControlCallback(), false);




#ifdef _DEBUG

    ui::FilePath resourcePath = ui::FilePathUtil::GetCurrentModuleDirectory();
    //resourcePath += L"resources\\";
    resourcePath += L"CSPMOD_res\\";

    ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));



#else
    //windows ����release�汾ʱ��zip���������Դ��


    //ԭ��û����ȷ����dllģ����
    //��hook����ʱ�޸��������

    //auto pTargetFunc=  (&ui::ZipManager::OpenResZip);
    //void** ppTargetFunc = (void**)&pTargetFunc;
    //void* pFunc = *ppTargetFunc;
    //CatHook::AutoHook(pFunc,(void*)Hook_ZipManager_OpenResZip,(void**) & orig_ZipManager_OpenResZip);
    






    //ʹ��exe��Դ�ļ��е�zipѹ����
    ui::ResZipFileResParam resParam;
    resParam.resourcePath = _T("CSPMOD_res\\");
    //������dll���ܿ��ţ����ű�ʾexe����Դ����Ҫ������dll��Դ
    resParam.hResModule = (HMODULE)CatHook::thisModule;
    resParam.resourceName = MAKEINTRESOURCE(IDR_THEME);
    resParam.resourceType = _T("Theme");
    resParam.zipPassword = _T("");
    ui::GlobalManager::Instance().Startup(resParam);
    //�ҳ����ui����bug���Լ���dll����Դû�н��в���.
    

#endif // _DEBUG










    //��������
    //BasicForm* window = new BasicForm();
    //window->CreateWnd(nullptr,ui::WindowCreateParam(L"CSP Helper", true));
    ////����Ӧ�������ڱ��ر�ʱ���˳����������Ǽ�����CSP���̵߳ģ�������CSP�˳�
    ////window->PostQuitMsgWhenClosed(true);
    //window->ShowWindow(ui::kSW_SHOW_NORMAL);




}

void UIMainThread::OnCleanup()
{


    misc_thread_->Stop();
    misc_thread_.reset(nullptr);
    ui::GlobalManager::Instance().Shutdown();



}