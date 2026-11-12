#include "CSPHelper.h"
#include"CSPMOD.h"
#include"AddressTable.h"
#include"LayerObject.h"
#include"DUI/Dialog/ApplyEffects_Dlg.h"
#include"duilib/duilib.h"
#include"CspData.h"
#include"CustomAdjustmentLayer.h"
#include"TestInvertAdjustmentLayer.h"

#include"DUI/Dialog/CheckFunction_Dlg.h"
#include"DUI/Dialog/About_Dlg.h"
#include"DUI/Dialog/ColWheelWnd.h"
#include"ScriptAction_ApplyEffects.h"
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
    const auto host = reinterpret_cast<HWND>(CspData::GetNativeWindowHandle());
    ::SetWindowLongPtr(child, GWLP_HWNDPARENT, reinterpret_cast<LONG_PTR>(host));
    ::UpdateWindow(child);

    ui::MenuItem* helperMenu_applyEffects = dynamic_cast<ui::MenuItem*>(menu->FindControl(L"helperMenu_applyEffects"));
    ui::MenuItem* helperMenu_checkFunctions = dynamic_cast<ui::MenuItem*>(menu->FindControl(L"helperMenu_checkFunctions"));
    ui::MenuItem* helperMenu_about = dynamic_cast<ui::MenuItem*>(menu->FindControl(L"helperMenu_about"));

    ui::MenuItem* helperMenu_colorWheel = dynamic_cast<ui::MenuItem*>(menu->FindControl(L"helperMenu_colorWheel"));
    auto* customAdjustment = dynamic_cast<ui::MenuItem*>(menu->FindControl(L"helperMenu_customAdjustment"));
    auto* editCustomAdjustment = dynamic_cast<ui::MenuItem*>(menu->FindControl(L"helperMenu_editCustomAdjustment"));
    
    ui::UiSize sizeMax(9999, 9999);
    int32_t maxW = 0;
    if (helperMenu_applyEffects)
    {
        int32_t curW =helperMenu_applyEffects->EstimateSize(sizeMax).cx.GetInt32();
        if (curW > maxW)maxW = curW;
        if (!ScriptAction_ApplyEffects::IsEnabled())
            helperMenu_applyEffects->SetEnabled(false);
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
    if (helperMenu_colorWheel)
    {
        int32_t curW = helperMenu_colorWheel->EstimateSize(sizeMax).cx.GetInt32();
        if (curW > maxW)maxW = curW;
    }
    const bool adjustmentAvailable = (customAdjustment || editCustomAdjustment) && CustomAdjustmentLayer::PrepareMenu(host);
    const auto bindAdjustment = [&](ui::MenuItem* item, void (*command)(HWND, uint32_t), bool enabled)
    {
        if (!item)
            return;
        item->SetVisible(adjustmentAvailable);
        if (!adjustmentAvailable)
            return;
        item->SetEnabled(enabled);
        maxW = (std::max)(maxW, item->EstimateSize(sizeMax).cx.GetInt32());
        item->AttachClick([host, command](const ui::EventArgs&) {
            command(host, TestInvertAdjustment::Type);
            return true;
        });
    };
    bindAdjustment(customAdjustment, CustomAdjustmentLayer::RequestCreate, adjustmentAvailable);
    bindAdjustment(editCustomAdjustment, CustomAdjustmentLayer::RequestEdit,
        adjustmentAvailable && CustomAdjustmentLayer::CanEdit(TestInvertAdjustment::Type));
    if (adjustmentAvailable)
    {
        if (customAdjustment)
            customAdjustment->SetFixedWidth(ui::UiFixedInt(maxW), true, false);
        if (editCustomAdjustment)
            editCustomAdjustment->SetFixedWidth(ui::UiFixedInt(maxW), true, false);
    }

    helperMenu_applyEffects->SetFixedWidth(ui::UiFixedInt(maxW), true, false);
    helperMenu_checkFunctions->SetFixedWidth(ui::UiFixedInt(maxW), true, false);
    helperMenu_about->SetFixedWidth(ui::UiFixedInt(maxW), true, false);
    helperMenu_colorWheel->SetFixedWidth(ui::UiFixedInt(maxW), true, false);


    helperMenu_colorWheel->SetVisible(isProUser());

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
    if (helperMenu_colorWheel)helperMenu_colorWheel->AttachClick(
        [](const ui::EventArgs&)->bool {
            ColWheelWnd::ShowForm();
            return true;
        });













	//ScriptAction_ApplyEffects script;
	//if(script.CheckHasEffects())
	//	script.DoAction();
	
}
