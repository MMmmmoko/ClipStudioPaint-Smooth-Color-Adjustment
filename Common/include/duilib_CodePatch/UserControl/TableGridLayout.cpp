
#include "TableGridLayout.h"

#include "duilib/Box/ScrollBox.h"
#include "duilib/Core/DpiManager.h"
using namespace ui;
TableGridLayout::TableGridLayout() :
    //m_nRows(0),
    m_nColumns(0),
    //m_nGridWidth(0),
    m_nGridHeight(0),
    m_bScaleDown(false)
{
    //默认水平居中
    SetChildHAlignType(HorAlignType::kAlignCenter);

    //默认垂直居中
    SetChildVAlignType(VerAlignType::kAlignCenter);
}

void TableGridLayout::SetColumns(int32_t nCols)
{
    ASSERT(nCols >= 0);
    nCols = std::max(nCols, 0);
    if (m_nColumns != nCols) {
        m_nColumns = nCols;
        if (GetOwner() != nullptr) {
            GetOwner()->Arrange();
        }
    }
}

void TableGridLayout::SetGridHeight(int32_t nGridHeight, bool bNeedDpiScale)
{
    ASSERT(nGridHeight >= 0);
    if (nGridHeight >= 0) {
        if (bNeedDpiScale) {
            ASSERT(GetOwner() != nullptr);
            if (GetOwner() != nullptr) {
                GetOwner()->Dpi().ScaleInt(nGridHeight);
            }
        }
        if (m_nGridHeight != nGridHeight) {
            m_nGridHeight = nGridHeight;
            if (GetOwner() != nullptr) {
                GetOwner()->Arrange();
            }
        }
    }
}
void TableGridLayout::SetScaleDown(bool bScaleDown)
{
    if (m_bScaleDown != bScaleDown) {
        m_bScaleDown = bScaleDown;
        if (GetOwner() != nullptr) {
            GetOwner()->Arrange();
        }
    }
}
void TableGridLayout::SetLayout(const std::wstring& layout, std::vector<ui::UiFixedInt>& target)
{
    std::vector<std::wstring> tokens;
    size_t start = 0;
    size_t end = layout.find(L',');

    while (end != std::wstring::npos) {
        tokens.push_back(layout.substr(start, end - start));
        start = end + 1;
        end = layout.find(L',', start);
    }
    tokens.push_back(layout.substr(start));


	//然后tokens转为fixedInt
    target.clear();

    for (auto& token : tokens)
    {
        if (token == _T("stretch")) {
            //宽度为拉伸：由父容器负责分配宽度
            target.push_back(UiFixedInt::MakeStretch());
        }
        else if (token == _T("auto")) {
            //宽度为自动：根据控件的文本、图片等自动计算宽度
            target.push_back(UiFixedInt::MakeAuto());
        }
        else if (!token.empty()) {
            if (token.back() == _T('%')) {
                //宽度为拉伸：由父容器负责按百分比分配宽度，比如 width="30%"，代表该控件的宽度期望值为父控件宽度的30%
                int32_t iValue = StringUtil::StringToInt32(token);
                if ((iValue <= 0) || (iValue > 100)) {
                    iValue = 100;
                }
                target.push_back(UiFixedInt::MakeStretch(iValue));
            }
            else {
                //宽度为固定值
                ASSERT(StringUtil::StringToInt32(token) >= 0);
                target.push_back(UiFixedInt(StringUtil::StringToInt32(token)));
            }
        }
        else {
            target.push_back(UiFixedInt(0));
        }
    }

}

