#ifndef _PowSlider_h
#define _PowSlider_h



#include "duilib/duilib.h"


//对数滑块 非线性滑块
class PowSlider :public ui::Slider
{
public:
	PowSlider(ui::Window* pWindow);

	virtual void SetAttribute(const DString& strName, const DString& strValue) override;
	void SetLeftValue(double val) { left = val; CalcParam(); };
	void SetRightValue(double val) { right = val; CalcParam(); };
	void SetMidValue(double val) { mid = val; CalcParam(); };
	void SetRealValue(double value);
	double GetRealValue()const;

private:
	void SetValue(double value)=delete;
	double GetValue() const= delete;
	void CalcParam();


	double left=0.1;
	double mid=1.;
	double right=10.;
	//正向Y=A*B^X+C  X为-1~1
	double A=1.;
	double B=10.;
	double C=0.;
};





















#endif