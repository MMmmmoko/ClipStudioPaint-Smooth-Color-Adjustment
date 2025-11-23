#include "DuiCommon.h"
#include "basic_form.h"
//#include <shellapi.h>


BasicForm::BasicForm()
{

}

BasicForm::~BasicForm()
{
}

std::wstring BasicForm::GetSkinFolder()
{
	return L"cspHelper_default";
}

std::wstring BasicForm::GetSkinFile()
{
	return L"controls.xml";
	//return L"cspHelper_default.xml";
}




void BasicForm::OnInitWindow()
{
	__super::OnInitWindow();
}


bool BasicForm::OnLanguageBtnClicked(ui::EventArgs* args)
{
	//RECT rect = args->pSender->GetPos();
	//ui::CPoint point;
	//point.x = rect.left - 175;
	//point.y = rect.top + 10;
	//ClientToScreen(m_hWnd, &point);

	//ui::CMenuWnd* pMenu = new ui::CMenuWnd(NULL);
	//ui::STRINGorID xml(L"../cattuber_default/CTB_LanguageMenu.xml");
	//pMenu->Init(xml, _T("xml"), point);







	//auto& setttingsystem = cat::app::AppSettingsSystem::GetInstance();
	//std::wstring currentLanguage = cat::util::String2Wstring(setttingsystem.GetSettingValue<std::string>("Language"));

	//auto btnvec = pMenu->FindSubControlsByClass(NULL, typeid(ui::CMenuElementUI));
	//for (auto& pmenuitem : *btnvec)
	//{
	//	if (pmenuitem->GetName() == currentLanguage)
	//	{
	//		pmenuitem->SetBkColor(L"bk_listitem_hovered");
	//	}
	//	else
	//	{
	//		((ui::CMenuElementUI*)pmenuitem)->AttachSelect(nbase::Bind(&BasicForm::OnLanguageMenuItemClicked, this, std::placeholders::_1));
	//	}

	//}







	return true;
}

bool BasicForm::OnLanguageMenuItemClicked(ui::EventArgs* msg)
{
//	if (msg)
//	{
//
//
//
//		std::wstring langName = msg->pSender->GetName();
//
//
//#ifdef _DEBUG
//		std::wstring exe_dir = nbase::win32::GetCurrentModuleDirectory();
//		ui::MutiLanSupport::GetInstance()->LoadStringTable(exe_dir + L"Dui\\lang\\" + langName + L"\\gdstrings.ini");
//#else
//		//ui::MutiLanSupport::GetInstance()->LoadStringTable(L"lang\\" + langName + L"\\gdstrings.ini");
//		HGLOBAL hGlobal = ui::GlobalManager::GetData(L"lang\\" + langName + L"\\gdstrings.ini");
//		if (hGlobal) {
//			ui::MutiLanSupport::GetInstance()->LoadStringTable(hGlobal);
//			GlobalFree(hGlobal);
//		}
//#endif // _DEBUG
//
//
//
//
//
//
//
//
//
//
//		auto& setttingsystem = cat::app::AppSettingsSystem::GetInstance();
//		setttingsystem.SetSettingValue<std::string>("Language", cat::util::Wstring2String(langName));
//
//		auto labelvec = FindSubControlsByClass(NULL, typeid(ui::Label));
//		for (auto& label : *labelvec)
//		{
//			label->Arrange();
//		}
//		auto labelvec2 = FindSubControlsByClass(NULL, typeid(ui::Button));
//		for (auto& label : *labelvec)
//		{
//			label->Arrange();
//		}
//		auto labelvec3 = FindSubControlsByClass(NULL, typeid(ui::CheckBox));
//		for (auto& label : *labelvec)
//		{
//			label->Arrange();
//		}
//		auto labelvec4 = FindSubControlsByClass(NULL, typeid(ui::Option));
//		for (auto& label : *labelvec)
//		{
//			label->Arrange();
//		}
//	}




	return true;
}

