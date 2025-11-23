#include "About_Dlg.h"
#include"CspData.h"
#include"DUI/CatDuiThread.h"
#include<SDL3/SDL.h>
#include"ScriptAction_ApplyEffects.h"



DString About_Dlg::GetSkinFolder()
{
    return L"cspHelper_default";
}
DString About_Dlg::GetSkinFile()
{
    return L"About_Dlg.xml";
}
void About_Dlg::OnInitWindow()
{
    __super::OnInitWindow();


    ui::Label* versionStr= static_cast<ui::Label*>(FindControl(L"text_version"));
    std::string vstr = "c4.1.4-m2.0.0";

    versionStr->SetUTF8Text(vstr);
    



    ui::RichText* url_openSourceCode= static_cast<ui::RichText*>(FindControl(L"url_openSourceCode"));
    ui::RichText* url_developerMainPage= static_cast<ui::RichText*>(FindControl(L"url_developerMainPage"));


    url_openSourceCode->AttachLinkClick(UiBind(&About_Dlg::OnLinkClick, this, std::placeholders::_1));
    url_developerMainPage->AttachLinkClick(UiBind(&About_Dlg::OnLinkClick, this, std::placeholders::_1));



    //其他UI控件
    ui::Button* pBtn_Close = static_cast<ui::Button*>(FindControl(L"btn_Close"));
    if (pBtn_Close)
    {
        pBtn_Close->AttachClick(UiBind(&About_Dlg::OnButtonClick, this, std::placeholders::_1));
    }
    ui::Button* pBtn_OK = static_cast<ui::Button*>(FindControl(L"btn_OK"));
    if (pBtn_OK)
    {
        pBtn_OK->AttachClick(UiBind(&About_Dlg::OnButtonClick, this, std::placeholders::_1));
    }
    //ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, [this] {static_cast<ui::Label*>(FindControl(L"text_version"))->Arrange(); }); 

  
   
}
bool About_Dlg::OnButtonClick(const ui::EventArgs& args)
{
    DString controlName = args.GetSender()->GetName();
    if (controlName == L"btn_Close" || controlName == L"btn_Cancel")
    {
        this->CloseWnd();
        return true;
    }
    if (controlName == L"btn_OK")
    {
        this->CloseWnd();
        return true;
    }
    return true;
}

bool About_Dlg::OnLinkClick(const ui::EventArgs& args)
{
    //link在wparam
    std::wstring urlStr = (wchar_t*)args.wParam;
    SDL_OpenURL(ui::StringConvert::WStringToUTF8(urlStr).c_str());
    return true;
}


void About_Dlg::ShowModalDlg()
{
    //没有使用原始的domodal函数，所以需要new
    About_Dlg* dialog = new About_Dlg();
    dialog->DoModalNative(CspData::GetNativeWindowHandle());
    return;
}







