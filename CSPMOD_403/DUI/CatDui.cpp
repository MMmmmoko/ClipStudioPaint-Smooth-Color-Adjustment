//#include "DuiCommon.h"
//#include "CatDui.h"
//#include"CatDuiThread.h"
//#include "basic_form.h"
////#include"NormalDialog.h"
//
//
//
////#include"../AppSettingsSystem/AppSettingsSystem.h"
//
//
//#include"../Util/StringUtil.h"
//CatDui* CatDui::pthis = NULL;
//
//CatDui* CatDui::GetInstance()
//{
//	if (pthis == nullptr)
//	{
//		//创建ui线程
//
//	// 启动杂事处理线程
//
//
//		//如果是Release则从压缩包载入资源
////#ifdef asfsadfsarfewtr
//
//#ifdef _DEBUG
//		//std::string currentLang = cat::app::AppSettingsSystem::GetInstance().GetSettingValue<std::string>("Language");
//		std::string currentLang = "zh_CN";
//
//
//		pthis = new CatDui();
//		std::wstring theme_dir = nbase::win32::GetCurrentModuleDirectory();
//		ui::GlobalManager::Startup(theme_dir + L"Dui\\", [](const std::wstring& strclass)->ui::Control* {
//			//if (strclass == L"CTB_D2DCanvas")
//			//{
//			//	assert(false, && "Dont Use D2DCanvas Now.");
//			//	return new cat::dui::D2DCanvas;
//			//}
//			//if (strclass == L"TableView")
//			//{
//			//	assert(false, && "Dont Use TableView Now.");
//			//	return new cat::dui::TableView;
//			//}
//			//添加控件的时候记得下面release也加加
//			return NULL;
//			}, false, L"themes\\default", L"lang\\" + cat::util::String2Wstring(currentLang));
//
//
//
//#else
//		std::string currentLang = cat::app::AppSettingsSystem::GetInstance().GetSettingValue<std::string>("Language");
//		bool result = ui::GlobalManager::OpenResZip(nbase::win32::GetCurrentModuleDirectory() + L"83D734FE-157D-4E9E-976F-D11CDC98A467", "catdui");
//		assert(result);
//
//		pthis = new CatDui();
//		std::wstring theme_dir = nbase::win32::GetCurrentModuleDirectory();
//		ui::GlobalManager::Startup(L"", [](const std::wstring& strclass)->ui::Control* {
//			if (strclass == L"DrawObjItemContainer")
//			{
//				return new DrawObjItemContainer;
//			}
//			if (strclass == L"CTB_Canvas")
//			{
//				return new cat::dui::Canvas;
//			}
//			if (strclass == L"CatSlider")
//			{
//				return new cat::dui::CatSlider;
//			}
//			if (strclass == L"SliderEx")
//			{
//				return new cat::dui::SliderEx;
//			}
//			if (strclass == L"CatChildBox")
//			{
//				return new cat::dui::ChildBox;
//			}
//			if (strclass == L"CatCheckBox")
//			{
//				return new cat::dui::CheckBox;
//			}
//			return NULL;
//			}, false, L"themes\\default", L"lang\\" + cat::util::String2Wstring(currentLang));
//
//#endif // _DEBUG
//	}
//	return pthis;
//}
//
//void CatDui::ShutDown()
//{
//	ui::GlobalManager::Shutdown();
//}
//
//
//
//void CatDui::OpenMainUIWindow()
//{
//	if (pMainWindow)
//	{//如果已经有了ui窗口则显示窗口并至于窗口上层
//		pMainWindow->ActiveWindow();
//	}
//	else
//	{//否则通知主线程创建窗口
//		//创建杂事线程
//
//		nbase::ThreadManager::PostTask(ThreadIds::kThreadMain, [this] {
//			//创建一个默认带有阴影的居中窗口
//			if (pMainWindow)return;
//
//			pMainWindow = new BasicForm();
//			pMainWindow->AttachWindowClose([this](ui::EventArgs*) {
//				//当窗口关闭时把指针置空//todo::ui多窗口时关闭所有窗口
//				//ui::GlobalManager::Shutdown();
//				ClearCache();
//				pMainWindow = NULL;
//				return false;
//				});
//			pMainWindow->Create(NULL, BasicForm::kClassName.c_str(), WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, 0);
//			pMainWindow->CenterWindow();
//			pMainWindow->ShowWindow();
//			pMainWindow->ActiveWindow();
//			});
//	}
//}
//
//void CatDui::ClearCache()
//{
//	//ui::GlobalManager::RemoveAllImages();
//	//ui::GlobalManager::RemoveAllFonts();
//}
//
//bool CatDui::isMainUIWindowOpen()
//{
//	if (pMainWindow)
//	{
//		return true;
//	}
//	else
//	{
//		return false;
//	}
//
//}
//
//HWND CatDui::GetMainUIWindowHWND()
//{
//	return pMainWindow->GetHWND();
//}
//
//CatDui::~CatDui()
//{
//	ui_main_thread.reset(NULL);
//}
//
//CatDui::CatDui()
//{
//	pMainWindow = NULL;
//	ui_main_thread.reset(new UIMainThread());
//	ui_main_thread->StartWithLoop(nbase::MessageLoop::kUIMessageLoop);
//}
