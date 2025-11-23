#include "ApplyEffects_Dlg.h"
#include"CspData.h"
#include"DUI/CatDuiThread.h"
#include<SDL3/SDL.h>
#include"ScriptAction_ApplyEffects.h"


void ApplyEffects_Dlg::Start()
{
    //先检查是否有执行条件
    ScriptAction_ApplyEffects applyEffects;
    if (!(ScriptAction_ApplyEffects::IsEnabled() && applyEffects.CheckHasEffects()))
    {
        ApplyEffectsCheckFailed_Dlg::ShowModalDlg();
        return;
    }

    if (!ApplyEffectsStart_Dlg::ShowModalDlg())
    {
        return;
    }

    //ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadMisc, [&applyEffects] (){
    //    applyEffects.DoAction();
    //    });
   
    //ApplyEffectsProgress_Dlg::ShowModalDlg(&applyEffects);

    ////防止用户ALT+4强行关闭窗口？
    //while (!applyEffects.IsEnd())
    //{
    //    SDL_Delay(100);
    //}



    //ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadMisc, [&applyEffects] (){
    //    ApplyEffectsProgress_Dlg::ShowModalDlg(&applyEffects);
    //    });


    ApplyEffectsProgress_Dlg::ShowModalDlg(&applyEffects);


    //applyEffects.DoAction();








    //ApplyEffectsProgress_Dlg::ShowModalDlg(&applyEffects);

    //防止用户ALT+4强行关闭窗口？
    //while (!applyEffects.IsEnd())
    //{
    //    SDL_Delay(100);
    //}


}

#pragma region CHECKFAILED

DString ApplyEffectsCheckFailed_Dlg::GetSkinFolder()
{
    return L"cspHelper_default";
}
DString ApplyEffectsCheckFailed_Dlg::GetSkinFile()
{
    return L"ApplyEffectsCheckFailed_Dlg.xml";
}
void ApplyEffectsCheckFailed_Dlg::OnInitWindow()
{
    __super::OnInitWindow();
    //其他UI控件
    ui::Button* pBtn_Close = static_cast<ui::Button*>(FindControl(L"btn_Close"));
    if (pBtn_Close)
    {
        pBtn_Close->AttachClick(UiBind(&ApplyEffectsCheckFailed_Dlg::OnButtonClick, this, std::placeholders::_1));
    }
    ui::Button* pBtn_OK = static_cast<ui::Button*>(FindControl(L"btn_OK"));
    if (pBtn_OK)
    {
        pBtn_OK->AttachClick(UiBind(&ApplyEffectsCheckFailed_Dlg::OnButtonClick, this, std::placeholders::_1));
    }


  
   
}
bool ApplyEffectsCheckFailed_Dlg::OnButtonClick(const ui::EventArgs& args)
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


void ApplyEffectsCheckFailed_Dlg::ShowModalDlg()
{
    //没有使用原始的domodal函数，所以需要new
    ApplyEffectsCheckFailed_Dlg* dialog = new ApplyEffectsCheckFailed_Dlg();
    dialog->DoModalNative(CspData::GetNativeWindowHandle());
    return;
}

#pragma endregion










#pragma region STARTWARNING






bool ApplyEffectsStart_Dlg::ShowModalDlg()
{
    //没有使用原始的domodal函数，所以需要new
    bool result=false;
    ApplyEffectsStart_Dlg* dialog = new ApplyEffectsStart_Dlg(&result);
    dialog->DoModalNative(CspData::GetNativeWindowHandle());
    return result;
}





ApplyEffectsStart_Dlg::ApplyEffectsStart_Dlg(bool* resultOut)
{
    pResult = resultOut;
}

ApplyEffectsStart_Dlg::~ApplyEffectsStart_Dlg()
{
}

DString ApplyEffectsStart_Dlg::GetSkinFolder()
{
    return L"cspHelper_default";
}

DString ApplyEffectsStart_Dlg::GetSkinFile()
{
    return L"ApplyEffectsStart_Dlg.xml";
}



void ApplyEffectsStart_Dlg::OnInitWindow()
{
    __super::OnInitWindow();



    auto contentBox= static_cast<ui::VBox*>(FindControl(L"btn_Close"));
    contentBox->ArrangeAncestor();
    

    //其他UI控件
    ui::Button* pBtn_Close = static_cast<ui::Button*>(FindControl(L"btn_Close"));
    if (pBtn_Close)
    {
        pBtn_Close->AttachClick(UiBind(&ApplyEffectsStart_Dlg::OnButtonClick, this, std::placeholders::_1));
    }

    ui::Button* pBtn_OK = static_cast<ui::Button*>(FindControl(L"btn_OK"));
    if (pBtn_OK)
    {
        pBtn_OK->AttachClick(UiBind(&ApplyEffectsStart_Dlg::OnButtonClick, this, std::placeholders::_1));
    }
    ui::Button* pBtn_Cancel = static_cast<ui::Button*>(FindControl(L"btn_Cancel"));
    if (pBtn_Cancel)
    {
        pBtn_Cancel->AttachClick(UiBind(&ApplyEffectsStart_Dlg::OnButtonClick, this, std::placeholders::_1));
    }
}

bool ApplyEffectsStart_Dlg::OnButtonClick(const ui::EventArgs& args)
{
    DString controlName = args.GetSender()->GetName();
    if (controlName == L"btn_Close" || controlName == L"btn_Cancel")
    {
        this->CloseWnd();
        if (pResult)*pResult = false;
        return true;
    }

    if (controlName == L"btn_OK")
    {
        this->CloseWnd();
        if (pResult)*pResult = true;
        return true;
    }
    return true;
}



#pragma endregion







#pragma region PROGRESS




void ApplyEffectsProgress_Dlg::ShowModalDlg(ScriptAction_ApplyEffects* pScript)
{
    ApplyEffectsProgress_Dlg* dialog = new ApplyEffectsProgress_Dlg(pScript);
    dialog->DoModalNative(CspData::GetNativeWindowHandle());
}


DString ApplyEffectsProgress_Dlg::GetSkinFolder()
{
    return L"cspHelper_default";
}

DString ApplyEffectsProgress_Dlg::GetSkinFile()
{
    return L"ApplyEffectsProgress_Dlg.xml";
}

void ApplyEffectsProgress_Dlg::OnInitWindow()
{
    pLabel_finishedCount = static_cast<ui::Label*>(FindControl(L"label_finishedCount"));
    pLabel_totalCount = static_cast<ui::Label*>(FindControl(L"label_totalCount"));



    //执行刷新
    //ui::StdClosure
    //这个和CSP共享主线程会出问题吗...
    if (pScript)
    {
        pScript->DoInit();
        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&ApplyEffectsProgress_Dlg::_UpdateDataLoop, this));
    
    
    
    }
}

void ApplyEffectsProgress_Dlg::_UpdateDataLoop()
{
    if (pScript)
    {
        uint32_t finishedCount=0;
        uint32_t totalCount=0;
        pScript->GetProgress(&finishedCount,&totalCount);

        pLabel_finishedCount->SetText(std::to_wstring(finishedCount));
        pLabel_totalCount->SetText(std::to_wstring(totalCount));
        if (pScript->IsEnd())
        {
            this->CloseWnd();
            return;
        }
        pScript->DoProcessLayer();

        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&ApplyEffectsProgress_Dlg::_UpdateDataLoop, this));
    }
}





#pragma endregion