bool TableGridLayout::SetAttribute(const DString& strName, const DString& strValue, const ui::DpiManager& dpiManager)
{
    bool hasAttribute = true;
    if (strName == _T("rows")) {
        //if (strValue == _T("auto")) {
        //    //自动计算
        //    SetRows(0);
        //}
        //else {
        //    SetRows(StringUtil::StringToInt32(strValue));
        //}
    }
    else if (strName == _T("columns")) {
        if (strValue == _T("auto")) {
            //自动计算
            SetColumns(0);
        }
        else {
            SetColumns(StringUtil::StringToInt32(strValue));
        }
    }
    else if (strName == _T("grid_width")) {
        //if (strValue == _T("auto")) {
        //    //自动计算
        //    SetGridWidth(0, false);
        //}
        //else {
        //    int32_t nGridWidth = StringUtil::StringToInt32(strValue);
        //    dpiManager.ScaleInt(nGridWidth);
        //    SetGridWidth(nGridWidth, false);
        //}
    }
    else if (strName == _T("grid_height")) {
        if (strValue == _T("auto")) {
            //自动计算
            SetGridHeight(0, false);
        }
        else {
            int32_t nGridHeight = StringUtil::StringToInt32(strValue);
            dpiManager.ScaleInt(nGridHeight);
            SetGridHeight(nGridHeight, false);
        }
    }
    else if (strName == _T("scale_down")) {
        SetScaleDown(strValue == _T("true"));
    }
    else if (strName == _T("columns_layout")) {
        SetLayout(strValue,columnsLayout);
    }
    else if (strName == _T("rows_layout")) {
        SetLayout(strValue, rowsLayout);
    }
    else {
        hasAttribute = __super::SetAttribute(strName, strValue, dpiManager);
    }
    return hasAttribute;
}
void TableGridLayout::ChangeDpiScale(const DpiManager& dpiManager, uint32_t nOldDpiScale)
{
    //int32_t nGridWidth = GetGridWidth();
    //nGridWidth = dpiManager.GetScaleInt(nGridWidth, nOldDpiScale);
    //SetGridWidth(nGridWidth, false);

    int32_t nGridHeight = GetGridHeight();
    nGridHeight = dpiManager.GetScaleInt(nGridHeight, nOldDpiScale);
    SetGridHeight(nGridHeight, false);
}

ui::UiSize64 TableGridLayout::ArrangeChildren(const std::vector<ui::Control*>& items, ui::UiRect rc, bool bEstimateOnly)
{
    if (items.empty()) {
        return UiSize64();
    }


    // 预处理：筛选所有可见、非浮动的控件
    std::vector<Control*> visibleControls;
    for (auto pControl : items) {
        if ((pControl != nullptr) && pControl->IsVisible() && !pControl->IsFloat()) {
            visibleControls.push_back(pControl);
        }
    }
    if (visibleControls.empty()) {
        //可见控件为空, 调整子控件的布局（浮动控件），然后直接返回
        if (!bEstimateOnly) {
            for (auto pControl : items) {
                if ((pControl != nullptr) && pControl->IsVisible() && pControl->IsFloat()) {
                    //浮动控件（容器本身的对齐方式不生效）
                    SetFloatPos(pControl, rc);
                }
            }
        }
        return UiSize64();
    }


    DeflatePadding(rc);
    const UiRect contentRect = rc; //容器的矩形范围, 已扣除容器内边距



    // 计算实际行列数（处理自动计算）
    //int32_t actualRows = GetRows();
    int32_t actualRows = 0;
    int32_t actualCols = GetColumns();


    //计算格子数量
    int32_t gridCount = 0;
    for (auto pControl : visibleControls) {
        int32_t rowSpan = pControl->GetRowSpan();
        int32_t colSpan = pControl->GetColumnSpan();
        ASSERT(rowSpan > 0);
        ASSERT(colSpan > 0);
        gridCount += rowSpan * colSpan;
    }

    //修改actualRows、actualCols为实际上的行数列数
    CalcActualGridSize(gridCount, actualRows, actualCols);

    // 计算单元格基础尺寸（未考虑合并）
    std::vector<int32_t> colWidths = CalcColumnWidths(contentRect, actualCols, items);
    std::vector<int32_t> rowHeights = CalcRowHeights(contentRect, actualRows, items);


    // 标记已占用的单元格（处理合并）
    std::vector<std::vector<bool>> cellOccupied(actualRows, std::vector<bool>(actualCols, false));
    std::unordered_map<Control*, UiRect> ctrlCellRects; // 控件对应的合并后单元格区域
    if (!CalcMergedCellRects(visibleControls, actualRows, actualCols, colWidths, rowHeights, cellOccupied, ctrlCellRects)) {
        ASSERT(!"CalcMergedCellRects failed!");
        return UiSize64(); // 单元格合并冲突，返回无效尺寸
    }






    //计算网格整体位置（基于布局的对齐属性）
    UiSize gridTotalSize = CalcGridTotalSize(colWidths, rowHeights);

    if (!bEstimateOnly) {
        UiRect gridRect = CalcGridPosition(contentRect, gridTotalSize);

        //调整合并单元格位置（基于网格整体偏移）
        AdjustMergedCellPositions(ctrlCellRects, gridRect);

        //排列控件到合并单元格（使用控件自身对齐属性）
        ArrangeControlsInMergedCells(visibleControls, ctrlCellRects);

        // 处理浮动控件
        for (auto pControl : items) {
            if ((pControl != nullptr) && pControl->IsVisible() && pControl->IsFloat()) {
                SetFloatPos(pControl, rc);
            }
        }
    }


    //计算最终所需尺寸（含内边距）
    UiPadding rcPadding = GetOwner() != nullptr ? GetOwner()->GetPadding() : UiPadding();
    return UiSize64(
        gridTotalSize.cx + rcPadding.left + rcPadding.right,
        gridTotalSize.cy + rcPadding.top + rcPadding.bottom
    );
}

