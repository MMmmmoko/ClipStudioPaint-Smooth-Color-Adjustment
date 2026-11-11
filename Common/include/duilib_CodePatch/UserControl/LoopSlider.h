#ifndef _LoopSlider_h
#define _LoopSlider_h



#include "duilib/duilib.h"
//继承以修复对数字的支持、对拉动改变数字的支持
//需要将RichEdit内部代码的private改为protected
class LoopSlider :public ui::Slider
{
public:
	LoopSlider(ui::Window* pWindow) :ui::Slider(pWindow) {};
	virtual void HandleEvent(const ui::EventArgs& msg) override;
};





















#endif