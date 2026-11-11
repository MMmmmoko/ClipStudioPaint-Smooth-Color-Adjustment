
#include"NumEdit.h"

NumEdit::NumEdit(ui::Window* pWindow)
	:ui::RichEdit(pWindow)
{
	SetNumberOnly(true);
}

void NumEdit::SetAttribute(const DString& strName, const DString& strValue)
{
	//

	if (strName == _T("decimal")) {
		SetDecimalSupport( strValue == _T("true"));
	}
	else if ((strName == _T("min_prec"))) {
		SetMinPrec(ui::StringUtil::StringToInt32(strValue));
	}
	else if ((strName == _T("max_prec"))) {
		SetMaxPrec(ui::StringUtil::StringToInt32(strValue));
	}
	else if ((strName == _T("lose_focus_when_enter"))) {
        SetLoseFocusWhenEnter(strValue == _T("true"));
	}
	//拦截数字相关设置项
	else if ((strName == _T("number_only")) || (strName == _T("number"))) {
		
	}
	else
	{
		__super::SetAttribute(strName, strValue);
	}

}

float NumEdit::GetFloatValue() const
{
    DString text = GetText();
    if (text.empty()) {
        return 0;
    }
    return static_cast<float>(ui::StringUtil::StringToDouble(text));
}

double NumEdit::GetDoubleValue() const
{
    DString text = GetText();
    if (text.empty()) {
        return 0;
    }
    return ui::StringUtil::StringToDouble(text);
}

static std::string DecimalToSimpleString(double value, int max_prec, int min_prec)
{
    if (min_prec < 0) min_prec = 0;
    if (max_prec < min_prec) max_prec = min_prec;

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(max_prec) << value;
    std::string s = oss.str();

    auto dot = s.find('.');
    if (dot == std::string::npos) {
        return s;
    }

    std::size_t min_end = dot + 1 + static_cast<std::size_t>(min_prec);
    std::size_t last_non_zero = s.find_last_not_of('0');

    if (last_non_zero == std::string::npos || last_non_zero < min_end) {
        s.erase(min_end);
    }
    else {
        s.erase(last_non_zero + 1);
    }

    if (dot == s.size() - 1)
    {
        s.pop_back();
    }


    return s;
}




void NumEdit::SetTextDecimalValue(double value, bool triggerEvent)
{
    if (triggerEvent)
    {
        this->SetText(ui::StringConvert::UTF8ToWString(
            DecimalToSimpleString(value, maxPrec, minPrec)));
    }
    else {
        this->SetTextNoEvent(ui::StringConvert::UTF8ToWString(
            DecimalToSimpleString(value, maxPrec, minPrec)));
    }
}

void NumEdit::SetTextNumberNoEvent(int32_t value)
{
    this->SetTextNoEvent(std::to_wstring(value));
}

bool NumEdit::OnKeyDown(const ui::EventArgs& msg)
{
    if (msg.vkCode == ui::kVK_RETURN)
    {
        if (b_loseFocusWhenEnter)
        {
            this->GetParent()->SetFocus();
        }
	}
    return __super::OnKeyDown(msg);
}

bool NumEdit::OnKillFocus(const ui::EventArgs& msg)
{
    //失去焦点的时候，对文本内容进行修正
    SetTextDecimalValue(GetDoubleValue());
    return __super::OnKillFocus(msg);
}

bool NumEdit::OnChar(const ui::EventArgs& msg)
{
	//TAB
	if (::GetKeyState(VK_TAB) < 0 && !RichEdit::IsWantTab()) {
		SendEvent(ui::kEventTab);
		return true;
	}

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

				int32_t selStart = 0;
				int32_t selEnd = 0;
                this->GetSel(selStart, selEnd);
                if (selStart > 0) {
                    //不是第一个字符，禁止输入负号
                    return true;
                }


                std::wstring text = GetText();
                if (text.size() > selEnd && text[selEnd] == L'-')
                {
                    //选区的后方存在负号，则禁止输入负号

                    return true;
                }
            }
            else if (msg.vkCode == _T('.'))
            {
                if (b_supportDecimal)
                {
                    std::wstring text = GetText();
                    if (text.find(L'.') != text.npos)
                    {
						//已经存在小数点，禁止输入小数点
                        return true;
                    }

                    int32_t selStart = 0;
                    int32_t selEnd = 0;
                    this->GetSel(selStart, selEnd);
                    if (text.size() > selEnd && text[selEnd] == L'-')
                    {
                        //选区的后方存在负号，则禁止输入小数点
                        return true;
                    }
                }
                else
                {
                    //不支持小数，禁止输入小数点
                    return true;
                }
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
    if (msg.modifierKey & ui::ModifierKey::kIsSystemKey) {
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


	return false;
}