ui::UiSize64 TableGridLayout::EstimateLayoutSize(const std::vector<ui::Control*>& items, ui::UiSize szAvailable)
{
    UiRect rc(0, 0, szAvailable.cx, szAvailable.cy);
    return ArrangeChildren(items, rc, true);
}


















void TableGridLayout::CalcActualGridSize(int32_t gridCount, int32_t& rows, int32_t& cols) const
{
    rows = std::max(rows, 0);
    cols = std::max(cols, 0);
    if (gridCount > 0) {
        if (rows == 0 && cols == 0) {
            cols = static_cast<int32_t>(std::sqrt(gridCount)) + 1;
            rows = (gridCount + cols - 1) / cols;
        }
        else if (rows == 0) {
            rows = (gridCount + cols - 1) / cols;
        }
        else if (cols == 0) {
            cols = (gridCount + rows - 1) / rows;
        }
    }
    rows = std::max(rows, 1);
    cols = std::max(cols, 1);
}

std::vector<int32_t> TableGridLayout::CalcColumnWidths(const UiRect& contentRect, int32_t cols,const std::vector<Control*>& items) const
{
    ASSERT(cols > 0);
    if (cols <= 0) {
        return std::vector<int32_t>();
    }

    const UiSize szAvailable(contentRect.Width(), contentRect.Height());


    //if (columnsLayout.empty())
    //{
    //    //计算子控件宽度
    //    std::vector<int32_t> widths(cols, 0);
    //    for (int curCol = 0; curCol < cols; curCol++)
    //    {
    //        int32_t maxWidth = 0;
    //        for (int rowItem = curCol; rowItem < items.size(); rowItem += cols)
    //        {
    //            Control* curControl = items[rowItem];
    //            UiMargin rcMargin = curControl->GetMargin();


    //            UiEstSize estSize = curControl->EstimateSize(szAvailable);
    //            UiSize sz = UiSize(estSize.cx.GetInt32(), estSize.cy.GetInt32());

    //            int32_t actualWidth = rcMargin.left + rcMargin.right + sz.Width();

    //            if (actualWidth > maxWidth)
    //            {
    //                maxWidth = actualWidth;
    //            }
    //        }
    //        widths[curCol] = maxWidth;

    //    }
    //}
    //else
    {
        //手动指定了宽度

        std::vector<int32_t> widths(cols, 0);
        int32_t allocatedWidth = 0;
		int32_t stretchCount = 0;
        for (int curCol = 0; curCol < cols; curCol++)
        {
			ui::UiFixedInt curFixedInt;
            if (curCol >= columnsLayout.size())
            {
				//如果指定的列数大于columnsLayout的大小，则使用Auto
				curFixedInt = ui::UiFixedInt::MakeAuto();
            }
            else
            {
                curFixedInt = columnsLayout[curCol];
            }

            //如果直接指定了宽度，那么直接使用
            if (curFixedInt.IsInt32())
            {
				widths[curCol] = curFixedInt.GetInt32();
				allocatedWidth += widths[curCol];
                continue;
            }

			//如果是Auto，则计算当前列的最大宽度

            if (curFixedInt.IsAuto())
            {
                //for (int curCol = 0; curCol < cols; curCol++)
                {
                    int32_t maxWidth = 0;
                    for (int rowItem = curCol; rowItem < items.size(); rowItem += cols)
                    {
                        Control* curControl = items[rowItem];
                        UiMargin rcMargin = curControl->GetMargin();


                        UiEstSize estSize = curControl->EstimateSize(szAvailable);
                        UiSize sz = UiSize(estSize.cx.GetInt32(), estSize.cy.GetInt32());

                        int32_t actualWidth = rcMargin.left + rcMargin.right + sz.Width();

                        if (actualWidth > maxWidth)
                        {
                            maxWidth = actualWidth;
                        }
                    }
                    widths[curCol] = maxWidth;
                }
                allocatedWidth += widths[curCol];
                continue;
            }

            //拉伸类型先进行统计
            if (curFixedInt.IsStretch())
            {
                stretchCount += curFixedInt.GetStretchPercentValue();
                continue;
            }
        }

		//计算剩余宽度并按比例分配给Stretch类型的列
		int32_t remainingWidth = std::max(0, contentRect.Width() - allocatedWidth);

		//计算是否有残余像素，如果有，则将其分配给最后一个Stretch类型的列
        int32_t finalRemainingWidth= remainingWidth;
		int lastStretchCol = -1;
        for (int curCol = 0; curCol < cols; curCol++)
        {
            ui::UiFixedInt curFixedInt;
            if (curCol >= columnsLayout.size())
                //如果指定的列数大于columnsLayout的大小，则使用Auto
                curFixedInt = ui::UiFixedInt::MakeAuto();
            else
                curFixedInt = columnsLayout[curCol];

            if (curFixedInt.IsStretch())
            {
                lastStretchCol = curCol;
                int32_t stretchPercent = curFixedInt.GetStretchPercentValue();
				widths[curCol] = (remainingWidth * stretchPercent) / stretchCount;
				finalRemainingWidth -= widths[curCol];
            }
        }
        if (finalRemainingWidth > 0&& lastStretchCol!=-1)
        {
			widths[lastStretchCol] += finalRemainingWidth;
        }


        return widths;
    }





    //SDL_Log("width test:contentRect:%d,%d", contentRect.Width(), contentRect.Height());
    //int i = 0;
    //for (auto& w : widths)
    //{
    //    i++;
    //    SDL_Log("%d:%d",i,w);
    //}

    //return widths;
}


