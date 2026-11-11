#include "CheckFunction_Dlg.h"
#include"CspData.h"
#include"DUI/CatDuiThread.h"

#include"TA_HSV.h"
#include"TA_ColorBalance.h"
#include"TA_ToneCurve.h"
#include"LevelsPreview.h"
#include"FreeTransform.h"
#include"TimeLapseExport.h"
#include"ScriptAction_ApplyEffects.h"
#include"AppSettings.h"
void CheckFunction_Dlg::ShowModalDlg()
{
    //没有使用原始的domodal函数，所以需要new
    CheckFunction_Dlg* dialog = new CheckFunction_Dlg();
    dialog->DoModalNative(CspData::GetNativeWindowHandle());
    return;
}





CheckFunction_Dlg::CheckFunction_Dlg()
{

}

CheckFunction_Dlg::~CheckFunction_Dlg()
{
}

DString CheckFunction_Dlg::GetSkinFolder()
{
    return L"cspHelper_default";
}

DString CheckFunction_Dlg::GetSkinFile()
{
    return L"CheckFunction_Dlg.xml";
}



void CheckFunction_Dlg::OnInitWindow()
{
    __super::OnInitWindow();

    //寻找目标控件
    auto supportedIcon_hsv = static_cast<ui::Control*>(FindControl(L"supportedIcon_hsv"));
    auto supportedIcon_colorbalance = static_cast<ui::Control*>(FindControl(L"supportedIcon_colorbalance"));
    auto supportedIcon_tonecurve = static_cast<ui::Control*>(FindControl(L"supportedIcon_tonecurve"));
    auto supportedIcon_plugin_gaussblur = static_cast<ui::Control*>(FindControl(L"supportedIcon_plugin_gaussblur"));
    auto supportedIcon_plugin_motionblur = static_cast<ui::Control*>(FindControl(L"supportedIcon_plugin_motionblur"));
    auto supportedIcon_timelapseexport = static_cast<ui::Control*>(FindControl(L"supportedIcon_timelapseexport"));
    auto supportedIcon_applyeffects = static_cast<ui::Control*>(FindControl(L"supportedIcon_applyeffects"));
    auto supportedIcon_hideNavigatorViewIndicator = static_cast<ui::Control*>(FindControl(L"supportedIcon_hideNavigatorViewIndicator"));
    
    if (supportedIcon_hsv && TA_HSV::Available())supportedIcon_hsv->SetVisible(true);
    if (supportedIcon_colorbalance && TA_ColorBalance::Available())supportedIcon_colorbalance->SetVisible(true);
    if (supportedIcon_tonecurve && TA_ToneCurve::Available())supportedIcon_tonecurve->SetVisible(true);
    if (supportedIcon_plugin_gaussblur &&  CspData::GetStrData("SmoothFilter_GaussBlur",nullptr))
        supportedIcon_plugin_gaussblur->SetVisible(true);
    if (supportedIcon_plugin_motionblur && CspData::GetStrData("SmoothFilter_MotionBlur", nullptr))
        supportedIcon_plugin_motionblur->SetVisible(true);
    if (supportedIcon_timelapseexport && TimeLapseExport_hook::Available())supportedIcon_timelapseexport->SetVisible(true);
    if (supportedIcon_applyeffects && ScriptAction_ApplyEffects::Available())supportedIcon_applyeffects->SetVisible(true);
    if (supportedIcon_hideNavigatorViewIndicator && CspData::GetHideNavigatorViewIndicatorAvaliable())supportedIcon_hideNavigatorViewIndicator->SetVisible(true);



    //checkBox控件
    checkBox_hsv = static_cast<ui::CheckBox*>(FindControl(L"checkBox_hsv"));
    checkBox_colorbalance = static_cast<ui::CheckBox*>(FindControl(L"checkBox_colorbalance"));
    checkBox_tonecurve = static_cast<ui::CheckBox*>(FindControl(L"checkBox_tonecurve"));
    
    checkBox_timelapseexport = static_cast<ui::CheckBox*>(FindControl(L"checkBox_timelapseexport"));
    checkBox_applyeffects = static_cast<ui::CheckBox*>(FindControl(L"checkBox_applyeffects"));

    checkBox_hideNavigatorViewIndicator= static_cast<ui::CheckBox*>(FindControl(L"checkBox_hideNavigatorViewIndicator"));
    //设置主题
    if (CspColorTable::GetTheme() == CspColorTable::Light)
    {
        checkBox_hsv->SetClass(L"CF_Checkbox_light");
        checkBox_colorbalance->SetClass(L"CF_Checkbox_light");
        checkBox_tonecurve->SetClass(L"CF_Checkbox_light");

        checkBox_timelapseexport->SetClass(L"CF_Checkbox_light");
        checkBox_applyeffects->SetClass(L"CF_Checkbox_light");
        checkBox_hideNavigatorViewIndicator->SetClass(L"CF_Checkbox_light");
    }
    //设置可见性
    {
        if(!TA_HSV::Available())checkBox_hsv->SetVisible(false);
        if (!TA_ColorBalance::Available())checkBox_colorbalance->SetVisible(false);
        if (!TA_ToneCurve::Available()) checkBox_tonecurve->SetVisible(false);
        if (!TimeLapseExport_hook::Available())checkBox_timelapseexport->SetVisible(false);
        if (!ScriptAction_ApplyEffects::Available())checkBox_applyeffects->SetVisible(false);
        if (!CspData::GetHideNavigatorViewIndicatorAvaliable())checkBox_hideNavigatorViewIndicator->SetVisible(false);
    }
    
    checkBox_hsv->SetSelected(AppSettings::GetFunctionEnableHSV());
    checkBox_colorbalance->SetSelected(AppSettings::GetFunctionEnableColorBalance());
    checkBox_tonecurve->SetSelected(AppSettings::GetFunctionEnableToneCurve());
    checkBox_timelapseexport->SetSelected(AppSettings::GetFunctionEnableTimeLapseExport());
    checkBox_applyeffects->SetSelected(AppSettings::GetFunctionEnableScript_ApplyEffects());
    checkBox_hideNavigatorViewIndicator->SetSelected(AppSettings::GetFunctionEnableHideNavigatorViewIndicator());
    //设置回调
    checkBox_hsv->AttachSelect(UiBind(&CheckFunction_Dlg::OnCheckBoxClick, this, std::placeholders::_1));
    checkBox_colorbalance->AttachSelect(UiBind(&CheckFunction_Dlg::OnCheckBoxClick, this, std::placeholders::_1));
    checkBox_tonecurve->AttachSelect(UiBind(&CheckFunction_Dlg::OnCheckBoxClick, this, std::placeholders::_1));
    checkBox_timelapseexport->AttachSelect(UiBind(&CheckFunction_Dlg::OnCheckBoxClick, this, std::placeholders::_1));
    checkBox_applyeffects->AttachSelect(UiBind(&CheckFunction_Dlg::OnCheckBoxClick, this, std::placeholders::_1));
    checkBox_hideNavigatorViewIndicator->AttachSelect(UiBind(&CheckFunction_Dlg::OnCheckBoxClick, this, std::placeholders::_1));
    
    checkBox_hsv->AttachUnSelect(UiBind(&CheckFunction_Dlg::OnCheckBoxClick, this, std::placeholders::_1));
    checkBox_colorbalance->AttachUnSelect(UiBind(&CheckFunction_Dlg::OnCheckBoxClick, this, std::placeholders::_1));
    checkBox_tonecurve->AttachUnSelect(UiBind(&CheckFunction_Dlg::OnCheckBoxClick, this, std::placeholders::_1));
    checkBox_timelapseexport->AttachUnSelect(UiBind(&CheckFunction_Dlg::OnCheckBoxClick, this, std::placeholders::_1));
    checkBox_applyeffects->AttachUnSelect(UiBind(&CheckFunction_Dlg::OnCheckBoxClick, this, std::placeholders::_1));
    checkBox_hideNavigatorViewIndicator->AttachUnSelect(UiBind(&CheckFunction_Dlg::OnCheckBoxClick, this, std::placeholders::_1));

    const auto initPreviewCheckbox = [this](const wchar_t* checkboxName, const wchar_t* iconName,
        bool available, bool selected) {
        if (auto* icon = FindControl(iconName))
            icon->SetVisible(available);
        auto* checkbox = static_cast<ui::CheckBox*>(FindControl(checkboxName));
        if (checkbox)
        {
            if (CspColorTable::GetTheme() == CspColorTable::Light)
                checkbox->SetClass(L"CF_Checkbox_light");
            checkbox->SetVisible(available);
            checkbox->SetSelected(selected);
            checkbox->AttachSelect(UiBind(&CheckFunction_Dlg::OnCheckBoxClick, this, std::placeholders::_1));
            checkbox->AttachUnSelect(UiBind(&CheckFunction_Dlg::OnCheckBoxClick, this, std::placeholders::_1));
        }
        return checkbox;
    };
    checkBox_levels = initPreviewCheckbox(L"checkBox_levels", L"supportedIcon_levels",
        LevelsPreview::Available(), AppSettings::GetFunctionEnableLevelsPreview());
    checkBox_freetransform = initPreviewCheckbox(L"checkBox_freetransform", L"supportedIcon_freetransform",
        FreeTransform::Available(), AppSettings::GetFunctionEnableFreeTransformPreview());


    //其他UI控件
    ui::Button* pBtn_Close = static_cast<ui::Button*>(FindControl(L"btn_Close"));
    if (pBtn_Close)
    {
        pBtn_Close->AttachClick(UiBind(&CheckFunction_Dlg::OnButtonClick, this, std::placeholders::_1));
    }
}

