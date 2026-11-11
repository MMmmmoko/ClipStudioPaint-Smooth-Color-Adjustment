#pragma once

#include "json/json.h"
#include <cstdint>
#include <ostream>

namespace LevelsPreview
{
inline constexpr char PixelGate[] = "LevelsPreview_PixelGate";
inline constexpr char PixelEvent[] = "LevelsPreview_PixelEvent";
inline constexpr char LayerEvent[] = "LevelsPreview_LayerEvent";
inline constexpr char EventId[] = "LevelsPreview_EventId";
inline constexpr char LayerChangedToken[] = "LevelsPreview_LayerChangedToken";
inline constexpr char TimerToken[] = "LevelsPreview_TimerToken";

bool Locate(const uint8_t* image, uint32_t imageSize, uint32_t textRva, uint32_t textSize,
    uint32_t unwindRva, uint32_t unwindSize, uint32_t dataRva, uint32_t dataSize,
    Json::Value& table, std::ostream& log);
void Install();
bool Available();
void SetEnabled(bool enabled);
}