std::vector<int32_t> TableGridLayout::CalcRowHeights(const UiRect& contentRect, int32_t rows, const std::vector<Control*>& items) const
{
    ASSERT(rows > 0);
    if (rows <= 0) {
        return std::vector<int32_t>();
    }

    int32_t nGridHeight = GetGridHeight();
    if (nGridHeight > 0) {
        // 已经设置了网格的固定宽度，直接使用
        std::vector<int32_t> heights(rows, nGridHeight);
        return heights;
    }
    else {
        //手动指定了高度度

        const UiSize szAvailable(contentRect.Width(), contentRect.Height());



        std::vector<int32_t> heights(rows, 0);
        int32_t allocatedHeight = 0;
        int32_t stretchCount = 0;
        for (int curRow = 0; curRow < rows; curRow++)
        {
            ui::UiFixedInt curFixedInt;
            if (curRow >= rowsLayout.size())
            {
                //如果指定的列数大于rowsLayout的大小，则使用Auto
                curFixedInt = ui::UiFixedInt::MakeAuto();
            }
            else
            {
                curFixedInt = rowsLayout[curRow];
            }

            //如果直接指定了高度度，那么直接使用
            if (curFixedInt.IsInt32())
            {
                heights[curRow] = curFixedInt.GetInt32();
                allocatedHeight += heights[curRow];
                continue;
            }

            //如果是Auto，则计算当前行的最大高度

            if (curFixedInt.IsAuto())
            {
                {
                    int32_t maxHeight = 0;
                    for (int columnItem = curRow* m_nColumns; columnItem < items.size(); columnItem ++)
                    {
                        Control* curControl = items[columnItem];
                        UiMargin rcMargin = curControl->GetMargin();


                        UiEstSize estSize = curControl->EstimateSize(szAvailable);
                        UiSize sz = UiSize(estSize.cx.GetInt32(), estSize.cy.GetInt32());

                        int32_t actualHeight = rcMargin.top + rcMargin.bottom + sz.Height();

                        if (actualHeight > maxHeight)
                        {
                            maxHeight = actualHeight;
                        }
                    }
                    heights[curRow] = maxHeight;
                }
                allocatedHeight += heights[curRow];
                continue;
            }

            //拉伸类型先进行统计
            if (curFixedInt.IsStretch())
            {
                stretchCount += curFixedInt.GetStretchPercentValue();
                continue;
            }
        }

        //计算剩余宽度并按比例分配给Stretch类型的列
        int32_t remainingHeight = std::max(0, contentRect.Height() - allocatedHeight);

        //计算是否有残余像素，如果有，则将其分配给最后一个Stretch类型的列
        int32_t finalRemainingHeight = remainingHeight;
        int lastStretchRow = -1;
        for (int curRow = 0; curRow < rows; curRow++)
        {
            ui::UiFixedInt curFixedInt;
            if (curRow >= rowsLayout.size())
                //如果指定的列数大于rowsLayout的大小，则使用Auto
                curFixedInt = ui::UiFixedInt::MakeAuto();
            else
                curFixedInt = rowsLayout[curRow];

            if (curFixedInt.IsStretch())
            {
                lastStretchRow = curRow;
                int32_t stretchPercent = curFixedInt.GetStretchPercentValue();
                heights[curRow] = (remainingHeight * stretchPercent) / stretchCount;
                finalRemainingHeight -= heights[curRow];
            }
        }
        if (finalRemainingHeight > 0 && lastStretchRow != -1)
        {
            heights[lastStretchRow] += finalRemainingHeight;
        }


        return heights;
    }
}






