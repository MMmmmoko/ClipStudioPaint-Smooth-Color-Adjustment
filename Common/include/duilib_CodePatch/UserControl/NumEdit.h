#ifndef _NumEdit_h
#define _NumEdit_h

#include "duilib/duilib.h"
#include "duilib/Control/RichEdit.h"
//TODO  放进duilibpatch而不是自己项目中？

//继承以修复对数字的支持、对拉动改变数字的支持
//需要将RichEdit内部代码的private改为protected
class NumEdit :public ui::RichEdit
{
public:
	NumEdit(ui::Window* pWindow) ;


	//virtual DString GetType() const override { return L"NumEdit"; };

	virtual void SetAttribute(const DString& pstrName, const DString& pstrValue) override;




	void SetDecimalSupport(bool b) { b_supportDecimal = b; };
	void SetMinPrec(int32_t prec) { minPrec=prec; };
	void SetMaxPrec(int32_t prec) { maxPrec = prec; };

	void SetLoseFocusWhenEnter(bool b) { b_loseFocusWhenEnter = b; };

	float GetFloatValue() const;
	double GetDoubleValue() const;
	void SetTextDecimalValue(double value,bool triggerEvent=true);
	void SetTextNumberNoEvent(int32_t value);
protected:

	virtual bool OnKeyDown(const ui::EventArgs& msg) override;
	virtual bool OnKillFocus(const ui::EventArgs& msg) override;

	//SDL版本可能修改的应该是OnInputChar
	virtual bool OnChar(const ui::EventArgs& msg)override;



private:
	bool b_supportDecimal = false;
	bool b_loseFocusWhenEnter = true;
	int32_t minPrec = 0;
	int32_t maxPrec = 3;

};





















#endif