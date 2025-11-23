#pragma once

#include "duilib/duilib.h"
#include"DUI/CatDui.h"

class ApplyEffectsCheckFailed_Dlg:public NativeModalWindow
{

public:
    static void ShowModalDlg();

    ApplyEffectsCheckFailed_Dlg() {};
    virtual ~ApplyEffectsCheckFailed_Dlg() override {};

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
};

class ApplyEffectsStart_Dlg:public NativeModalWindow
{

public:
    static bool ShowModalDlg();

    ApplyEffectsStart_Dlg(bool* resultOut);
    virtual ~ApplyEffectsStart_Dlg() override;

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
    bool* pResult = NULL;
};

class ApplyEffectsProgress_Dlg:public NativeModalWindow
{

public:
    static void ShowModalDlg(class ScriptAction_ApplyEffects* pScript);

    ApplyEffectsProgress_Dlg(ScriptAction_ApplyEffects* pScript) :pScript(pScript) {};
    virtual ~ApplyEffectsProgress_Dlg() override {};

    /** 资源相关接口
     * GetSkinFolder 接口设置你要绘制的窗口皮肤资源路径
     * GetSkinFile 接口设置你要绘制的窗口的 xml 描述文件
     */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;



    
private:
    void _UpdateDataLoop();




    ScriptAction_ApplyEffects* pScript = NULL;

    ui::Label* pLabel_finishedCount=nullptr;
    ui::Label* pLabel_totalCount = nullptr;
};





class ApplyEffects_Dlg
{


public:
    static void Start();



};