UiSize TableGridLayout::CalcGridTotalSize(const std::vector<int32_t>& colWidths, const std::vector<int32_t>& rowHeights) const
{
    int32_t totalWidth = 0;
    for (size_t i = 0; i < colWidths.size(); ++i) {
        totalWidth += colWidths[i] + (i == (colWidths.size() - 1) ? 0 : GetChildMarginX());
    }
    int32_t totalHeight = 0;
    for (size_t i = 0; i < rowHeights.size(); ++i) {
        totalHeight += rowHeights[i] + (i == (rowHeights.size() - 1) ? 0 : GetChildMarginY());
    }
    return UiSize(totalWidth, totalHeight);
}



UiRect TableGridLayout::CalcGridPosition(const UiRect& contentRect, const UiSize& gridSize) const
{
    UiRect gridRect;
    gridRect.left = contentRect.left;
    gridRect.top = contentRect.top;
    gridRect.right = gridRect.left + gridSize.cx;
    gridRect.bottom = gridRect.top + gridSize.cy;

    // 水平对齐（布局自身的GetChildHAlignType）
    switch (GetChildHAlignType()) {
    case HorAlignType::kAlignCenter:
        gridRect.Offset((contentRect.Width() - gridSize.cx) / 2, 0);
        break;
    case HorAlignType::kAlignRight:
        gridRect.Offset(contentRect.Width() - gridSize.cx, 0);
        break;
    default:
        break;
    }

    // 垂直对齐（布局自身的GetChildVAlignType）
    switch (GetChildVAlignType()) {
    case VerAlignType::kAlignCenter:
        gridRect.Offset(0, (contentRect.Height() - gridSize.cy) / 2);
        break;
    case VerAlignType::kAlignBottom:
        gridRect.Offset(0, contentRect.Height() - gridSize.cy);
        break;
    default:
        break;
    }
    return gridRect;
}


