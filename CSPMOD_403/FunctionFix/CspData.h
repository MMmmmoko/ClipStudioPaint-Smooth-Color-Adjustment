
#ifndef CspData_h
#define CspData_h

#include<iostream>


//部分颜色信息
#include"duilib/duilib.h"



class CspData
{
public:
	//static CspData& GetIns() { static CspData ref; return ref; };

	static void GetCurrentProjCanvasSize(uint32_t* sizeW, uint32_t* sizeH);

	//144F61560 +290+20+38+240+378
	//static void GetCurrentSelectLayer();

	//获取CSP主窗口
	static uintptr_t GetNativeWindowHandle();




	//某些信息的获取或者修改使用hook更佳
	static void SetUpHook();


private:
	static ui::UiColor Hook_ColorManager_ConvertToUiColor(const DString& colorName);
};




class CspColorTable
{
public://0x144f62860 + 8 +0
	//神色模式调最高后搜索00 00 00 00 00 00 00 00 00 00 00 00 69 69 69 69 69 69 69 69 69 69 69 69 C2 C2 C2 C2 C2 C2 C2 C2 C2 C2 C2 C2
	static uintptr_t GetColorTableAddr();


	//使用的DUI库的颜色从低到高位是BGRA
	//+0X288
	static uint32_t GetColorCaptionNoFocus();
	//+0X24
	static uint32_t GetColorCaptionFocus();

	//对话框面板基础底色 +0x48
	static uint32_t GetColorPanelBG();

	//按钮、编辑框、复选框等控件的背景色 +0x144
	static uint32_t GetColorControlBG();

	//正常文字、单选框复选框中心色+0x120
	static uint32_t GetContentColor();

	//分割线 +0x264
	static uint32_t GetSeparatorColor();

	//标题栏文字颜色+0x18
	static uint32_t GetCaptionContentColor();

	//标题栏文字失焦颜色+0x280
	static uint32_t GetCaptionContentColorNoFocus();

	//标题栏控件颜色  无 标题栏的关闭、最小化按钮等控件是直接使用的图片图标，不会随着界面调整而变化



	//列表控件、非确定按钮的颜色。
	static uint32_t GetSelectedColor();
	static uint32_t GetSelectedHotColor();

	static uint32_t GetBlueButtonColor();
	static uint32_t GetBlueButtonHotColor();
	static uint32_t GetBlueButtonPushColor();
	static uint32_t GetButtonPushColor();

private:
	static uint32_t _GetColorAt(uintptr_t offset);


};












#endif //PCH_H
