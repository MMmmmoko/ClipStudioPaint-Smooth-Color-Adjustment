#pragma once

#include "TestInvertAdjustmentLayer.h"

#include <functional>

namespace TestInvertAdjustmentDialog
{
bool Show(HWND parent, TestInvertAdjustment::Strengths& strengths,
    const std::function<void(TestInvertAdjustment::Strengths)>& preview, bool allowCanvasNavigation);
}