bool TableGridLayout::CalcMergedCellRects(const std::vector<Control*>& controls,
    int32_t rows, int32_t cols,
    const std::vector<int32_t>& colWidths,
    const std::vector<int32_t>& rowHeights,
    std::vector<std::vector<bool>>& cellOccupied,
    std::unordered_map<Control*, UiRect>& ctrlCellRects) const
{
    ASSERT((rows > 0) && (cols > 0) && !controls.empty() && ((int32_t)colWidths.size() == cols) && ((int32_t)rowHeights.size() == rows));
    if ((rows <= 0) || (cols <= 0) || controls.empty() ||
        ((int32_t)colWidths.size() != cols) ||
        ((int32_t)rowHeights.size() != rows)) {
        return false;
    }
    int32_t ctrlIndex = 0;
    for (int32_t row = 0; row < rows && ctrlIndex < (int32_t)controls.size(); ++row) {
        for (int32_t col = 0; col < cols && ctrlIndex < (int32_t)controls.size(); ++col) {
            if (cellOccupied[row][col]) {
                continue; // 跳过已占用单元格
            }

            Control* ctrl = controls[ctrlIndex++];
            int32_t rowSpan = std::min(ctrl->GetRowSpan(), rows - row); // 限制最大行合并
            int32_t colSpan = std::min(ctrl->GetColumnSpan(), cols - col); // 限制最大列合并

            // 标记合并范围内的单元格为已占用
            for (int32_t r = row; r < row + rowSpan; ++r) {
                for (int32_t c = col; c < col + colSpan; ++c) {
                    if (cellOccupied[r][c]) {
                        return false; // 合并冲突（重叠），返回失败
                    }
                    cellOccupied[r][c] = true;
                }
            }

            // 计算合并后单元格的原始尺寸（未考虑网格偏移）
            UiRect cellRect;
            cellRect.left = 0;
            for (int32_t c = 0; c < col; ++c) {
                cellRect.left += colWidths[c] + GetChildMarginX();
            }
            cellRect.right = cellRect.left;
            for (int32_t c = col; c < col + colSpan; ++c) {
                cellRect.right += colWidths[c] + (c == (col + colSpan - 1) ? 0 : GetChildMarginX());
            }

            cellRect.top = 0;
            for (int32_t r = 0; r < row; ++r) {
                cellRect.top += rowHeights[r] + GetChildMarginY();
            }
            cellRect.bottom = cellRect.top;
            for (int32_t r = row; r < row + rowSpan; ++r) {
                cellRect.bottom += rowHeights[r] + (r == (row + rowSpan - 1) ? 0 : GetChildMarginY());
            }

            ctrlCellRects[ctrl] = cellRect;
        }
    }
    return true;
}



void TableGridLayout::AdjustMergedCellPositions(std::unordered_map<Control*, UiRect>& ctrlCellRects,
    const UiRect& gridRect) const
{
    for (auto& [ctrl, rect] : ctrlCellRects) {
        rect.Offset(gridRect.left, gridRect.top);
    }
}

