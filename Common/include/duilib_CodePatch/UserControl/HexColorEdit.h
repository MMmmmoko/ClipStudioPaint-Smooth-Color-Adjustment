#ifndef _HexColorEdit_h
#define _HexColorEdit_h

#include "duilib/duilib.h"
//TODO  放进duilibpatch而不是自己项目中？

//继承以修复对数字的支持、对拉动改变数字的支持
//需要将RichEdit内部代码的private改为protected
class HexColorEdit :public ui::RichEdit
{
public:
	HexColorEdit(ui::Window* pWindow) ;


	//virtual DString GetType() const override { return L"NumEdit"; };

	virtual void SetAttribute(const DString& pstrName, const DString& pstrValue) override;




	void SetLoseFocusWhenEnter(bool b) { b_loseFocusWhenEnter = b; };
	void SetUseAlpha(bool b);

	bool GetColorValue(ui::UiColor& valueOut);
protected:

	virtual bool OnKeyDown(const ui::EventArgs& msg) override;
	virtual bool OnKillFocus(const ui::EventArgs& msg) override;

	//SDL版本可能修改的应该是OnInputChar
	//virtual bool OnChar(const ui::EventArgs& msg)override;
private:
	std::wstring TextFix(const std::wstring& text);


private:
	bool b_loseFocusWhenEnter = true;
	bool b_useAlpha = true;
};





















#endif