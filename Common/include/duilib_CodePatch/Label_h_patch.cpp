#include"duilib/Control/Label.h"



//好像没用
//#ifndef LABEL_H_CODEPATCH
//static_assert(false,"Do Patch Code!");
//#endif // !LABEL_H_CODEPATCH


//修改EstimateText(UISize szAvailable)中的如下部分：
//DString textValue = GetText();
//if (!textValue.empty() && (this->GetWindow() != nullptr)) {
//    auto pRender = this->GetWindow()->GetRender();
//    if (pRender != nullptr) {
//        MeasureStringParam measureParam = GetMeasureParam();
//        measureParam.rectSize = !m_bVerticalText ? nWidth : nHeight;
//        UiRect rect = pRender->MeasureString(textValue, measureParam);
//        fixedSize.cx = rect.Width();
//        if (fixedSize.cx > 0) {
//            fixedSize.cx += (rcTextPadding.left + rcTextPadding.right);
//            fixedSize.cx += (rcPadding.left + rcPadding.right);
//        }
//
//        fixedSize.cy = rect.Height();
//        if (fixedSize.cy) {
//            fixedSize.cy += (rcTextPadding.top + rcTextPadding.bottom);
//            fixedSize.cy += (rcPadding.top + rcPadding.bottom);
//        }
//    }
//}



//修改后的版本:
#if 0


UiSize fixedSize;
DString textValue = GetText();
bool  textempty = textValue.empty();
if (textempty)textValue = L" ";
if (!textValue.empty() && (this->GetWindow() != nullptr)) {
    auto pRender = this->GetWindow()->GetRender();
    if (pRender != nullptr) {
        MeasureStringParam measureParam = GetMeasureParam();
        measureParam.rectSize = !m_bVerticalText ? nWidth : nHeight;
        UiRect rect = pRender->MeasureString(textValue, measureParam);
        fixedSize.cx = rect.Width();
        if (fixedSize.cx > 0) {
            fixedSize.cx += (rcTextPadding.left + rcTextPadding.right);
            fixedSize.cx += (rcPadding.left + rcPadding.right);
        }

        if (textempty)
            fixedSize.cx -= rect.Width();

        fixedSize.cy = rect.Height();
        if (fixedSize.cy) {
            fixedSize.cy += (rcTextPadding.top + rcTextPadding.bottom);
            fixedSize.cy += (rcPadding.top + rcPadding.bottom);
        }
    }
}

#endif // 0