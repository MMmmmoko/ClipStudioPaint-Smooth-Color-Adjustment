#pragma once

#include "duilib/duilib.h"
#include"DUI/CatDui.h"

class About_Dlg:public NativeModalWindow
{

public:
    static void ShowModalDlg();

    About_Dlg() {};
    virtual ~About_Dlg() override {};

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
    bool OnLinkClick(const ui::EventArgs& args);
};
