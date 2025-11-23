#include "TimeLapseExport_Dlg.h"
#include"CspData.h"
#include"DUI/CatDuiThread.h"


bool TimeLapseExport_Dlg::ShowModalDlg(uint32_t* pVideoSize, uint64_t* pVideoDuration)
{
    bool result;

    //没有使用原始的domodal函数，所以需要new
    TimeLapseExport_Dlg* dialog = new TimeLapseExport_Dlg(pVideoSize, pVideoDuration, &result);

    dialog->DoModalNative(CspData::GetNativeWindowHandle());

    return result;
}





TimeLapseExport_Dlg::TimeLapseExport_Dlg(uint32_t* pVideoSize, uint64_t* pVideoDuration, bool* resultOut)
    :pVideoSize(pVideoSize), pVideoDuration(pVideoDuration), pResult(resultOut)
{

}

TimeLapseExport_Dlg::~TimeLapseExport_Dlg()
{
}

DString TimeLapseExport_Dlg::GetSkinFolder()
{
    return L"cspHelper_default";
}

DString TimeLapseExport_Dlg::GetSkinFile()
{
    return L"TimeLapseExport_Dlg_.xml";
}



void TimeLapseExport_Dlg::OnInitWindow()
{
    __super::OnInitWindow();

    //寻找目标控件
    pEdit_VideoSize = static_cast<ui::RichEdit*>(FindControl(L"edit_VideoSize"));
    pEdit_VideoDuration = static_cast<ui::RichEdit*>(FindControl(L"edit_VideoDuration"));

    if (pEdit_VideoSize && pVideoSize)
    {
        pEdit_VideoSize->SetText(std::to_wstring(*pVideoSize));
    }
    if (pEdit_VideoDuration && pVideoDuration)
    {
        pEdit_VideoDuration->SetText(std::to_wstring(*pVideoDuration / 1000'000'000));
    }



    ui::Button* pBtn_Close = static_cast<ui::Button*>(FindControl(L"btn_Close"));
    if (pBtn_Close)
    {
        pBtn_Close->AttachClick(UiBind(&TimeLapseExport_Dlg::OnButtonClick, this, std::placeholders::_1));
    }
    ui::Button* pBtn_Cancel = static_cast<ui::Button*>(FindControl(L"btn_Cancel"));
    if (pBtn_Cancel)
    {
        pBtn_Cancel->AttachClick(UiBind(&TimeLapseExport_Dlg::OnButtonClick, this, std::placeholders::_1));
    }
    ui::Button* pBtn_SetSize = static_cast<ui::Button*>(FindControl(L"btn_SetAsCanvasSize"));
    if (pBtn_SetSize)
    {
        pBtn_SetSize->AttachClick(UiBind(&TimeLapseExport_Dlg::OnButtonClick, this, std::placeholders::_1));
    }
    uint32_t width, height;
    if (!CspData::GetCurrentProjCanvasSize(&width, &height)) { pBtn_SetSize->SetEnabled(false); }


    ui::Button* pBtn_OK = static_cast<ui::Button*>(FindControl(L"btn_OK"));
    if (pBtn_OK)
    {
        pBtn_OK->AttachClick(UiBind(&TimeLapseExport_Dlg::OnButtonClick, this, std::placeholders::_1));
    }









    //界面优化用
    ui::Label* pLabel_VideoSize = static_cast<ui::Label*>(FindControl(L"label_videoSize"));
    ui::Label* pLabel_VideoDuration = static_cast<ui::Label*>(FindControl(L"label_videoDuration"));

    ui::UiSize sizeMax(9999,9999);
    int32_t maxW=pLabel_VideoSize->EstimateSize(sizeMax).cx.GetInt32();
    int32_t curW = pLabel_VideoDuration->EstimateSize(sizeMax).cx.GetInt32();
    if (curW > maxW)maxW = curW;
    pLabel_VideoSize->SetFixedWidth(ui::UiFixedInt( maxW),true,false);
    pLabel_VideoDuration->SetFixedWidth(ui::UiFixedInt(maxW),true,false);
}



bool TimeLapseExport_Dlg::OnButtonClick(const ui::EventArgs& args)
{
    DString controlName = args.GetSender()->GetName();
    if (controlName == L"btn_Close" || controlName == L"btn_Cancel")
    {
        if (pVideoSize)*pVideoSize = 0;
        if (pVideoDuration)*pVideoDuration = 0;
        if (pResult)
            *pResult = false;
        this->CloseWnd();
        return true;
    }

    if (controlName == L"btn_SetAsCanvasSize")
    {
        if (pEdit_VideoSize)
        {

            uint32_t width, height;
            if (CspData::GetCurrentProjCanvasSize(&width, &height))
            {
                pEdit_VideoSize->SetText(std::to_wstring(std::max(width, height)));
            }
            else
            {
                
                
            }
        }
        return true;
    }

    if (controlName == L"btn_OK")
    {
        uint32_t _videoSize = 0;
        uint64_t _videoDuration = 0;
        if (pEdit_VideoSize && pVideoSize)
        {
            _videoSize = static_cast<uint32_t>(pEdit_VideoSize->GetTextNumber());
        }
        if (pEdit_VideoDuration && pVideoDuration)
        {
            _videoDuration = pEdit_VideoDuration->GetTextNumber() * 1000'000'000;
        }
        if(_videoSize==0|| _videoDuration==0)
        {
            *pResult = false;
            this->CloseWnd();
            return true;
        }

        if (pVideoSize)*pVideoSize = _videoSize;
        if (pVideoDuration)*pVideoDuration = _videoDuration;

        if (pResult)
            *pResult = true;
        this->CloseWnd();
        return true;
    }
    return true;
}