bool CheckFunction_Dlg::OnButtonClick(const ui::EventArgs& args)
{
    DString controlName = args.GetSender()->GetName();
    if (controlName == L"btn_Close" || controlName == L"btn_Cancel")
    {
        this->CloseWnd();
        return true;
    }
    return true;
}

bool CheckFunction_Dlg::OnCheckBoxClick(const ui::EventArgs& args)
{
    if (args.GetSender() == checkBox_hsv)
    {
        AppSettings::SetFunctionEnableHSV(checkBox_hsv->IsSelected());
        return true;
    }
    if (args.GetSender() == checkBox_colorbalance)
    {
        AppSettings::SetFunctionEnableColorBalance(checkBox_colorbalance->IsSelected());
        return true;
    }
    if (args.GetSender() == checkBox_tonecurve)
    {
        AppSettings::SetFunctionEnableToneCurve(checkBox_tonecurve->IsSelected());
        return true;
    }
    if (checkBox_levels && args.GetSender() == checkBox_levels)
    {
        AppSettings::SetFunctionEnableLevelsPreview(checkBox_levels->IsSelected());
        return true;
    }
    if (checkBox_freetransform && args.GetSender() == checkBox_freetransform)
    {
        AppSettings::SetFunctionEnableFreeTransformPreview(checkBox_freetransform->IsSelected());
        return true;
    }
    if (args.GetSender() == checkBox_timelapseexport)
    {
        AppSettings::SetFunctionEnableTimeLapseExport(checkBox_timelapseexport->IsSelected());
        return true;
    }
    if (args.GetSender() == checkBox_applyeffects)
    {
        AppSettings::SetFunctionEnableScript_ApplyEffects(checkBox_applyeffects->IsSelected());
         return true;
    }
    if (args.GetSender() == checkBox_hideNavigatorViewIndicator)
    {
        AppSettings::SetFunctionEnableHideNavigatorViewIndicator(checkBox_hideNavigatorViewIndicator->IsSelected());
         return true;
    }
    

    return true;
}
