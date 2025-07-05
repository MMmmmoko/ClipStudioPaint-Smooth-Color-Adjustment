#ifndef TimeLapseExport_Dlg_h
#define TimeLapseExport_Dlg_h


#include "duilib/duilib.h"



//�ڲ��ƻ�nimdui��װ�������ʵ��modal
//
class NativeModalWindow :public ui::WindowImplBase
{
public:
    void DoModalNative(uintptr_t nativeWindowHandle);
};






class TimeLapseExport_Dlg :public NativeModalWindow
{


public:
    static bool ShowModalDlg(uint32_t* pVideoSize, uint64_t* pVideoDuration);






    TimeLapseExport_Dlg(uint32_t* pVideoSize,uint64_t* pVideoDuration,bool* resultOut=nullptr);
    virtual ~TimeLapseExport_Dlg() override;

    /** ��Դ��ؽӿ�
     * GetSkinFolder �ӿ�������Ҫ���ƵĴ���Ƥ����Դ·��
     * GetSkinFile �ӿ�������Ҫ���ƵĴ��ڵ� xml �����ļ�
     */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** �����ڴ�������Ժ���ô˺���������������һЩ��ʼ���Ĺ���
    */
    virtual void OnInitWindow() override;



    bool OnButtonClick(const ui::EventArgs& args);



private:
    ui::RichEdit* pEdit_VideoSize=NULL;
    ui::RichEdit* pEdit_VideoDuration = NULL;
    uint32_t* pVideoSize;
    uint64_t* pVideoDuration;
    bool* pResult = NULL;

};

#endif