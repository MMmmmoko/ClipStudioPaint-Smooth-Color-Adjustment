#include "CSPHelper.h"
#include"CSPMOD.h"
#include"AddressTable.h"
#include"LayerObject.h"
#include"DUI/Dialog/ApplyEffects_Dlg.h"
#include"duilib/duilib.h"
#include"CspData.h"

#include"DUI/Dialog/CheckFunction_Dlg.h"
#include"DUI/Dialog/About_Dlg.h"

void CSPHelper::Init()
{
	void* funcAddr= AddressTable::GetAddress("CSPHelper_Entrance");
	if(funcAddr)
		CSPMOD::Hook(funcAddr, OnClickEntrance,(void**)&orig_OnClickEntrance);
}




//重要！！！！！！！！！
//LayerObject在原CSP实现中是有引用计数的共享指针.
//在调用原CSP函数的时候，需要所有有引用计数的入参增加引用计数！！

void CSPHelper::OnClickEntrance()
{


    //auto curLayer = LayerObject::GetCurrentLayer();
    //printf("CurLayerIsAjdustLayer:%d\n", *(uint8_t*)(curLayer.ptr + 0x10)&0x01);
    //return;




    POINT p = {};
    ::GetCursorPos(&p);


    //右键弹出菜单
    ui::Menu* menu = new ui::Menu(nullptr);
    menu->SetSkinFolder(L"cspHelper_default");
    DString xml(L"HelperMenu.xml");


 

    menu->ShowMenu(xml, { p.x-10,p.y+4 });
    HWND child = menu->NativeWnd()->GetHWND();
    ::SetWindowLongPtr(child, GWLP_HWNDPARENT, (LONG_PTR)CspData::GetNativeWindowHandle());
    ::UpdateWindow(child);

    ui::MenuItem* helperMenu_applyEffects = dynamic_cast<ui::MenuItem*>(menu->FindControl(L"helperMenu_applyEffects"));
    ui::MenuItem* helperMenu_checkFunctions = dynamic_cast<ui::MenuItem*>(menu->FindControl(L"helperMenu_checkFunctions"));
    ui::MenuItem* helperMenu_about = dynamic_cast<ui::MenuItem*>(menu->FindControl(L"helperMenu_about"));
    
    ui::UiSize sizeMax(9999, 9999);
    int32_t maxW = 0;
    if (helperMenu_applyEffects)
    {
        int32_t curW =helperMenu_applyEffects->EstimateSize(sizeMax).cx.GetInt32();
        if (curW > maxW)maxW = curW;
    }
    if (helperMenu_checkFunctions)
    {
        int32_t curW = helperMenu_checkFunctions->EstimateSize(sizeMax).cx.GetInt32();
        if (curW > maxW)maxW = curW;
    }
    if (helperMenu_about)
    {
        int32_t curW = helperMenu_about->EstimateSize(sizeMax).cx.GetInt32();
        if (curW > maxW)maxW = curW;
    }
    helperMenu_applyEffects->SetFixedWidth(ui::UiFixedInt(maxW), true, false);
    helperMenu_checkFunctions->SetFixedWidth(ui::UiFixedInt(maxW), true, false);
    helperMenu_about->SetFixedWidth(ui::UiFixedInt(maxW), true, false);




    ////设置部分项目的显隐
    //if (AppSettings::GetIns().GetWindowTransparent())
    //{
    //    if (windowMenu_hideWindowBorder)windowMenu_hideWindowBorder->SetVisible(false);
    //}
    //else
    //{
    //    if (windowMenu_showWindowBorder)windowMenu_showWindowBorder->SetVisible(false);
    //}


    //添加按钮功能
    if (helperMenu_applyEffects)helperMenu_applyEffects->AttachClick(
        [](const ui::EventArgs&)->bool {
            ApplyEffects_Dlg::Start();
            return true;
        });
    if (helperMenu_checkFunctions)helperMenu_checkFunctions->AttachClick(
        [](const ui::EventArgs&)->bool {
            CheckFunction_Dlg::ShowModalDlg();
            return true;
        });
    if (helperMenu_about)helperMenu_about->AttachClick(
        [](const ui::EventArgs&)->bool {
            About_Dlg::ShowModalDlg();
            return true;
        });













	//ScriptAction_ApplyEffects script;
	//if(script.CheckHasEffects())
	//	script.DoAction();
	
}
