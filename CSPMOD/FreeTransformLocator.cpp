#include "pch.h"
#include "FreeTransformLocator.h"
#include "NativeCode.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <string_view>
#include <utility>
#include <vector>

using NativeCode::Code;
using NativeCode::FunctionRange;

bool FreeTransform::Locate(const uint8_t* image, uint32_t imageSize, uint32_t textRva, uint32_t textSize,
    uint32_t unwindRva, uint32_t unwindSize, Json::Value& table, std::ostream& log)
{
    const auto fail = [&log](const char* feature) {
        log << "ERROR: " << feature << " missing, ambiguous, or failed its call/layout checks.\n";
        return false;
    };
    if (!image || !textRva || textRva >= imageSize || !textSize || textSize > imageSize - textRva ||
        !unwindRva || unwindRva >= imageSize || unwindSize > imageSize - unwindRva ||
        unwindSize < sizeof(FunctionRange) || unwindSize % sizeof(FunctionRange))
        return fail("FreeTransform PE sections");

    const Code code(image, imageSize, textRva, textSize,
        reinterpret_cast<const FunctionRange*>(image + unwindRva), unwindSize / sizeof(FunctionRange));

    const auto starts = code.Find("BA 25 00 0D 00 48 8D 4C 24 ?? E8 ?? ?? ?? ?? 4C 8D 44 24 ?? BA 04 00 00 00 48 8B CE E8");
    const auto begin = starts.size() == 1 ? code.Call(starts.front() + 28) : 0;
    if (!begin || code.Containing(begin).begin != begin ||
        !code.Match(begin, "48 89 5C 24 20 55 56 57 41 56 41 57 48 8D AC 24 E0 FC FF FF 48 81 EC 20 04 00 00") ||
        !code.Match(begin + 44, "4D 8B F8 44 8B F2 48 8B F1 33 DB"))
        return fail(Begin);

    // The two canvas handlers pass opposing flags to the same teardown function.
    const auto cancels = code.Find("48 0F 7E C1 48 85 C9 74 0A B8 01 00 00 00 F0 0F C1 41 08 41 B9 01 00 00 00 4C 8D 44 24 70 48 8D 55 80 48 8D 4D 90 E8");
    const auto finishes = code.Find("48 0F 7E C1 48 85 C9 74 0A B8 01 00 00 00 F0 0F C1 41 08 45 33 C9 4C 8D 44 24 70 48 8D 55 80 48 8D 4D 90 E8");
    if (finishes.size() != 1 || cancels.size() != 1)
        return fail(End);
    const auto end = code.Call(finishes.front() + 35);
    if (!end || end != code.Call(cancels.front() + 38) || code.Containing(end).begin != end ||
        !code.Match(end, "40 53 56 57 41 54 41 55 41 56 41 57 48 81 EC 70 01 00 00"))
        return fail(End);

    const auto sessions = code.Find("48 8B 1D ?? ?? ?? ?? 48 85 DB 0F 84 ?? ?? ?? ?? 48 8B BB ?? 02 00 00",
        end, code.Containing(end).end);
    const auto modes = code.Find("48 8B 05 ?? ?? ?? ?? 89 A8 ?? ?? 00 00 48 8B 0E E8");
    const auto tool = sessions.size() == 1 ? code.Relative(sessions.front(), 3, 7) : 0;
    if (!tool || tool > imageSize - sizeof(void*) || modes.size() != 1 ||
        code.Relative(modes.front(), 3, 7) != tool)
        return fail("FreeTransform tool session/mode");
    const auto modeChanges = code.Find("41 8B D6 48 8D 4D C0 E8 ?? ?? ?? ?? E9",
        begin, code.Containing(begin).end);
    if (modeChanges.size() != 1 || code.Call(modeChanges.front() + 7) != code.Containing(modes.front()).begin)
        return fail("FreeTransform execution path to mode update");

    // Drag and release both accumulate their dirty rectangle through the same virtual slot.
    const auto dragModes = code.Find("8B 97 9C 01 00 00 8D 42 FF 83 F8 01 0F 86 ?? ?? ?? ?? 83 FA 04 75 ?? 85 C9 75 ??");
    const auto baseDrag = dragModes.size() == 1 ? code.Containing(dragModes.front()) : FunctionRange{};
    if (!code.Match(baseDrag.begin, "48 8B C4 48 89 58 18 55 56 57 41 54 41 55 41 56 41 57 48 8D A8 08 FE FF FF") ||
        code.Find("48 8B DA 48 8B F9 48 8B 02 48 89 45 80 48 8B 4A 08 48 89 4D 88 BE 01 00 00 00 48 85 C9 74 07 8B C6 F0 0F C1 41 08",
            baseDrag.begin, baseDrag.end).size() != 1 ||
        code.Find("48 8D 43 70 48 89 44 24 30 44 89 7C 24 28 8B 43 68 89 44 24 20 45 8B CE 4C 8D 85 20 01 00 00 48 8D 55 A0 48 8B CF 41 FF 92 C8 01 00 00",
            baseDrag.begin, baseDrag.end).size() != 1)
        return fail(Drag);

    // Verify the scale handler's corner and edge action range.
    const auto scaleCalls = code.Find(
        "8B 44 24 40 89 44 24 20 45 8B CC 4C 8D 45 00 48 8B D3 48 8B CF E8",
        baseDrag.begin, baseDrag.end);
    const auto scale = scaleCalls.size() == 1 ? code.Call(scaleCalls.front() + 21) : 0;
    if (!scale || code.Containing(scale).begin != scale ||
        !code.Match(scale, "48 8B C4 48 89 58 20 55 56 57 41 54 41 55 41 56 41 57") ||
        code.Find("8B 7A 30 8D 47 FB 83 F8 07 0F 87 ?? ?? ?? ?? E8",
            scale, code.Containing(scale).end).size() != 1)
        return fail("FreeTransform scale drag handle layout");

    // Modes 1 and 2 can also use a separate scale handler depending on the object's geometry.
    const auto alternateScalePaths = code.Find(
        "0F 28 44 24 50 66 0F 7F 85 90 00 00 00 66 0F 73 D8 08 66 48 0F 7E C1 48 85 C9 74 07 8B C6 F0 0F C1 41 08 "
        "8B 44 24 40 89 44 24 20 45 8B CC 4C 8D 85 90 00 00 00 48 8B D3 48 8B CF E8",
        baseDrag.begin, baseDrag.end);
    const auto scalePath = code.Relative(dragModes.front() + 12, 2, 6);
    const auto alternateScale = alternateScalePaths.size() == 1 && alternateScalePaths.front() == scalePath ?
        code.Call(scalePath + 59) : 0;
    if (!alternateScale || code.Containing(alternateScale).begin != alternateScale ||
        !code.Match(alternateScale, "48 8B C4 48 89 58 10 55 56 57 41 54 41 55 41 56 41 57") ||
        !code.Match(alternateScale + 0xc8, "8B 7A 30"))
        return fail("FreeTransform alternate scale drag path");

    // The derived callback applies layer offsets after the base callback returns.
    // 4.2.0 holds the event in RSI; 5.0.0 holds it in RBX.
    uint32_t derivedDrag = 0;
    uint32_t layerDrag = 0;
    for (const auto& [baseCall, layerCall, prolog] : {
        std::array<std::string_view, 3>{
            "48 8B D3 48 8B CF E8 ?? ?? ?? ?? 48 8B 43 10 48 89 44 24 30 48 8B 4B 18 48 89 4C 24 38",
            "48 8D 43 70 48 89 44 24 20 44 8B 4B 68 41 B8 01 00 00 00 48 8D 54 24 30 48 8B CF E8",
            "48 89 5C 24 10 57 48 83 EC 50 48 8B DA 48 8B F9"},
        std::array<std::string_view, 3>{
            "48 8B D6 48 8B CF E8 ?? ?? ?? ?? 48 8B 46 10 48 89 44 24 30 48 8B 4E 18 48 89 4C 24 38",
            "48 8D 46 70 48 89 44 24 20 44 8B 4E 68 41 B8 01 00 00 00 48 8D 54 24 30 48 8B CF E8",
            "48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 57 48 83 EC 50 48 8B F2 48 8B F9"}})
    {
        for (const auto callSite : code.Find(baseCall))
        {
            if (code.Call(callSite + 6) != baseDrag.begin)
                continue;
            const auto caller = code.Containing(callSite);
            if (!code.Match(caller.begin, prolog))
                continue;
            const auto layers = code.Find(layerCall, callSite + 11, caller.end);
            const auto layer = layers.size() == 1 ? code.Call(layers.front() + 27) : 0;
            if (!layer || code.Containing(layer).begin != layer)
                continue;
            const auto offsets = code.Find("48 8D 55 30 48 8B 4C 24 48 E8 ?? ?? ?? ?? 48 8D 95 30 01 00 00 48 8B 4C 24 48 E8 ?? ?? ?? ?? 0F 10 00 0F 29 45 40",
                layer, code.Containing(layer).end);
            const auto setOffset = offsets.size() == 1 ? code.Call(offsets.front() + 9) : 0;
            if (!setOffset || code.Containing(setOffset).begin != setOffset ||
                !code.Match(setOffset + 31, "83 B9 88 00 00 00 09 48 8B FA 48 8B D9 74 ?? C7 81 88 00 00 00 09 00 00 00"))
                continue;
            if (derivedDrag)
                return fail(DerivedDrag);
            derivedDrag = caller.begin;
            layerDrag = layer;
        }
    }
    if (!derivedDrag)
        return fail(DerivedDrag);

    // The integer delta event has a separate slot, but feeds the same dirty-rect callback.
    const auto moves = code.Find("48 8D 43 30 48 89 44 24 30 C7 44 24 28 11 00 00 00 8B 43 28 89 44 24 20 41 B9 01 00 00 00 4C 8D 45 F0 48 8D 54 24 70 48 8B CF 41 FF 92 C8 01 00 00");
    const auto move = moves.size() == 1 ? code.Containing(moves.front()) : FunctionRange{};
    if (!code.Match(move.begin, "48 89 5C 24 18 55 56 57 48 8D 6C 24 E0 48 81 EC 20 01 00 00") ||
        code.Find("48 8B DA 48 8B F9 48 8B 02 48 89 44 24 60 48 8B 4A 08 48 89 4C 24 68", move.begin, move.end).size() != 1 ||
        code.Find("8B 4B 20 85 C9 7E ?? 01 4D F8 EB ?? 01 4D F0 8B 43 24 85 C0 7E ?? 01 45 FC", move.begin, move.end).size() != 1)
        return fail(Move);

    uint32_t derivedMove = 0;
    for (const auto& [layerCall, baseCall, prolog] : {
        std::array<std::string_view, 3>{
            "48 8D 43 30 48 89 44 24 20 44 8B 4B 28 41 B8 01 00 00 00 48 8D 54 24 30 48 8B CF E8",
            "48 8B D3 48 8B CF E8 ?? ?? ?? ?? 48 8B 43 10 48 89 44 24 30 48 8B 4B 18",
            "48 89 5C 24 10 57 48 83 EC 50 48 8B DA 48 8B F9"},
        std::array<std::string_view, 3>{
            "44 8B 4B 28 48 8D 43 30 41 B8 01 00 00 00 48 89 44 24 20 48 8D 54 24 30 48 8B CF E8",
            "48 8B DA 48 8B F9 E8 ?? ?? ?? ?? 4C 8B 4B 18 48 8B 43 10 48 89 44 24 30",
            "48 89 5C 24 08 57 48 83 EC 40 48 8B DA 48 8B F9"}})
    {
        for (const auto callSite : code.Find(layerCall))
        {
            if (code.Call(callSite + 27) != layerDrag)
                continue;
            const auto caller = code.Containing(callSite);
            if (!code.Match(caller.begin, prolog))
                continue;
            const auto bases = code.Find(baseCall, caller.begin, callSite);
            if (bases.size() != 1 || code.Call(bases.front() + 6) != move.begin)
                continue;
            if (derivedMove)
                return fail(DerivedMove);
            derivedMove = caller.begin;
        }
    }
    if (!derivedMove)
        return fail(DerivedMove);
    const auto releaseSites = code.Find("49 8D 7E 3C 48 8B 06 48 89 7C 24 30 44 89 7C 24 28 44 89 7C 24 20 41 B9 01 00 00 00 4C 8D 85 B0 00 00 00 48 8D 55 A8 48 8B CE FF 90 C8 01 00 00");
    if (releaseSites.size() != 1)
        return fail("FreeTransform mouse release dirty rectangle");
    const auto release = code.Containing(releaseSites.front());
    const auto refreshCalls = code.Find("49 8B 06 48 89 45 B8 49 8B 4E 08 48 89 4D C0 48 85 C9 74 0F B8 01 00 00 00 F0 0F C1 41 08 48 8B 5C 24 58 48 8B D7 48 8D 4D B8 E8",
        releaseSites.front(), release.end);
    const auto refresh = refreshCalls.size() == 1 ? code.Call(refreshCalls.front() + 42) : 0;
    const auto refreshFunction = code.Containing(refresh);
    if (!refresh || refreshFunction.begin != refresh ||
        !code.Match(refresh, "48 89 5C 24 18 48 89 74 24 20 57 48 81 EC 90 00 00 00"))
        return fail(CanvasRefresh);
    const auto invalidateCalls = code.Find("44 8D 47 02 48 8D 54 24 78 48 8B C8 E8 ?? ?? ?? ?? 90",
        refresh, refreshFunction.end);
    const auto invalidate = invalidateCalls.size() == 1 ? code.Call(invalidateCalls.front() + 12) : 0;
    if (!invalidate || code.Containing(invalidate).begin != invalidate ||
        code.Find("41 B9 01 00 00 00 4C 8D 44 24 78 48 8D 94 24 98 00 00 00 48 8B 4C 24 40 E8 ?? ?? ?? ?? 48 8D 54 24 78 48 8B CB E8",
            invalidate, code.Containing(invalidate).end).size() != 1)
        return fail("FreeTransform canvas cache and widget invalidation");

    // Resolve both blitters from the call chain that receives the two quadrilaterals.
    const auto quad = code.Find("44 89 44 24 30 44 89 74 24 28 48 8B 44 24 50 48 89 44 24 20 4C 8D 4D ?? 4C 8D 44 24 ?? 48 8D 54 24 ?? 48 8D 4C 24 ?? E8");
    const auto quadCall = quad.size() == 1 ? quad.front() + 39 : 0;
    const auto wrapper = code.Call(quadCall);
    if (!wrapper || code.Containing(wrapper).begin != wrapper ||
        !code.Match(wrapper, "48 8B C4 48 89 58 18 48 89 50 10 48 89 48 08 56 57 41 56"))
        return fail(Wrapper);
    const auto forwards = code.Find("4C 8D 4C 24 60 48 8D 54 24 40 48 8D 4C 24 50 E8",
        wrapper, code.Containing(wrapper).end);
    const auto core = forwards.size() == 1 ? code.Call(forwards.front() + 15) : 0;
    if (!core || code.Containing(core).begin != core ||
        !code.Match(core, "40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 B8 FC FF FF 48 81 EC 48 04 00 00"))
        return fail("FreeTransform quad core");
    const auto coreEnd = code.Containing(core).end;
    const auto nearestCalls = code.Find("E8 ?? ?? ?? ?? E9 ?? ?? ?? ?? 83 FF 03 75 ??", core, coreEnd);
    const auto linearCalls = code.Find("48 8B D3 48 8D 8D 90 00 00 00 E8 ?? ?? ?? ?? 48 8D 55 C0", core, coreEnd);
    const auto nearest = nearestCalls.size() == 1 ? code.Call(nearestCalls.front()) : 0;
    const auto linear = linearCalls.size() == 1 ? code.Call(linearCalls.front() + 10) : 0;
    if (!code.Match(nearest, "44 89 4C 24 20 44 89 44 24 18 48 89 4C 24 08 53 55 56 57 41 54 41 55 41 57 48 81 EC C0 00 00 00"))
        return fail(Nearest);
    if (!code.Match(linear, "48 89 5C 24 08 44 89 4C 24 20 44 89 44 24 18 55 56 57 41 54 41 55 41 56 41 57 48 81 EC B0 00 00 00 4C 8B E1"))
        return fail(Bilinear);

    const auto oversample = nearestCalls.size() == 1 ? code.Call(nearestCalls.front() + 0x2a) : 0;
    if (!code.Match(oversample, "40 53 55 56 57 41 56 48 81 EC 50 01 00 00 0F 29 BC 24 20 01 00 00") ||
        code.Containing(oversample).begin != oversample)
        return fail(Oversample);

    // Confirm the sampler context, unmasked native pixel views, and exact getter semantics.
    const auto contextCalls = code.Find("44 89 6C 24 20 4C 8D 4C 24 50 4C 8D 44 24 40 48 8D 54 24 60 48 8D 8D 90 00 00 00 E8 ?? ?? ?? ?? 90 0F 28 45 D0",
        core, coreEnd);
    const auto context = contextCalls.size() == 1 ? code.Call(contextCalls.front() + 27) : 0;
    const auto viewConstructor = context ? code.Call(context + 0x37) : 0;
    if (!context ||
        !code.Match(context + 0x30, "48 8B 12 48 83 C1 08 E8 ?? ?? ?? ?? 90 48 8D 8E 40 01 00 00 49 8B 17 E8") ||
        !viewConstructor || code.Call(context + 0x47) != viewConstructor ||
        !code.Match(viewConstructor + 0x3a, "48 89 47 10 48 8B C7 48 89 5F 08") ||
        !code.Match(context + 0x4d, "0F 10 03 0F 11 86 78 02 00 00 C7 86 88 02 00 00 01 00 00 00 8B 44 24 70 89 86 8C 02 00 00") ||
        !code.Match(context + 0x95, "89 86 94 02 00 00 F6 C3 20 74 0A C7 86 94 02 00 00 03 00 00 00 49 8B 0E E8 ?? ?? ?? ?? 89 86 98 02 00 00"))
        return fail("FreeTransform sampler context");

    const auto prepareContext = code.Call(core + 0x2c3);
    if (!code.Match(prepareContext + 0x15,
        "0F 10 02 48 8B D9 4C 8D 81 78 02 00 00 48 83 C1 08 33 D2 0F 11 44 24 20 E8 ?? ?? ?? ?? 4C 8D 44 24 20 BA 01 00 00 00 48 8D 8B 40 01 00 00 E8"))
        return fail("FreeTransform prepared source and destination views");
    const auto prepareView = code.Call(prepareContext + 0x2d);
    if (!prepareView || code.Call(prepareContext + 0x43) != prepareView ||
        !code.Match(prepareView + 0x8e, "0F 10 00 0F 11 43 28"))
        return fail("FreeTransform prepared view bounds");

    const auto alpha = code.Call(nearest + 0x2c);
    const auto color = code.Call(nearest + 0x3c);
    const auto alphaPitch = code.Call(nearest + 0x4c);
    const auto colorPitch = code.Call(nearest + 0x57);
    const auto alphaBytes = code.Call(nearest + 0x66);
    const auto colorBytes = code.Call(nearest + 0x72);
    if (!code.Match(alpha, "48 8B 41 58 C3") || !code.Match(color, "48 8B 41 60 C3") ||
        !code.Match(alphaPitch, "8B 81 A0 00 00 00 C3") || !code.Match(colorPitch, "8B 81 98 00 00 00 C3") ||
        !code.Match(alphaBytes, "8B 81 9C 00 00 00 C3") || !code.Match(colorBytes, "8B 81 94 00 00 00 C3") ||
        code.Call(linear + 0x30) != alpha || code.Call(linear + 0x40) != color ||
        code.Call(linear + 0x50) != alphaPitch || code.Call(linear + 0x5b) != colorPitch ||
        code.Call(linear + 0x67) != colorBytes || code.Call(oversample + 0x43) != alpha ||
        code.Call(oversample + 0x56) != color || code.Call(oversample + 0x69) != alphaPitch ||
        code.Call(oversample + 0x74) != colorPitch)
        return fail("FreeTransform pixel plane getters");

    const auto tile = code.Call(core + 0x30a);
    if (!code.Match(tile, "48 83 EC 38 4C 8B D1 48 8D 41 28 48 89 44 24 28 45 8B C8 44 8B C2 48 83 C1 38 41 8B 42 10 49 8B 52 08"))
        return fail(SelectTile);
    const auto selectTileData = code.Call(tile + 0x26);
    if (!selectTileData ||
        !code.Match(selectTileData + 0x19c, "89 73 38 89 4B 40 8B 4B 74 03 CF 89 7B 3C 89 4B 44") ||
        !code.Match(selectTileData + 0x1e8, "44 2B 73 3C 44 2B 7B 38 41 8B C6"))
        return fail("FreeTransform tile bounds and pixel coordinates");

    const auto dimensions = code.Find("E8 ?? ?? ?? ?? 48 8B CB 89 47 28 E8 ?? ?? ?? ?? 8B 4F 28 44 8B C8 89 47 2C");
    const auto tileWidth = dimensions.size() == 1 ? code.Call(dimensions.front()) : 0;
    const auto tileHeight = dimensions.size() == 1 ? code.Call(dimensions.front() + 11) : 0;
    if (!code.Match(tileWidth, "8B 81 C0 00 00 00 C3") || !code.Match(tileHeight, "8B 81 C4 00 00 00 C3"))
        return fail("FreeTransform tile dimensions");

    const std::pair<const char*, uint32_t> entries[] = {
        {Begin, begin}, {End, end}, {Drag, baseDrag.begin}, {DerivedDrag, derivedDrag},
        {Move, move.begin}, {DerivedMove, derivedMove},
        {CanvasRefresh, refresh}, {Wrapper, wrapper}, {QuadCall, quadCall},
        {Nearest, nearest}, {Bilinear, linear}, {Oversample, oversample}, {ToolSlot, tool},
        {ToolModeInstruction, modes.front()}, {ToolSessionInstruction, sessions.front()},
        {SelectTile, tile}, {Alpha, alpha}, {Color, color}, {AlphaPitch, alphaPitch},
        {ColorPitch, colorPitch}, {AlphaBytes, alphaBytes}, {ColorBytes, colorBytes},
        {TileWidth, tileWidth}, {TileHeight, tileHeight}
    };
    for (const auto& [name, address] : entries)
    {
        table[name] = address;
        log << "Located " << name << '\n';
    }
    return true;
}
