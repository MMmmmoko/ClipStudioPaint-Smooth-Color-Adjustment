#pragma once
#include"DUI/CatDui.h"
#include "duilib/duilib.h"




class TimeLapseExport_Dlg :public NativeModalWindow
{


public:
    static bool ShowModalDlg(uint32_t* pVideoSize, uint64_t* pVideoDuration);






    TimeLapseExport_Dlg(uint32_t* pVideoSize, uint64_t* pVideoDuration, bool* resultOut = nullptr);
    virtual ~TimeLapseExport_Dlg() override;

    /** 资源相关接口
     * GetSkinFolder 接口设置你要绘制的窗口皮肤资源路径
     * GetSkinFile 接口设置你要绘制的窗口的 xml 描述文件
     */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;



    bool OnButtonClick(const ui::EventArgs& args);



private:
    ui::RichEdit* pEdit_VideoSize = NULL;
    ui::RichEdit* pEdit_VideoDuration = NULL;
    uint32_t* pVideoSize;
    uint64_t* pVideoDuration;
    bool* pResult = NULL;

};
