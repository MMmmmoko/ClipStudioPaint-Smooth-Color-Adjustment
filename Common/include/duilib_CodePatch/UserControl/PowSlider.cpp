
#include"PowSlider.h"

using namespace ui;

PowSlider::PowSlider(ui::Window* pWindow)
:ui::Slider(pWindow) {
	SetMaxValue(10000);
	SetMinValue(-10000);
	Slider::SetChangeStep(200);
	//CalcParam();
}

void PowSlider::SetAttribute(const DString& strName, const DString& strValue)
{
	//屏蔽部分参数
	if (strName == _T("step")) {
	}else if (strName == _T("min")) {

		SetLeftValue(StringUtil::StringToDouble(strValue));
    }
	else if (strName == _T("mid")) {

		SetMidValue(StringUtil::StringToDouble(strValue));
    }
    else if (strName == _T("max")) {
		SetRightValue(StringUtil::StringToDouble(strValue));
    }
    else if (strName == _T("value")) {
		SetRealValue(StringUtil::StringToDouble(strValue));
    }
	else {
		Slider::SetAttribute(strName, strValue);
	}
}

void PowSlider::SetRealValue(double value)
{
	double baseValue = log((value - C) / A) / log(B);
	ui::Progress::SetValue(baseValue*10000.);
}

double PowSlider::GetRealValue()const
{
	double baseValue = ui::Progress::GetValue()/10000.;

	double resultValue = A * pow(B, baseValue)+C;
	return resultValue;
}



void PowSlider::CalcParam()
{
	if (left > mid || mid > right || left > right)return;

	B = (right - mid) / (mid - left);
	A = (mid - left) * B / (B - 1.);
	C = mid - A;

}
