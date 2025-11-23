#include"CSPMOD.h"
#include "CspData.h"
#include"AddressTable.h"
#include"duilib/duilib.h"
#include"DUI/CatDuiThread.h"





void CspData::Init()
{
	curCanvasSizeBaseAddr = AddressTable::GetAddress("CSPInfo_CanvasSize");
	nativeWindowBaseAddr = AddressTable::GetAddress("CSPInfo_NativeWindowHandle");







	CspColorTable::Init();

}

bool CspData::GetCurrentProjCanvasSize(uint32_t* sizeW, uint32_t* sizeH)
{


	if (!curCanvasSizeBaseAddr)return false;
	uintptr_t pSize= *(uintptr_t*)(curCanvasSizeBaseAddr);
	if (!CSPMOD::IsPtrValid(pSize + 0x290))return false;
	pSize= *(uintptr_t*)(pSize + 0x290);
	if (!CSPMOD::IsPtrValid(pSize + 0x20))return false;
	pSize= *(uintptr_t*)(pSize + 0x20);
	if (!CSPMOD::IsPtrValid(pSize + 0x38))return false;
	pSize= *(uintptr_t*)(pSize + 0x38);

	pSize = pSize + 0x2B8;
	if (!CSPMOD::IsPtrValid(pSize))return false;

	*sizeW = *(uint32_t*)pSize;
	*sizeH = *(uint32_t*)(pSize+4);
	return true;
}

void CspData::SetStrData(const char* key, const char* value)
{
	strdata[key] = value;
}

const char* CspData::GetStrData(const char* key, const char* _default)
{
	auto it = strdata.find(key);
	if (it != strdata.end())
	{
		return it->second.c_str();
	}
	return _default;
}

uintptr_t CspData::GetNativeWindowHandle()
{
	if (nativeWindowBaseAddr)
	{
		return *(uintptr_t*)nativeWindowBaseAddr;
	}
	return 0;
}












static ui::UiColor(*orig_ColorManager_ConvertToUiColor)(const DString& colorName) = nullptr;
static ui::UiColor Hook_ColorManager_ConvertToUiColor(const DString& colorName)
{
	if (colorName[0] == L'#'&& colorName[1]==L'C')
	{
		if (colorName == L"#CSPColor_CaptionNoFocus")
		{
			return ui::UiColor(CspColorTable::GetCaptionContentColorNoFocus());
		}
		if (colorName == L"#CSPColor_CaptionFocus")
		{
			return ui::UiColor(CspColorTable::GetColorCaptionFocus());
		}
		if (colorName == L"#CSPColor_PanelBG")
		{
			return ui::UiColor(CspColorTable::GetColorPanelBG());
		}
		if (colorName == L"#CSPColor_ControlBG")
		{
			return ui::UiColor(CspColorTable::GetColorControlBG());
		}
		if (colorName == L"#CSPColor_Content")
		{
			return ui::UiColor(CspColorTable::GetContentColor());
		}
		if (colorName == L"#CSPColor_Separator")
		{
			return ui::UiColor(CspColorTable::GetSeparatorColor());
		}
		if (colorName == L"#CSPColor_CaptionContent")
		{
			return ui::UiColor(CspColorTable::GetCaptionContentColor());
		}
		if (colorName == L"#CSPColor_CaptionContentNoFocus")
		{
			return ui::UiColor(CspColorTable::GetCaptionContentColorNoFocus());
		}


		if (colorName == L"#CSPColor_Selected")
		{
			return ui::UiColor(CspColorTable::GetSelectedColor());
		}
		if (colorName == L"#CSPColor_SelectedHot")
		{
			return ui::UiColor(CspColorTable::GetSelectedHotColor());
		}

		if (colorName == L"#CSPColor_BlueButton")
		{
			return ui::UiColor(CspColorTable::GetBlueButtonColor());
		}
		if (colorName == L"#CSPColor_BlueButtonHot")
		{
			return ui::UiColor(CspColorTable::GetBlueButtonHotColor());
		}
		if (colorName == L"#CSPColor_BlueButtonPush")
		{
			return ui::UiColor(CspColorTable::GetBlueButtonPushColor());
		}

		if (colorName == L"#CSPColor_ButtonPush")
		{
			return ui::UiColor(CspColorTable::GetButtonPushColor());
		}

	}
	return orig_ColorManager_ConvertToUiColor(colorName);
}


void CspColorTable::Init()
{

	colorTableBaseAttr = AddressTable::GetAddress("CSPInfo_UIColorTable");
	//无论如何进行一个内部hook
	CSPMOD::Hook((void*)(ui::ColorManager::ConvertToUiColor), Hook_ColorManager_ConvertToUiColor, (void**)&orig_ColorManager_ConvertToUiColor);

}







CspColorTable::Theme CspColorTable::GetTheme()
{
	auto color = GetContentColor()&0xFF;
	if (color > 0x88)
		return Theme::Dark;
	else
		return Theme::Light;
}

uintptr_t CspColorTable::GetColorTableAddr()
{
	//0x144f62860 + 8 +0

	if(!CSPMOD::IsPtrValid(colorTableBaseAttr))return 0;

	uintptr_t addr = *(uintptr_t*)(colorTableBaseAttr);
	if (!CSPMOD::IsPtrValid(addr + 0x8))return 0;
	addr = *(uintptr_t*)(addr + 0x8);
	if (!CSPMOD::IsPtrValid(addr))return 0;
	return addr;
}



//DUI int 颜色 int32 ARGB //字节序为BGRA
uint32_t CspColorTable::GetColorCaptionNoFocus()
{
	
	return _GetColorAt(0X288,0xFF474747);
}
uint32_t CspColorTable::GetColorCaptionFocus()
{
	return _GetColorAt(0X24,0XFF383838);
}

uint32_t CspColorTable::GetColorPanelBG()
{
	return _GetColorAt(0x48,0XFF4E4E4E);
}
uint32_t CspColorTable::GetColorControlBG()
{
	return _GetColorAt(0xF0, 0XFF676767);
}

uint32_t CspColorTable::GetContentColor()
{
	return _GetColorAt(0x120,0XFFDADADA);
}
uint32_t CspColorTable::GetSeparatorColor()
{
	return _GetColorAt(0x264,0XFF727272);
}
uint32_t CspColorTable::GetCaptionContentColor()
{
	return _GetColorAt(0x18,0XFFA1A1A1);
}

uint32_t CspColorTable::GetCaptionContentColorNoFocus()
{
	return _GetColorAt(0x280,0XFF626262);
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

uint32_t CspColorTable::_GetColorAt(uintptr_t offset, uint32_t fullback)
{
	uint32_t resultColor = 0xFFFFFFFF;
	uintptr_t pColor = GetColorTableAddr();
	if (0 == pColor|| !CSPMOD::IsPtrValid(pColor+offset))
	{
		return fullback;
	}

	pColor += offset;
	memcpy(&resultColor, (void*)pColor, 3);
	return resultColor;
}
