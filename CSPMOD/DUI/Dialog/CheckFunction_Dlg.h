#pragma once

#include "duilib/duilib.h"
#include"DUI/CatDui.h"



class CheckFunction_Dlg :public NativeModalWindow
{


public:
    static void ShowModalDlg();






    CheckFunction_Dlg();
    virtual ~CheckFunction_Dlg() override;

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
    bool OnCheckBoxClick(const ui::EventArgs& args);



private:
    ui::CheckBox* checkBox_hsv;
    ui::CheckBox* checkBox_colorbalance;
    ui::CheckBox* checkBox_tonecurve;
    ui::CheckBox* checkBox_plugin_gaussblur;
    ui::CheckBox* checkBox_plugin_motionblur;
    ui::CheckBox* checkBox_timelapseexport;
    ui::CheckBox* checkBox_applyeffects;

};
