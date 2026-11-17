#pragma once
#include<iostream>
#include<map>

//部分颜色信息
//#include"duilib/duilib.h"
#include<SDL3/SDL.h>


class CspData
{
public:

	static void Init();



	static bool GetCurrentProjCanvasSize(uint32_t* sizeW, uint32_t* sizeH);
	static void SetStrData(const char* key, const char* value);
	static const char* GetStrData(const char*,const char* _default);

	//144F61560 +290+20+38+240+378
	//static void GetCurrentSelectLayer();

	//获取CSP主窗口
	static uintptr_t GetNativeWindowHandle();

	static bool GetHideNavigatorViewIndicatorAvaliable() { return navigatorRectPosConvertCodeAddr; };
	static void SetHideNavigatorViewIndicatorEnabled(bool b);

private:

	static int64_t Hook_OnSetPanelsShowStates(const void* object, uintptr_t showStates);//1表示显示，3表示隐藏，应该是第二bit控制
	static inline int64_t(*orig_OnSetPanelsShowStates)(const void* object, uintptr_t showStates) = NULL;



	static inline void* curCanvasSizeBaseAddr = nullptr;
	static inline void* nativeWindowBaseAddr = nullptr;
	static inline void* onSetPanelsShowStates = nullptr;//TAB键控制界面显隐时//5.0.0//0000000141DF2630

	static inline void* navigatorRectPosConvertCodeAddr = nullptr;


	static inline std::map<std::string, std::string> strdata;
};





class CspColorTable
{
private:
	friend class CspData;
	static void Init();

public://0x144f62860 + 8 +0
	enum Theme
	{
		Dark,
		Light
	};
	static Theme GetTheme();

	


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
	static uint32_t _GetColorAt(uintptr_t offset,uint32_t fullback);
	static uint32_t _GetColor96At(uintptr_t offset,uint32_t fullback);//CSP色标颜色其实是92位的... （3个uint32组成的R0 G1 B2颜色.）




	static inline void* colorTableBaseAttr = nullptr;
};
