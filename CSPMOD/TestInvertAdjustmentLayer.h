#pragma once

#include "CustomAdjustmentLayer.h"

#include <array>

namespace TestInvertAdjustment
{
inline constexpr uint32_t Type = CustomAdjustmentLayer::UserStart + 1;
using Strengths = std::array<uint8_t, 3>; // R, G, B in percent.
inline constexpr Strengths FullInvert{100, 100, 100};

constexpr uint8_t InvertChannel(uint8_t value, uint8_t strength)
{
    return static_cast<uint8_t>((value * (100 - strength) + (255 - value) * strength + 50) / 100);
}

const CustomAdjustmentLayer::AdjustmentLayer& Definition();
}
