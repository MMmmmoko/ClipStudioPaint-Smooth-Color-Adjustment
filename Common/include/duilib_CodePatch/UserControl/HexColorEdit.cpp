
#include"HexColorEdit.h"

HexColorEdit::HexColorEdit(ui::Window* pWindow)
	:ui::RichEdit(pWindow)
{
    SetLimitChars(L"0123456789ABCDEFabcdef");
    SetUseAlpha(true);
}

void HexColorEdit::SetAttribute(const DString& strName, const DString& strValue)
{
    if ((strName == _T("lose_focus_when_enter"))) {
        SetLoseFocusWhenEnter(strValue == _T("true"));
	}
    if ((strName == _T("use_alpha"))) {
        SetUseAlpha(strValue == _T("true"));
	}
	else
	{
		__super::SetAttribute(strName, strValue);
	}

}


void HexColorEdit::SetUseAlpha(bool b)
{
    b_useAlpha = b;
    //SetLimitText(b_useAlpha?8:6);
}
static int hex_val(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}
static bool ColorFromHexStr(const char* s, uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a)
{
    if (!s || !r || !g || !b || !a) return false;

    if (s[0] == '#') s++;

    size_t len = strlen(s);
    if (len != 6 && len != 8) return false;

    /* 逐字符校验并累加，避免依赖 strtoul 的宽松容错 */
    uint32_t v = 0;
    for (size_t i = 0; i < len; i++) {
        int d = hex_val(s[i]);
        if (d < 0) return false;
        v = (v << 4) | (uint32_t)d;
    }

    uint8_t ur, ug, ub, ua;

    if (len == 8) {
        ua = ((v >> 24) & 0xFF);
        ur = ((v >> 16) & 0xFF);
        ug = ((v >> 8) & 0xFF);
        ub = (v & 0xFF);
    }
    else {
        ua = 255;
        ur = ((v >> 16) & 0xFF);
        ug = ((v >> 8) & 0xFF);
        ub = (v & 0xFF);
    }

    /* 全部算完再写回，保证失败安全（原子性） */
    *r = ur;
    *g = ug;
    *b = ub;
    *a = ua;
    return true;
}
bool HexColorEdit::GetColorValue(ui::UiColor& valueOut)
{
    std::wstring text = TextFix( GetText());
    //if (text.size() == 6)
    {
        uint8_t r, g, b, a;
        bool result=ColorFromHexStr(ui::StringConvert::WStringToUTF8(text).c_str(), &r,&g,&b,&a);
        if (result)
        {
            valueOut = ui::UiColor(a,r, g, b);
            return true;
        }
    }
    return false;
}

bool HexColorEdit::OnKeyDown(const ui::EventArgs& msg)
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

bool HexColorEdit::OnKillFocus(const ui::EventArgs& msg)
{
    //补齐字符串
    std::wstring text = GetText();
    SetText(TextFix(text));

    return __super::OnKillFocus(msg);
}

std::wstring HexColorEdit::TextFix(const std::wstring& text)
{
    std::wstring fixedText;
    switch (text.size())
    {
    case 0:
    {
        fixedText = (b_useAlpha ? L"00000000" : L"000000");
        break;
    }
    case 1:
    {
        fixedText = (b_useAlpha ? L"FF" : L"")+text + text + text + text + text + text;
        break;
    }
    case 2:
    {
        fixedText = (b_useAlpha ? L"FF" : L"") + text + text + text ;
        break;
    }
    case 3:
    {
        wchar_t t[7];
        t[0] = text[0];
        t[1] = text[0];
        t[2] = text[1];
        t[3] = text[1];
        t[4] = text[2];
        t[5] = text[2];
        t[6] = L'\0';
        fixedText = t;
        fixedText=((b_useAlpha ? L"FF" : L"") + fixedText);
        break;
    }
    case 4:
    {
        if (b_useAlpha)
        {

            wchar_t t[9];
            t[0] = text[0];
            t[1] = text[0];
            t[2] = text[1];
            t[3] = text[1];
            t[4] = text[2];
            t[5] = text[2];
            t[6] = text[3];
            t[7] = text[3];
            t[8] = L'\0';
            fixedText = t;
            fixedText=((b_useAlpha ? L"FF" : L"") + fixedText);
        }
        else
        {
            fixedText = text + L"00";

        }
        break;
    }
    case 5:
    {
        fixedText = text;
        fixedText.push_back(text[4]);
        fixedText=((b_useAlpha ? L"FF" : L"") + fixedText);
        break;
    }

    case 6:
    {
        if (b_useAlpha)
            fixedText = ((b_useAlpha ? L"FF" : L"") + text);
        else
            fixedText = text;
        break;
    }

    case 7:
    {
        if (b_useAlpha)
        {
            fixedText = text;
            fixedText.push_back(text[6]);
        }
        else
        {
            fixedText = text;
            fixedText.pop_back();
        }
        break;
    }

    case 8:
    {
        if (b_useAlpha)
        {
            fixedText = text;
        }
        else
        {
            fixedText = text.substr(2,6);
        }
        break;
    }
    default:
        //fixedText= (b_useAlpha ? L"00000000" : L"000000");
        return TextFix(text.substr(0,8));
        break;
    }
    return fixedText;
}


