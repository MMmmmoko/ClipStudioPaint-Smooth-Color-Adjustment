// pch.cpp: 与预编译标头对应的源文件
#include"../HookTool.h"

#include "CspData.h"






void CspData::GetCurrentProjCanvasSize(uint32_t* sizeW, uint32_t* sizeH)
{
//144f61560 + 290 + 20 + 38 +2b8
	uintptr_t pSize = *(uintptr_t*)(CatHook::baseAddr + 0x144f61560);
	pSize = *(uintptr_t*)(pSize+0x290);
	pSize = *(uintptr_t*)(pSize+0x20);
	pSize = *(uintptr_t*)(pSize+0x38);
	pSize = pSize+0x2b8;
	*sizeW = *(uint32_t*)pSize;
	*sizeH = *(uint32_t*)(pSize+4);
}




uintptr_t CspData::GetNativeWindowHandle()
{
	//windows
	//0X144F661A8
	//直接搜索窗口句柄就能搜索到基址
	//这是主窗口地址，不一定是最前台窗口的地址
	return *(uintptr_t*)(0X144F661A8);
}






static ui::UiColor (*orig_ColorManager_ConvertToUiColor)(const DString& colorName) = NULL;
ui::UiColor CspData::Hook_ColorManager_ConvertToUiColor( const DString& colorName)
{
	//当colorName以#开头时才会进入这个函数，所以需要以#开头来命名CSP中的各种颜色
	if (colorName==L"#CSPColor_CaptionNoFocus")
	{
		return ui::UiColor(CspColorTable::GetCaptionContentColorNoFocus());
	}
	if (colorName==L"#CSPColor_CaptionFocus")
	{
		return ui::UiColor(CspColorTable::GetColorCaptionFocus());
	}
	if (colorName==L"#CSPColor_PanelBG")
	{
		return ui::UiColor(CspColorTable::GetColorPanelBG());
	}
	if (colorName==L"#CSPColor_ControlBG")
	{
		return ui::UiColor(CspColorTable::GetColorControlBG());
	}
	if (colorName==L"#CSPColor_Content")
	{
		return ui::UiColor(CspColorTable::GetContentColor());
	}
	if (colorName==L"#CSPColor_Separator")
	{
		return ui::UiColor(CspColorTable::GetSeparatorColor());
	}
	if (colorName==L"#CSPColor_CaptionContent")
	{
		return ui::UiColor(CspColorTable::GetCaptionContentColor());
	}
	if (colorName==L"#CSPColor_CaptionContentNoFocus")
	{
		return ui::UiColor(CspColorTable::GetCaptionContentColorNoFocus());
	}
	

	if (colorName==L"#CSPColor_Selected")
	{
		return ui::UiColor(CspColorTable::GetSelectedColor());
	}
	if (colorName==L"#CSPColor_SelectedHot")
	{
		return ui::UiColor(CspColorTable::GetSelectedHotColor());
	}
	
	if (colorName==L"#CSPColor_BlueButton")
	{
		return ui::UiColor(CspColorTable::GetBlueButtonColor());
	}
	if (colorName==L"#CSPColor_BlueButtonHot")
	{
		return ui::UiColor(CspColorTable::GetBlueButtonHotColor());
	}
	if (colorName==L"#CSPColor_BlueButtonPush")
	{
		return ui::UiColor(CspColorTable::GetBlueButtonPushColor());
	}

	if (colorName==L"#CSPColor_ButtonPush")
	{
		return ui::UiColor(CspColorTable::GetButtonPushColor());
	}





	return orig_ColorManager_ConvertToUiColor(colorName);
}





void CspData::SetUpHook()
{
	//CatHook::AutoHook((void*)(ui::ColorManager::ConvertToUiColor), (void*)CspData::Hook_ColorManager_ConvertToUiColor, (void**)&orig_ColorManager_ConvertToUiColor);
	
	//手动hook.. 使用反汇编读取的目标函数值和VS调试器的内存或反汇编匹配不上
	//读取的结果为一堆jmp指令，且并没有jmp到目标函数，有点摸不到头脑..
	//用下方这段代码后再反汇编得到的却是jmp表.....
	//CatHook::Hook((void*)(ui::ColorManager::ConvertToUiColor), (void*)CspData::Hook_ColorManager_ConvertToUiColor, (void**)&orig_ColorManager_ConvertToUiColor,18);


	//询问Ai并检查后发现VS其实读取到的也是检查表，只是它帮我贴心跳转了..
	//跳转到目标函数位置时可以发现输入的地址和实际地址是不一致的。
	//对这种静态库跳转表进行识别处理一下就可以继续hook了.
	CatHook::AutoHook((void*)(ui::ColorManager::ConvertToUiColor), (void*)CspData::Hook_ColorManager_ConvertToUiColor, (void**)&orig_ColorManager_ConvertToUiColor);
}






uintptr_t CspColorTable::GetColorTableAddr()
{
	//0x144f62860 + 8 +0
	uintptr_t addr= *(uintptr_t*)(CatHook::baseAddr + 0x144f62860);
	addr = *(uintptr_t*)(addr + 0x8);
	//addr = *(uintptr_t*)(addr + 0);
	return addr;
}




uint32_t CspColorTable::GetColorCaptionNoFocus()
{
	return _GetColorAt(0X288);
}
uint32_t CspColorTable::GetColorCaptionFocus()
{
	return _GetColorAt(0X24);
}

uint32_t CspColorTable::GetColorPanelBG()
{
	return _GetColorAt(0x48);
}
uint32_t CspColorTable::GetColorControlBG()
{
	return _GetColorAt(0x144);
}

uint32_t CspColorTable::GetContentColor()
{
	return _GetColorAt(0x120);
}
uint32_t CspColorTable::GetSeparatorColor()
{
	return _GetColorAt(0x264);
}
uint32_t CspColorTable::GetCaptionContentColor()
{
	return _GetColorAt(0x18);
}

uint32_t CspColorTable::GetCaptionContentColorNoFocus()
{
	return _GetColorAt(0x280);
}



uint32_t CspColorTable::GetSelectedColor()
{
	return 0x3073B2E6;//找不到，手动选个相近的颜色
}
uint32_t CspColorTable::GetSelectedHotColor()
{
	return 0xE0C7DFFA;
}

uint32_t CspColorTable::GetBlueButtonColor()
{
	return 0xE073B2E6;
}
uint32_t CspColorTable::GetBlueButtonHotColor()
{
	return 0xE0A1C7E6;
}
uint32_t CspColorTable::GetBlueButtonPushColor()
{
	return 0x300036A2;
}
uint32_t CspColorTable::GetButtonPushColor()
{
	return 0x800036A2;
}

uint32_t CspColorTable::_GetColorAt(uintptr_t offset)
{
	uint32_t resultColor = 0xFFFFFFFF;
	uintptr_t pColor =GetColorTableAddr();
	pColor += offset;
	memcpy(&resultColor, (void*)pColor,3);
	return resultColor;
}
