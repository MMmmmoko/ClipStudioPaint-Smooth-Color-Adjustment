//需要对duilib一些地方进行修改，修改的位置是OnChar函数
//修改对负号的判定





#include "duilib/Control/RichEdit_Windows.h"
#include "duilib/Control/RichEditHost_Windows.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/WindowMessage.h"
#include "duilib/Core/ControlDropTarget.h"
#include "duilib/Core/ScrollBar.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/AttributeUtil.h"
#include "duilib/Utils/BitmapHelper_Windows.h"
#include "duilib/Utils/PerformanceUtil.h"
#include "duilib/Render/IRender.h"
#include "duilib/Render/AutoClip.h"
#include "duilib/Animation/AnimationManager.h"
#include "duilib/Animation/AnimationPlayer.h"
#include "duilib/Control/Menu.h"
#include "duilib/Box/VBox.h"
#include "duilib/Control/Button.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

#include "duilib/Core/ControlDropTargetImpl_Windows.h"
#include "duilib/Core/ControlDropTargetUtils.h"


namespace ui
{
    bool RichEdit::OnChar(const EventArgs& msg)
    {
        //TAB
        if (::GetKeyState(VK_TAB) < 0 && !m_bWantTab) {
            SendEvent(kEventTab);
            return true;
        }


        //这里是修改部分
        //这里是修改部分
        //这里是修改部分
        //这里是修改部分
        //这里是修改部分
        //Number
        if (IsNumberOnly()) {
            if (msg.vkCode < '0' || msg.vkCode > '9') {
                if (msg.vkCode == _T('-')) {

                    if (GetMinNumber() >= 0) {
                        //最小数字是0或者正数，禁止输入符号
                        return true;
                    }


                    //if (GetTextLength() > 0) {
                    //    //不是第一个字符，禁止输入负号
                    //    return true;
                    //}

                    //this->getc
                    CHARRANGE charrange;
                    GetSel(charrange);
                    if (charrange.cpMin > 0) {
                        //不是第一个字符，禁止输入负号
                        return true;
                    }


                    std::wstring text=GetText();
                    if (text.size() > charrange.cpMax&& text[charrange.cpMax]=='-')
                    {
                        //选区的后方存在负号，则禁止输入负号

                        return true;
                    }
                }
                else if (msg.vkCode == _T('.'))
                {
                    return true;
                }
                else
                {
                    return true;
                }
            }
        }




        //限制允许输入的字符
        if (m_pLimitChars != nullptr) {
            if (!IsInLimitChars((DStringW::value_type)msg.vkCode)) {
                //字符不在列表里面，禁止输入
                return true;
            }
        }
#ifdef DUILIB_UNICODE
        WPARAM wParam = msg.wParam;
        WPARAM lParam = msg.lParam;
        if (msg.modifierKey & ModifierKey::kIsSystemKey) {
            m_richCtrl.TxSendMessage(WM_SYSCHAR, wParam, lParam);
        }
        else {
            m_richCtrl.TxSendMessage(WM_CHAR, wParam, lParam);
        }
#else
        //只支持1字节和2字节的文字输入，不支持4字节的文字输入
        if ((::GetTickCount() - m_dwLastCharTime) > 5000) {
            m_pendingChars.clear();
        }
        bool bHandled = false;
        if (m_pendingChars.empty()) {
            if (IsDBCSLeadByte((BYTE)msg.wParam)) {
                m_pendingChars.push_back((BYTE)msg.wParam);
                bHandled = true;
            }
        }
        else {
            if (m_pendingChars.size() == 1) {
                BYTE chMBCS[8] = { m_pendingChars.front(), (BYTE)msg.wParam, 0, };
                DStringW::value_type chWideChar[4] = { 0, };
                ::MultiByteToWideChar(CP_ACP, 0, (const char*)chMBCS, 2, chWideChar, 4);
                if (chWideChar[0] != 0) {
                    WPARAM wParam = chWideChar[0];
                    m_richCtrl.TxSendMessage(WM_CHAR, wParam, msg.lParam);
                    bHandled = true;
                }
                m_pendingChars.clear();
            }
            else {
                ASSERT(false);
                m_pendingChars.clear();
            }
        }
        m_dwLastCharTime = ::GetTickCount();
        if (!bHandled) {
            m_richCtrl.TxSendMessage(WM_CHAR, msg.wParam, msg.lParam);
        }
#endif    
        return true;
    }

}


#endif    