void TableGridLayout::ArrangeControlsInMergedCells(const std::vector<Control*>& controls,
    const std::unordered_map<Control*, UiRect>& ctrlCellRects) const
{
    for (Control* pControl : controls) {
        auto it = ctrlCellRects.find(pControl);
        if (it == ctrlCellRects.end()) {
            continue;
        }

        const UiRect& cellRect = it->second;
        UiMargin margin = pControl->GetMargin();

        // 控件在单元格内的位置
        UiRect rcChild;

        // 单元格内可用空间（扣除控件外边距）
        UiRect contentRect = cellRect;
        contentRect.left += margin.left;
        contentRect.top += margin.top;
        contentRect.right -= margin.right;
        contentRect.bottom -= margin.bottom;
        if (contentRect.Width() <= 0 || contentRect.Height() <= 0) {
            // 单元格内空间不足：设置其宽度和高度为0
            rcChild = contentRect;
            rcChild.right = rcChild.left;
            rcChild.bottom = rcChild.top;
        }
        else {
            // 单元格内空间充足: 计算控件尺寸（受限于自身min/max）
            UiSize childSize;
            if (IsScaleDown()) {
                UiSize szAvailable(contentRect.Width(), contentRect.Height());
                UiEstSize estSize = pControl->EstimateSize(szAvailable);
                childSize = UiSize(estSize.cx.GetInt32(), estSize.cy.GetInt32());
                if (estSize.cx.IsStretch()) {
                    childSize.cx = CalcStretchValue(estSize.cx, szAvailable.cx);
                }
                if (estSize.cy.IsStretch()) {
                    childSize.cy = CalcStretchValue(estSize.cy, szAvailable.cy);
                }
                childSize.cx = std::clamp(childSize.cx, pControl->GetMinWidth(), pControl->GetMaxWidth());
                childSize.cy = std::clamp(childSize.cy, pControl->GetMinHeight(), pControl->GetMaxHeight());

                // 如果子控件大小超出格子范围，则等比例缩小到格子以内，避免超出边界
                if ((childSize.cx > 0) && (childSize.cy > 0)) {
                    if ((childSize.cx > contentRect.Width()) || (childSize.cy > contentRect.Height())) {
                        //满足缩放条件，进行等比缩放
                        double cx = childSize.cx;
                        double cy = childSize.cy;
                        double cxRatio = cx / contentRect.Width();
                        double cyRatio = cy / contentRect.Height();
                        if (cxRatio > cyRatio) {
                            ASSERT(childSize.cx > contentRect.Width());
                            double ratio = cx / childSize.cy;
                            childSize.cx = contentRect.Width();
                            childSize.cy = static_cast<int32_t>(childSize.cx / ratio + 0.5);
                        }
                        else {
                            ASSERT(childSize.cy > contentRect.Height());
                            double ratio = cy / childSize.cx;
                            childSize.cy = contentRect.Height();
                            childSize.cx = static_cast<int32_t>(childSize.cy / ratio + 0.5);
                        }
                    }
                }
            }
            else {
                childSize = UiSize(contentRect.Width(), contentRect.Height());
            }

            // 计算控件在单元格内的位置（使用控件自身的对齐属性）
            rcChild = CalcControlPosition(contentRect, childSize, pControl);
        }
        pControl->SetPos(rcChild);
    }
}



UiRect TableGridLayout::CalcControlPosition(const UiRect& contentRect, const UiSize& ctrlSize, Control* ctrl) const
{
    UiRect ctrlRect;
    ctrlRect.left = contentRect.left;
    ctrlRect.top = contentRect.top;
    ctrlRect.right = ctrlRect.left + ctrlSize.cx;
    ctrlRect.bottom = ctrlRect.top + ctrlSize.cy;

    // 水平对齐（控件自身的GetHorAlignType）
    switch (ctrl->GetHorAlignType()) {
    case HorAlignType::kAlignCenter:
        ctrlRect.Offset((contentRect.Width() - ctrlSize.cx) / 2, 0);
        break;
    case HorAlignType::kAlignRight:
        ctrlRect.Offset(contentRect.Width() - ctrlSize.cx, 0);
        break;
    default:
        break;
    }

    // 垂直对齐（控件自身的GetVerAlignType）
    switch (ctrl->GetVerAlignType()) {
    case VerAlignType::kAlignCenter:
        ctrlRect.Offset(0, (contentRect.Height() - ctrlSize.cy) / 2);
        break;
    case VerAlignType::kAlignBottom:
        ctrlRect.Offset(0, contentRect.Height() - ctrlSize.cy);
        break;
    default:
        break;
    }
    return ctrlRect;
}