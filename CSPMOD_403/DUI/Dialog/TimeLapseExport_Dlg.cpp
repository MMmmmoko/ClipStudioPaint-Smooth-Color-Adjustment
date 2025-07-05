#include "TimeLapseExport_Dlg.h"

#include"FunctionFix/CspData.h"



bool TimeLapseExport_Dlg::ShowModalDlg(uint32_t* pVideoSize, uint64_t* pVideoDuration)
{
    bool result;

    //û��ʹ��ԭʼ��domodal������������Ҫnew
    TimeLapseExport_Dlg* dialog=new TimeLapseExport_Dlg(pVideoSize, pVideoDuration, &result);
    
    dialog->DoModalNative(CspData::GetNativeWindowHandle());

    return result;
}





TimeLapseExport_Dlg::TimeLapseExport_Dlg(uint32_t* pVideoSize, uint64_t* pVideoDuration, bool* resultOut)
    :pVideoSize(pVideoSize), pVideoDuration(pVideoDuration),pResult(resultOut)
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

    //Ѱ��Ŀ��ؼ�
    pEdit_VideoSize = static_cast<ui::RichEdit*>(FindControl(L"edit_VideoSize"));
    pEdit_VideoDuration = static_cast<ui::RichEdit*>(FindControl(L"edit_VideoDuration"));

    if (pEdit_VideoSize&& pVideoSize)
    {
        pEdit_VideoSize->SetText(std::to_wstring(*pVideoSize));
    }
    if (pEdit_VideoDuration && pVideoDuration)
    {
        pEdit_VideoDuration->SetText(std::to_wstring(*pVideoDuration/1000000000));
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
    ui::Button* pBtn_OK = static_cast<ui::Button*>(FindControl(L"btn_OK"));
    if (pBtn_OK)
    {
        pBtn_OK->AttachClick(UiBind(&TimeLapseExport_Dlg::OnButtonClick, this, std::placeholders::_1));
    }


}



bool TimeLapseExport_Dlg::OnButtonClick(const ui::EventArgs& args)
{
    DString controlName=args.GetSender()->GetName();
    if (controlName == L"btn_Close" || controlName == L"btn_Cancel")
    {
        if (pVideoSize)*pVideoSize = 0;
        if (pVideoDuration)*pVideoDuration = 0;
        this->CloseWnd();
        if (pResult)
            *pResult = false;
        return true;
    }

    if (controlName == L"btn_SetAsCanvasSize")
    {
        if (pEdit_VideoSize)
        {

            uint32_t width, height;
            CspData::GetCurrentProjCanvasSize(&width, &height);
            pEdit_VideoSize->SetText(std::to_wstring(std::max(width, height)));
        }
        return true;
    }

    if (controlName == L"btn_OK")
    {
        if (pEdit_VideoSize&&pVideoSize)
        {
            *pVideoSize=pEdit_VideoSize->GetTextNumber();
        }
        if (pEdit_VideoDuration&&pVideoDuration)
        {
            *pVideoDuration = pEdit_VideoDuration->GetTextNumber()* 1000000000;
        }
        if(pResult)
        *pResult = true;
        this->CloseWnd();
        return true;
    }
    return true;
}

void NativeModalWindow::DoModalNative(uintptr_t nativeWindowHandle_parent)
{
#ifdef _WINDOWS
    //WINDOWS��nativeWindowHandle��HWND
    HWND parent = (HWND)nativeWindowHandle_parent;
    if (!this->IsWindow())
    {
        this->CreateWnd(nullptr,ui::WindowCreateParam(L"CSP Helper Dialog",true));
    }


    if (this->IsWindow())
    {
        HWND child = this->NativeWnd()->GetHWND();
        //���ø�����
        if (parent)
        {
            ::EnableWindow(parent, FALSE);
        }

        //����ֱ�ӵ���SetParent����ʹ��SetWindowLong�����������ߴ��ڣ�owned��
        // �����߹�ϵ�͸��ӹ�ϵ��̫һ�������ӹ�ϵ���ڻ�һ�𱻽���
        //::SetParent(this->NativeWnd()->GetHWND(), parent);
        ::SetWindowLongPtr(child, GWLP_HWNDPARENT, (LONG_PTR)parent);
        ::UpdateWindow(child);
        this->PostQuitMsgWhenClosed(true);
        this->ShowWindow(ui::kSW_SHOW_NORMAL);
        ::SetForegroundWindow(child);
        //�ֲ���Ϣѭ��

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)&& ::IsWindow(child))
        {
            //����IsDialogMessage�������ܲ���������Ϣ ������
            if (true/* || !::IsDialogMessage(child, &msg)*/)
            {
                TranslateMessage(&msg);
               DispatchMessage(&msg);
            }
        }


        if (parent)
        {
            //���ø�����
            ::EnableWindow(parent, TRUE);
            ::SetFocus(parent);
        }
    }
    
   





#endif



#ifdef _MACOS
    ASSERT(false&&"MACOS NOT DEVELOPE YET");







#endif


}
