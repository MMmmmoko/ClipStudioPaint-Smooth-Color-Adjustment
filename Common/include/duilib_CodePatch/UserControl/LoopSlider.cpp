
#include"LoopSlider.h"

using namespace ui;
void LoopSlider::HandleEvent(const ui::EventArgs& msg)
{
    constexpr bool isLoop = true;



    if (IsDisabledEvents(msg)) {
        //如果是鼠标键盘消息，并且控件是Disabled的，转发给上层控件
        Box* pParent = GetParent();
        if (pParent != nullptr) {
            pParent->SendEventMsg(msg);
        }
        else {
            Progress::HandleEvent(msg);
        }
        return;
    }
    if (msg.eventType == kEventMouseButtonDown || msg.eventType == kEventMouseDoubleClick) {
        UiPoint newPtMouse(msg.ptMouse);
        newPtMouse.Offset(GetScrollOffsetInScrollBox());
        //UiRect rcThumb = GetThumbRect();
        //if (rcThumb.ContainsPt(newPtMouse)) {
        //    SetMouseFocused(true);
        //}
        //无论是否点到滑块均设置鼠标焦点
        SetMouseFocused(true);
        return;
    }
    if (msg.eventType == kEventMouseButtonUp) {
        UiSize szThumb = GetThumbSize();
        double oldValue = GetValue();
        if (IsMouseFocused()) {
            SetMouseFocused(false);
        }
        const int32_t nMin = GetMinValue();
        const int32_t nMax = GetMaxValue();

        if (IsHorizontal()) {
            if (isLoop)
            {
                //鼠标在控件范围内时，factor为0~1，这里factor可以表示鼠标在控件范围外的情况
                double factor = double(msg.ptMouse.x - GetRect().left - szThumb.cx / 2) / (GetRect().right - GetRect().left - szThumb.cx);
                //得到真实的0~1的factor
                factor = factor - floor(factor);
                double newValue = nMin + double((nMax - nMin) * factor);
                SetValue(newValue);

            }
            else
            {
                if (msg.ptMouse.x >= GetRect().right - szThumb.cx / 2) {
                    SetValue(nMax);
                }
                else if (msg.ptMouse.x <= GetRect().left + szThumb.cx / 2) {
                    SetValue(nMin);
                }
                else {
                    double newValue = nMin + double((nMax - nMin) * (msg.ptMouse.x - GetRect().left - szThumb.cx / 2)) / (GetRect().right - GetRect().left - szThumb.cx);
                    SetValue(newValue);
                }
            }
        }
        else {
            if (isLoop)
            {
                double factor = double(GetRect().bottom - msg.ptMouse.y - szThumb.cy / 2) / (GetRect().bottom - GetRect().top - szThumb.cy);
                factor = factor - floor(factor);
                double newValue = nMin + double((nMax - nMin) * factor);
                SetValue(newValue);
            }
            else
            {
                if (msg.ptMouse.y >= GetRect().bottom - szThumb.cy / 2) {
                    SetValue(nMin);
                }
                else if (msg.ptMouse.y <= GetRect().top + szThumb.cy / 2) {
                    SetValue(nMax);
                }
                else {

                    double newValue = nMin + double((nMax - nMin) * (GetRect().bottom - msg.ptMouse.y - szThumb.cy / 2)) / (GetRect().bottom - GetRect().top - szThumb.cy);
                    SetValue(newValue);
                }
            }
        }
        SendEvent(kEventValueChanged, (WPARAM)GetValue(), (LPARAM)oldValue);
        Invalidate();
        return;
    }
    if (msg.eventType == kEventMouseWheel) {
        double oldValue = GetValue();
        int32_t detaValue = msg.eventData;
        {
            double targetValue = (detaValue > 0) ? (GetValue() + GetChangeStep()) : (GetValue() - GetChangeStep());
        
        
            if (isLoop)
            {
                const int32_t nMin = GetMinValue();
                const int32_t nMax = GetMaxValue();
                if (nMax == nMin|| (targetValue>= nMin&&targetValue<= nMax))
                {
                    SetValue(targetValue);
                }
                else
                {
                    double factor = (targetValue - nMin) / (nMax - nMin);
                    factor = factor - floor(factor);
                    double newValue = nMin + double((nMax - nMin) * factor);
                    SetValue(newValue);
                }
            }
            else
            {
                SetValue(targetValue);
            }

            SendEvent(kEventValueChanged, (WPARAM)GetValue(), (LPARAM)oldValue);
            return;
        }
        //if (detaValue > 0) {
        //    SetValue(GetValue() + GetChangeStep());
        //    SendEvent(kEventValueChanged, (WPARAM)GetValue(), (LPARAM)oldValue);
        //    return;
        //}
        //else {
        //    SetValue(GetValue() - GetChangeStep());
        //    SendEvent(kEventValueChanged, (WPARAM)GetValue(), (LPARAM)oldValue);
        //    return;
        //}
    }
    if (msg.eventType == kEventMouseMove) {
        UiSize szThumb = GetThumbSize();
        const int32_t nMin = GetMinValue();
        const int32_t nMax = GetMaxValue();
        if (IsMouseFocused()) {
            double oldValue = GetValue();


            //此处有修改
            //此处有修改
            //此处有修改
            //此处有修改
            //此处有修改
            if (IsHorizontal()) {
                if (isLoop)
                {
                    //鼠标在控件范围内时，factor为0~1，这里factor可以表示鼠标在控件范围外的情况
                    double factor = double(msg.ptMouse.x - GetRect().left - szThumb.cx / 2) / (GetRect().right - GetRect().left - szThumb.cx);
                    //得到真实的0~1的factor
                    factor = factor - floor(factor);
                    double newValue = nMin + double((nMax - nMin) * factor);
                    SetValue(newValue);

                }
                else
                {
                    if (msg.ptMouse.x >= GetRect().right - szThumb.cx / 2) {
                        SetValue(nMax);
                    }
                    else if (msg.ptMouse.x <= GetRect().left + szThumb.cx / 2) {
                        SetValue(nMin);
                    }
                    else {
                        double newValue = nMin + double((nMax - nMin) * (msg.ptMouse.x - GetRect().left - szThumb.cx / 2)) / (GetRect().right - GetRect().left - szThumb.cx);
                        SetValue(newValue);
                    }
                }
            }
            else {
                if (isLoop)
                {
                    double factor = double(GetRect().bottom - msg.ptMouse.y - szThumb.cy / 2) / (GetRect().bottom - GetRect().top - szThumb.cy);
                    factor = factor - floor(factor);
                    double newValue = nMin + double((nMax - nMin) * factor);
                     SetValue(newValue);
                }
                else
				{
					if (msg.ptMouse.y >= GetRect().bottom - szThumb.cy / 2) {
						SetValue(nMin);
					}
					else if (msg.ptMouse.y <= GetRect().top + szThumb.cy / 2) {
						SetValue(nMax);
					}
					else {

						double newValue = nMin + double((nMax - nMin) * (GetRect().bottom - msg.ptMouse.y - szThumb.cy / 2)) / (GetRect().bottom - GetRect().top - szThumb.cy);
						SetValue(newValue);
					}
				}
			}
//此处有修改
//此处有修改
//此处有修改
//此处有修改
//此处有修改
            SendEvent(kEventValueChanged, (WPARAM)GetValue(), (LPARAM)oldValue);
            Invalidate();
        }
        return;
    }

    Progress::HandleEvent(msg);



}
