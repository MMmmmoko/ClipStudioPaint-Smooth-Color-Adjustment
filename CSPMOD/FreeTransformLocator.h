#pragma once

#include "json/json.h"
#include <cstdint>
#include <ostream>

namespace FreeTransform
{
inline constexpr uint32_t earlierManipulatorModeOffset = 0x194;
inline constexpr const char* Begin = "FreeTransform_Begin";
inline constexpr const char* End = "FreeTransform_End";
inline constexpr const char* Drag = "FreeTransform_Drag";
inline constexpr const char* DerivedDrag = "FreeTransform_DerivedDrag";
inline constexpr const char* Move = "FreeTransform_Move";
inline constexpr const char* DerivedMove = "FreeTransform_DerivedMove";
inline constexpr const char* CanvasRefresh = "FreeTransform_CanvasRefresh";
inline constexpr const char* Wrapper = "FreeTransform_Wrapper";
inline constexpr const char* QuadCall = "FreeTransform_QuadCall";
inline constexpr const char* Nearest = "FreeTransform_Nearest";
inline constexpr const char* Bilinear = "FreeTransform_Bilinear";
inline constexpr const char* Oversample = "FreeTransform_Oversample";
inline constexpr const char* ToolSlot = "FreeTransform_ToolSlot";
inline constexpr const char* ToolModeInstruction = "FreeTransform_ToolModeInstruction";
inline constexpr const char* ManipulatorModeInstruction = "FreeTransform_ManipulatorModeInstruction";
inline constexpr const char* ToolSessionInstruction = "FreeTransform_ToolSessionInstruction";
inline constexpr const char* SelectTile = "FreeTransform_SelectTile";
inline constexpr const char* Alpha = "FreeTransform_Alpha";
inline constexpr const char* Color = "FreeTransform_Color";
inline constexpr const char* AlphaPitch = "FreeTransform_AlphaPitch";
inline constexpr const char* ColorPitch = "FreeTransform_ColorPitch";
inline constexpr const char* AlphaBytes = "FreeTransform_AlphaBytes";
inline constexpr const char* ColorBytes = "FreeTransform_ColorBytes";
inline constexpr const char* TileWidth = "FreeTransform_TileWidth";
inline constexpr const char* TileHeight = "FreeTransform_TileHeight";

bool Locate(const uint8_t* image, uint32_t imageSize, uint32_t textRva, uint32_t textSize,
    uint32_t unwindRva, uint32_t unwindSize, Json::Value& table, std::ostream& log);
}
