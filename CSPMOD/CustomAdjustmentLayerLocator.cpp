#include "CustomAdjustmentLayer.h"

#include "NativeCode.h"
#include "json/json.h"

#include <ostream>
#include <string>

namespace CustomAdjustmentLayer
{
bool Locate(const uint8_t* image, uint32_t imageSize, uint32_t textRva, uint32_t textSize,
    uint32_t unwindRva, uint32_t unwindSize, Json::Value& entries, std::ostream& log)
{
    const auto fail = [&](const char* feature) {
        log << "ERROR: CustomAdjustment_" << feature << " missing, ambiguous or incompatible\n";
        return false;
    };
    if (!image || textRva >= imageSize || textSize > imageSize - textRva || unwindRva >= imageSize ||
        !unwindSize || unwindSize > imageSize - unwindRva || unwindSize % sizeof(NativeCode::FunctionRange))
        return fail("Image");
    const auto* functions = reinterpret_cast<const NativeCode::FunctionRange*>(image + unwindRva);
    const NativeCode::Code code(image, imageSize, textRva, textSize, functions,
        unwindSize / sizeof(NativeCode::FunctionRange));
    const auto matchesString = [&](uint32_t at, std::string_view name) {
        return at && name.size() < imageSize - at && image[at + name.size()] == 0 &&
            memcmp(image + at, name.data(), name.size()) == 0;
    };

    // The unsigned branch bounds the host-owned range. Its size can grow without
    // changing the custom IDs, as long as the dispatch and invert ABI still match.
    const auto typeStores = code.Find("48 8B 06 44 89 78 08 45 85 FF 0F 84 ?? ?? ?? ?? "
        "41 8D 47 FF 83 F8 ?? 0F 87 ?? ?? ?? ?? 48 98 48 8D 15 ?? ?? ?? ?? "
        "8B 8C 82 ?? ?? ?? ?? 48 03 CA FF E1");
    if (typeStores.size() != 1)
        return fail("FactoryTypeDispatch");
    const uint32_t typeStore = typeStores.front();
    const auto factory = code.Containing(typeStore);
    if (!factory.begin || !code.Match(factory.begin, "48 89 5C 24 10 55 56 57 41 54 41 55 41 56 41 57"))
        return fail("FactoryPrologue");
    const auto vtableStores = code.Find("48 8D 05 ?? ?? ?? ?? 48 89 07 89 5F 08 B9 A0 00 00 00",
        factory.begin, factory.end);
    if (vtableStores.size() != 1)
        return fail("FactoryVtable");
    const uint32_t vtable = code.Relative(vtableStores.front(), 3, 7);
    const auto destroyUses = code.Find("48 8D 05 ?? ?? ?? ?? 48 89 01 48 8B 81 A8 00 00 00");
    if (!vtable || std::count_if(destroyUses.begin(), destroyUses.end(), [&](uint32_t at) {
            return code.Relative(at, 3, 7) == vtable;
        }) != 1)
        return fail("FactoryVtable");

    constexpr uint32_t invert = 6;
    const uint32_t highestIndex = image[typeStore + 22];
    const uint32_t nativeTypeCount = highestIndex + 1;
    if (highestIndex >= 0x80 || nativeTypeCount < invert || nativeTypeCount >= UserStart)
        return fail("NativeTypeRange");

    // The dispatch uses the image base and a table of 32-bit code offsets.
    const uint32_t tableInstruction = typeStore + 38;
    int32_t baseDisplacement = 0;
    memcpy(&baseDisplacement, image + tableInstruction - 4, sizeof(baseDisplacement));
    uint32_t jumpTable = 0;
    memcpy(&jumpTable, image + tableInstruction + 3, sizeof(jumpTable));
    if (static_cast<int64_t>(tableInstruction) + baseDisplacement != 0 ||
        jumpTable >= imageSize || nativeTypeCount * sizeof(uint32_t) > imageSize - jumpTable)
        return fail("FactoryJumpTable");
    uint32_t invertBranch = 0;
    for (uint32_t type = 1; type <= nativeTypeCount; ++type)
    {
        uint32_t branch = 0;
        memcpy(&branch, image + jumpTable + (type - 1) * sizeof(branch), sizeof(branch));
        if (branch < tableInstruction + 12 || branch >= factory.end)
            return fail("FactoryJumpTable");
        if (type == invert)
            invertBranch = branch;
    }
    if (!code.Call(invertBranch))
        return fail("InvertType");
    const auto invertSetup = code.Find("48 8D 54 24 20 48 8B 0E E8 ?? ?? ?? ?? E9", invertBranch,
        (std::min)(invertBranch + 0xb0, factory.end));
    if (invertSetup.size() != 1)
        return fail("InvertSetup");
    const auto invertTypes = code.Find("48 8D 15 ?? ?? ?? ?? E8 ?? ?? ?? ?? 85 C0",
        invertBranch, invertSetup.front());
    if (std::count_if(invertTypes.begin(), invertTypes.end(), [&](uint32_t at) {
            const uint32_t type = code.Relative(at, 3, 7);
            return type && type < imageSize - 8 &&
                matchesString(type + 8, ".?AUSAdjustmentInvert@Planeswalker@@");
        }) != 1)
        return fail("InvertType");

    const auto newInfo = code.Find("0F 57 C0 F3 0F 7F 44 24 38 4C 8D 44 24 38 "
        "E8 ?? ?? ?? ?? B9 01 00 00 00");
    if (newInfo.size() != 1 || code.Call(newInfo.front() + 14) != factory.begin)
        return fail("CreateInfo");
    const auto newInfoFunction = code.Containing(newInfo.front());
    const auto invertCommands = code.Find("BA 06 00 00 00 48 8D 4C 24 60 E8 ?? ?? ?? ?? 90 0F 28 44 24 60");
    if (!newInfoFunction.begin || std::count_if(invertCommands.begin(), invertCommands.end(), [&](uint32_t at) {
            return code.Call(at + 10) == newInfoFunction.begin;
        }) != 1)
        return fail("NativeInvertCommand");

    const auto loads = code.Find("E8 ?? ?? ?? ?? 0F 57 C0 F3 0F 7F 44 24 70 4C 8D 44 24 70 "
        "8B D0 48 8D 4D 90 E8 ?? ?? ?? ??");
    if (loads.size() != 1 || code.Call(loads.front() + 25) != factory.begin ||
        !code.Match(code.Call(loads.front()), "48 83 EC 28 BA 04 00 00 00 E8 ?? ?? ?? ?? "
            "0F B6 48 01 0F B6 10 C1 E2 08 0B D1 0F B6 48 02 0F B6 40 03 C1 E2 08 0B D1 C1 E2 08 0B C2"))
        return fail("ReadType32");
    const auto saves = code.Find("49 8B 04 24 8B 50 08 48 8D 4D B0 E8 ?? ?? ?? ?? "
        "48 8D 4D B0 E8 ?? ?? ?? ?? 48 8B F8 33 D2 48 8D 4D B0 E8");
    if (saves.size() != 1)
        return fail("WriteType32");
    const auto writer = code.Containing(code.Call(saves.front() + 11));
    if (!writer.begin || code.Find("C1 E8 18 88 02 8B C7 C1 E8 10 88 42 01 "
        "8B C7 C1 E8 08 88 42 02 40 88 7A 03", writer.begin, writer.end).size() != 1)
        return fail("WriteType32");

    uint32_t commandToken = 0;
    for (uint32_t at : code.Find("4C 8D 05 ?? ?? ?? ?? 48 8D 15 ?? ?? ?? ?? 48 8D 0D ?? ?? ?? ?? E9 ?? ?? ?? ??"))
    {
        if (!matchesString(code.Relative(at, 3, 7), "layeradjustmentinvertnew"))
            continue;
        if (commandToken)
            return fail("InvertCommandToken");
        commandToken = code.Relative(at + 14, 3, 7);
    }
    if (!commandToken)
        return fail("InvertCommandToken");

    uint32_t lookup = 0;
    uint32_t dispatch = 0;
    for (uint32_t at : code.Find("48 8D 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 8B C8 E8 ?? ?? ?? ?? 41 8B C7 E9 ?? ?? ?? ??"))
    {
        if (code.Relative(at, 3, 7) != commandToken)
            continue;
        if (dispatch)
            return fail("CommandDispatch");
        lookup = code.Call(at + 7);
        dispatch = code.Call(at + 14);
    }
    if (!lookup || !code.Match(dispatch, "33 D2 E9 ?? ?? ?? ??") ||
        !code.Match(code.Relative(dispatch + 2, 1, 5), "48 83 EC 28 48 8B 05 ?? ?? ?? ?? 48 85 C0 74 ?? "
            "83 3D ?? ?? ?? ?? 00 7F ?? 85 C9 74 ?? 45 33 C9 44 8B C2 8B D1 48 8B C8 E8"))
        return fail("CommandDispatch");

    // Use a native no-parameter layer to locate the shared callback setters.
    // The custom effect installs its own callbacks and does not run this effect.
    const auto config = code.Containing(code.Call(invertSetup.front() + 8));
    if (!config.begin)
        return fail("CallbackConfiguration");
    uint32_t setStreams = 0;
    for (const auto at : code.Find("48 8D 55 ?? 48 8B CF E8", config.begin, config.end))
    {
        const auto target = code.Call(at + 7);
        const auto setter = code.Containing(target);
        if (!target || setter.begin != target ||
            code.Find("48 8D 71 30 45 33 FF", setter.begin, setter.end).size() != 1 ||
            code.Find("49 8D 76 58", setter.begin, setter.end).size() != 1)
            continue;
        if (setStreams)
            return fail("AmbiguousParameterStreamSetter");
        setStreams = target;
    }
    if (!setStreams)
        return fail("ParameterStreamSetterLayout");

    uint32_t setRender = 0;
    NativeCode::FunctionRange nativeRender{};
    const auto callbacks = code.Find("48 8D 05 ?? ?? ?? ?? 48 89 45 ?? 48 8D 05 ?? ?? ?? ?? "
        "48 83 C8 01 48 89 45 ?? 48 8D 55 ?? 48 8B CF E8", config.begin, config.end);
    for (const auto at : callbacks)
    {
        // The tagged callback and the argument to the setter use the same slot.
        if (image[at + 25] != image[at + 29])
            continue;
        const auto target = code.Call(at + 33);
        const auto setter = code.Containing(target);
        if (!target || setter.begin != target ||
            code.Find("48 8D B9 80 00 00 00 33 C0 48 89 44 24 28 4C 8B 0A 4D 85 C9",
                setter.begin, setter.end).size() != 1)
            continue;
        const auto callback = code.Relative(at, 3, 7);
        const auto render = code.Containing(callback);
        if (!callback || render.begin != callback || setRender)
            return fail("RenderCallbackLayout");
        setRender = target;
        nativeRender = render;
    }
    if (!setRender)
        return fail("RenderCallbackSetterLayout");

    const auto formats = code.Find("48 8B CA 49 8B F9 49 8B F0 48 8B DA E8 ?? ?? ?? ?? 83 F8 01 75 ?? "
        "48 8D 05 ?? ?? ?? ??", nativeRender.begin, nativeRender.end);
    const auto renderCalls = code.Find("4C 8B C7 48 8B D6 48 8B CB E8 ?? ?? ?? ??",
        nativeRender.begin, nativeRender.end);
    if (formats.size() != 1 || renderCalls.size() != 1)
        return fail("NativeRowRender");
    const uint32_t getFormat = code.Call(formats.front() + 12);
    const uint32_t invertRow = code.Relative(formats.front() + 22, 3, 7);
    const auto renderer = code.Containing(code.Call(renderCalls.front() + 9));
    if (!getFormat || !renderer.begin ||
        !code.Match(invertRow, "45 33 D2 44 39 51 10 7E 46 4C 8B 44 24 28 66 90 41 80 38 00 76 1C "
            "41 0F B6 41 02 F6 D0 88 42 02") ||
        !code.Match(invertRow + 50, "48 63 41 08 41 FF C2 4C 03 C8 48 63 41 0C 4C 03 C0 48 63 01 48 03 D0"))
        return fail("RowRenderLayout");

    // The renderer loads the same two planes for each accessor. The callback
    // receives destination color/auxiliary in registers and source color/alpha on the stack.
    const auto planes = code.Find("48 8B CE E8 ?? ?? ?? ?? 48 8B E8 48 8B CE E8 ?? ?? ?? ?? 4C 8B F0 "
        "48 8B CF E8 ?? ?? ?? ?? 4C 8B F8 48 8B CF E8 ?? ?? ?? ?? 48 8B F8 48 85 C0",
        renderer.begin, renderer.end);
    const auto invokeRow = code.Find("4C 8B 13 4D 85 D2 0F 84 ?? ?? ?? ?? 49 83 E2 FE 48 8D 4B 08 "
        "48 89 7C 24 28 4C 89 7C 24 20 4D 8B CE 4C 8B C5 48 8D 94 24 88 00 00 00 41 FF 52 08",
        renderer.begin, renderer.end);
    if (planes.size() != 1 || invokeRow.size() != 1 || planes.front() >= invokeRow.front())
        return fail("RowRenderPlanes");
    const auto colorPlane = code.Call(planes.front() + 3);
    const auto auxiliaryPlane = code.Call(planes.front() + 14);
    if (!code.Match(colorPlane, "48 8B 41 60 C3") || !code.Match(auxiliaryPlane, "48 8B 41 58 C3") ||
        code.Call(planes.front() + 25) != colorPlane || code.Call(planes.front() + 36) != auxiliaryPlane)
        return fail("RowRenderPlanes");

    const uint32_t read = code.Call(loads.front());
    const auto readBuffer = code.Containing(code.Call(read + 9));
    const auto loadFunction = code.Containing(loads.front());
    const auto saveFunction = code.Containing(saves.front());
    if (!readBuffer.begin || !loadFunction.begin || !saveFunction.begin ||
        code.Find("48 63 4D 58 4C 8B F8 8B 45 5C 8B D0 2B D1 41 3B D6",
            readBuffer.begin, readBuffer.end).size() != 1 ||
        code.Find("49 8B 17 48 8B 42 58 48 85 C0 0F 84 ?? ?? ?? ?? 48 83 E0 FE 48 8D 4A 60 4C 8D 45 C0 FF 50 08",
            loadFunction.begin, loadFunction.end).size() != 1 ||
        code.Find("4C 8B 4A 30 4D 85 C9 0F 84 ?? ?? ?? ?? 49 83 E1 FE 48 8D 4A 38 4C 8D 45 B0 41 FF 51 08",
            saveFunction.begin, saveFunction.end).size() != 1)
        return fail("ParameterStreamLayout");

    const auto selected = code.Find("48 8D 54 24 60 48 8B 0E E8 ?? ?? ?? ?? 90 0F 28 44 24 60 66 0F 7F 45 40");
    if (selected.size() != 1)
        return fail("SelectedLayer");
    const uint32_t getLayer = code.Call(selected.front() + 8);
    const auto layerGetter = code.Containing(getLayer);
    const auto settings = code.Containing(selected.front());
    if (!layerGetter.begin || !settings.begin ||
        code.Find("48 8B 89 00 01 00 00 48 85 C9", layerGetter.begin, layerGetter.end).size() != 1)
        return fail("SelectedLayer");
    const auto editCalls = code.Find("44 89 64 24 20 4C 8D 8D ?? ?? ?? ?? 4C 8D 44 24 78 "
        "48 8D 55 50 48 8D 4D A0 E8 ?? ?? ?? ?? 90", settings.begin, settings.end);
    if (editCalls.size() != 1)
        return fail("EditTransaction");
    const auto edit = code.Containing(code.Call(editCalls.front() + 25));
    if (!edit.begin)
        return fail("EditTransaction");
    const auto make = code.Find("48 8D 55 C7 48 8D 4D 0F E8 ?? ?? ?? ?? 90 49 8B D7 48 8B 4D 0F E8", edit.begin, edit.end);
    const auto commit = code.Find("48 8D 55 1F 48 8B 0E E8 ?? ?? ?? ?? EB ??", edit.begin, edit.end);
    const auto refresh = code.Find("48 8D 55 E7 48 8D 4D F7 E8 ?? ?? ?? ?? 90 BF FF FF FF FF", edit.begin, edit.end);
    if (make.size() != 1 || commit.size() != 1 || refresh.size() != 1)
        return fail("EditTransaction");
    const uint32_t createUndo = code.Call(make.front() + 8);
    const uint32_t submitUndo = code.Call(commit.front() + 7);
    const uint32_t notifyChange = code.Call(refresh.front() + 8);
    const auto undoFactory = code.Containing(createUndo);
    if (!undoFactory.begin || !submitUndo || !notifyChange)
        return fail("EditTransaction");
    const auto snapshots = code.Find("48 89 55 E0 48 89 4D E8 48 8D 4D E0 E8 ?? ?? ?? ?? 85 C0 75",
        undoFactory.begin, undoFactory.end);
    if (snapshots.size() != 1)
        return fail("UndoSnapshot");
    const auto snapshot = code.Containing(code.Call(snapshots.front() + 12));
    if (!snapshot.begin)
        return fail("UndoSnapshot");
    const auto getters = code.Find("48 8D 54 24 20 48 8D 4C 24 30 E8 ?? ?? ?? ?? 90 BE FF FF FF FF 48 83 7C 24 30 00",
        snapshot.begin, snapshot.end);
    if (getters.size() != 1)
        return fail("LayerProperty");
    const auto getInfo = code.Containing(code.Call(getters.front() + 10));
    if (!getInfo.begin)
        return fail("LayerProperty");
    const auto properties = code.Find("4C 8B C1 BA BC 02 00 00 48 8B 0B E8 ?? ?? ?? ?? 90 48 8B 5B 08",
        getInfo.begin, getInfo.end);
    if (properties.size() != 1 || !code.Call(properties.front() + 11))
        return fail("LayerProperty");
    const auto roots = code.Find("40 53 48 83 EC 20 48 83 3D ?? ?? ?? ?? 00 48 8B D9 74 36 E8 ?? ?? ?? ?? "
        "8B C8 E8 ?? ?? ?? ?? 85 C0 75 2E 48 8B CB");
    if (roots.size() != 1 || !code.Relative(roots.front() + 6, 3, 8))
        return fail("ControllerRoot");

    const auto typeQueries = code.Find("48 8D 4D 80 E8 ?? ?? ?? ?? 8B F0 48 8D 4D 30 E8 ?? ?? ?? ??");
    if (typeQueries.size() != 1)
        return fail("LayerSettingsTypeQuery");
    const uint32_t typeQuery = code.Call(typeQueries.front() + 4);
    const auto palette = code.Containing(typeQueries.front());
    const auto typeQueryFunction = code.Containing(typeQuery);
    if (!palette.begin || !typeQueryFunction.begin ||
        !code.Match(typeQuery, "48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20") ||
        code.Find("45 33 C9 4C 8D 44 24 20 8B D6 48 8B C8 E8 ?? ?? ?? ?? 90", palette.begin, palette.end).size() != 1)
        return fail("LayerSettingsTypeQuery");
    const auto typeProperties = code.Find("BA BC 02 00 00 48 8B 09 E8 ?? ?? ?? ?? 48 8B 4C 24 20",
        typeQueryFunction.begin, typeQueryFunction.end);
    if (typeProperties.size() != 1 || code.Call(typeProperties.front() + 8) != code.Call(properties.front() + 11))
        return fail("LayerSettingsProperty");
    const auto dialogQueries = code.Find("48 8D 4D 87 E8 ?? ?? ?? ?? FF C8 83 F8 08 0F 87 ?? ?? ?? ?? 48 98");
    if (dialogQueries.size() != 1 || code.Call(dialogQueries.front() + 4) != typeQuery)
        return fail("LayerSettingsDialogQuery");

    const uint32_t undoDispatch = dispatch + 16;
    if (!code.Match(undoDispatch, "33 D2 E9 ?? ?? ?? ??"))
        return fail("UndoDispatch");

    // The edit handler dispatches redo and undo in adjacent guarded branches.
    uint32_t undoToken = 0;
    for (uint32_t at : code.Find("48 8D 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 8B C8 E8 ?? ?? ?? ?? E9 ?? ?? ?? ?? "
             "E8 ?? ?? ?? ?? 85 C0 0F 84 ?? ?? ?? ?? "
             "48 8D 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 8B C8 E8 ?? ?? ?? ?? E9 ?? ?? ?? ?? "
             "48 8D 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 3B F8 0F 85"))
    {
        const uint32_t undo = at + 37;
        const uint32_t exit = code.Relative(at + 19, 1, 5);
        const auto handler = code.Containing(at);
        const uint32_t token = code.Relative(undo, 3, 7);
        if (!handler.begin || exit < handler.begin || exit >= handler.end || !token ||
            token == code.Relative(at, 3, 7) || code.Call(at + 7) != lookup || code.Call(undo + 7) != lookup ||
            code.Call(at + 14) != undoDispatch || code.Call(undo + 14) != undoDispatch ||
            code.Relative(at + 31, 2, 6) != exit || code.Relative(undo + 19, 1, 5) != exit)
            continue;
        if (undoToken)
            return fail("UndoToken");
        undoToken = token;
    }
    if (!undoToken)
        return fail("UndoToken");
    const uint32_t undoBody = code.Relative(undoDispatch + 2, 1, 5);
    const uint32_t dispatchBody = code.Relative(dispatch + 2, 1, 5);
    if (!code.Match(undoBody, "48 83 EC 28 48 8B 05 ?? ?? ?? ?? 48 85 C0 74 ?? "
            "83 3D ?? ?? ?? ?? 00 7F ?? 85 C9 74 ?? 41 B9 01 00 00 00 44 8B C2 8B D1 48 8B C8 E8") ||
        code.Call(undoBody + 43) != code.Call(dispatchBody + 40) || !code.Call(undoBody + 43))
        return fail("UndoDispatch");

    entries["CustomAdjustment_Factory"] = factory.begin;
    entries["CustomAdjustment_InfoVtable"] = vtable;
    entries["CustomAdjustment_CreateReturn"] = newInfo.front() + 19;
    entries["CustomAdjustment_CommandToken"] = commandToken;
    entries["CustomAdjustment_CommandLookup"] = lookup;
    entries["CustomAdjustment_CommandDispatch"] = dispatch;
    entries["CustomAdjustment_SetStreams"] = setStreams;
    entries["CustomAdjustment_SetRender"] = setRender;
    entries["CustomAdjustment_RenderRows"] = renderer.begin;
    entries["CustomAdjustment_GetFormat"] = getFormat;
    entries["CustomAdjustment_Read32"] = read;
    entries["CustomAdjustment_Write32"] = writer.begin;
    entries["CustomAdjustment_GetLayer"] = getLayer;
    entries["CustomAdjustment_GetLayerProperty"] = code.Call(properties.front() + 11);
    entries["CustomAdjustment_CreateUndo"] = createUndo;
    entries["CustomAdjustment_SubmitUndo"] = submitUndo;
    entries["CustomAdjustment_NotifyChange"] = notifyChange;
    entries["CustomAdjustment_ControllerRoot"] = code.Relative(roots.front() + 6, 3, 8);
    entries["CustomAdjustment_QueryType"] = typeQuery;
    entries["CustomAdjustment_PaletteTypeReturn"] = typeQueries.front() + 9;
    entries["CustomAdjustment_DialogTypeReturn"] = dialogQueries.front() + 9;
    entries["CustomAdjustment_UndoToken"] = undoToken;
    entries["CustomAdjustment_UndoDispatch"] = undoDispatch;
    log << "CustomAdjustment: verified factory, row renderer, parameter streams, layer settings and undo\n";
    return true;
}

bool LocateDialogNavigation(const uint8_t* image, uint32_t imageSize, uint32_t textRva, uint32_t textSize,
    uint32_t unwindRva, uint32_t unwindSize, Json::Value& entries, std::ostream& log)
{
    const auto fail = [&](const char* feature) {
        log << "ERROR: CustomAdjustment_Dialog" << feature << " missing, ambiguous or incompatible\n";
        return false;
    };
    if (!image || textRva >= imageSize || textSize > imageSize - textRva || unwindRva >= imageSize ||
        !unwindSize || unwindSize > imageSize - unwindRva || unwindSize % sizeof(NativeCode::FunctionRange))
        return fail("NavigationImage");
    const auto* functions = reinterpret_cast<const NativeCode::FunctionRange*>(image + unwindRva);
    const NativeCode::Code code(image, imageSize, textRva, textSize, functions,
        unwindSize / sizeof(NativeCode::FunctionRange));

    const auto toolGuards = code.Find("41 8B E8 4C 8B F2 48 8B D9 48 8D 05 ?? ?? ?? ?? 48 89 01 "
        "33 C0 48 89 41 08 48 89 41 10 48 89 41 20 48 89 41 28 89 41 18");
    const auto toolCleanups = code.Find("48 8B F1 48 8D 05 ?? ?? ?? ?? 48 89 01 48 8B 41 08 "
        "48 89 44 24 30 48 8B 49 10 48 89 4C 24 38");
    if (toolGuards.size() != 1 || toolCleanups.size() != 1)
        return fail("CanvasToolGuard");
    const auto toolGuard = code.Containing(toolGuards.front());
    const auto toolCleanup = code.Containing(toolCleanups.front());
    const auto toolVtable = code.Relative(toolGuards.front() + 9, 3, 7);
    if (!toolGuard.begin || !toolCleanup.begin || !toolVtable ||
        code.Relative(toolCleanups.front() + 3, 3, 7) != toolVtable)
        return fail("CanvasToolGuard");

    // PWAutoModalHiddenTool uses mode 1 for the canvas tool while a curve dialog runs.
    const auto getBefore = code.Find("48 8D 4C 24 30 E8 ?? ?? ?? ?? 90 48 8B 4C 24 30 "
        "48 85 C9 74 ?? 83 FD 01 74", toolGuard.begin, toolGuard.end);
    const auto getAfter = code.Find("48 8D 4C 24 20 E8 ?? ?? ?? ?? 48 8B 4C 24 20 48 85 C9 74",
        toolCleanup.begin, toolCleanup.end);
    const auto enter = code.Find("BA 01 00 00 00 E8 ?? ?? ?? ?? C7 43 18 02 00 00 00",
        toolGuard.begin, toolGuard.end);
    const auto leave = code.Find("48 8D 54 24 40 E8 ?? ?? ?? ?? 48 8B 5C 24 28",
        toolCleanup.begin, toolCleanup.end);
    if (getBefore.size() != 1 || getAfter.size() != 1 || enter.size() != 1 || leave.size() != 1)
        return fail("CanvasToolCalls");
    const auto getTools = code.Call(getBefore.front() + 5);
    const auto beginCanvas = code.Call(enter.front() + 5);
    const auto endCanvas = code.Call(leave.front() + 5);
    const bool knownCanvasModeField = code.Match(endCanvas + 34, "A0 00 00 00") ||
        code.Match(endCanvas + 34, "90 00 00 00");
    if (!getTools || code.Call(getAfter.front() + 5) != getTools ||
        !code.Match(beginCanvas, "48 89 5C 24 10 57 48 83 EC 60 8B FA 48 8B D9 48 8D 54 24 50 "
            "48 8B 49 40 E8 ?? ?? ?? ?? 90 0F 57 C0 F3 0F 7F 44 24 30") ||
        !code.Match(endCanvas, "48 89 5C 24 18 48 89 54 24 10 56 57 41 56 48 81 EC 90 00 00 00 "
            "4C 8B F2 48 8B D9 33 F6 8D 7E FF 39 B1 ?? 00 00 00 74 ?? 48 8B 0A 48 85 C9 74 ?? E8") ||
        !knownCanvasModeField)
        return fail("CanvasToolABI");
    const auto toolGetter = code.Containing(getTools);
    const auto cast = toolGetter.begin == getTools ? code.Find(
        "4C 8D 0D ?? ?? ?? ?? 4C 8D 05 ?? ?? ?? ?? 33 D2 48 8B 0B E8",
        toolGetter.begin, toolGetter.end) : std::vector<uint32_t>{};
    const auto matchesType = [&](uint32_t type, std::string_view name) {
        return type && imageSize - type > 16 + name.size() &&
            memcmp(image + type + 16, name.data(), name.size()) == 0 && image[type + 16 + name.size()] == 0;
    };
    if (cast.size() != 1 ||
        !matchesType(code.Relative(cast.front(), 3, 7), ".?AVPWToolManager@Planeswalker@@") ||
        !matchesType(code.Relative(cast.front() + 7, 3, 7), ".?AVPWToolManagerBase@Planeswalker@@"))
        return fail("CanvasToolManager");

    const auto modalGuards = code.Find("41 8B F8 44 8B FA 48 8B F1 48 89 4D 87 48 8D 05 ?? ?? ?? ?? "
        "48 89 01 48 8D 59 08");
    if (modalGuards.size() != 1)
        return fail("ModalModeGuard");
    const auto modalGuard = code.Containing(modalGuards.front());
    const auto modalVtable = code.Relative(modalGuards.front() + 13, 3, 7);
    if (!modalGuard.begin || !modalVtable)
        return fail("ModalModeGuard");
    uint32_t endModal = 0;
    for (const auto at : code.Find("48 8B D9 48 8D 05 ?? ?? ?? ?? 48 89 01 E8 ?? ?? ?? ?? 90 EB ??"))
    {
        if (code.Relative(at + 3, 3, 7) != modalVtable)
            continue;
        if (endModal)
            return fail("ModalModeCleanup");
        endModal = code.Containing(at).begin;
    }
    if (!endModal)
        return fail("ModalModeCleanup");
    const auto sizedDeletes = code.Find("E8 ?? ?? ?? ?? F6 C3 01 74 ?? BA 60 00 00 00 48 8B CF");
    if (std::count_if(sizedDeletes.begin(), sizedDeletes.end(), [&](uint32_t at) {
            return code.Call(at) == endModal;
        }) != 1)
        return fail("ModalModeSize");

    const auto modalCalls = code.Find("33 D2 44 8D 42 04 48 8D 8D ?? ?? ?? ?? E8 ?? ?? ?? ?? "
        "90 0F 28 44 24 40");
    if (std::none_of(modalCalls.begin(), modalCalls.end(), [&](uint32_t at) {
            if (code.Call(at + 13) != modalGuard.begin)
                return false;
            const auto caller = code.Containing(at);
            const auto toolCalls = code.Find("41 B8 01 00 00 00 48 8D 55 ?? 48 8D 4D ?? E8 ?? ?? ?? ?? 90",
                at + 18, (std::min)(caller.end, at + 100));
            return std::any_of(toolCalls.begin(), toolCalls.end(), [&](uint32_t toolCall) {
                return code.Call(toolCall + 14) == toolGuard.begin;
            });
        }))
        return fail("ModalCanvasMode");

    entries["CustomAdjustment_DialogGetToolManager"] = getTools;
    entries["CustomAdjustment_DialogBeginCanvas"] = beginCanvas;
    entries["CustomAdjustment_DialogEndCanvas"] = endCanvas;
    entries["CustomAdjustment_DialogBeginModal"] = modalGuard.begin;
    entries["CustomAdjustment_DialogEndModal"] = endModal;
    log << "CustomAdjustment: verified native dialog canvas navigation and modal state\n";
    return true;
}

bool LocateNativeMenu(const uint8_t* image, uint32_t imageSize, uint32_t textRva, uint32_t textSize,
    uint32_t unwindRva, uint32_t unwindSize, Json::Value& entries, std::ostream& log)
{
    const auto fail = [&](const char* feature) {
        log << "ERROR: CustomAdjustment_Menu" << feature << " missing, ambiguous or incompatible\n";
        return false;
    };
    if (!image || textRva >= imageSize || textSize > imageSize - textRva || unwindRva >= imageSize ||
        !unwindSize || unwindSize > imageSize - unwindRva || unwindSize % sizeof(NativeCode::FunctionRange))
        return fail("Image");
    const auto* functions = reinterpret_cast<const NativeCode::FunctionRange*>(image + unwindRva);
    const NativeCode::Code code(image, imageSize, textRva, textSize, functions,
        unwindSize / sizeof(NativeCode::FunctionRange));
    const auto unique = [&](std::string_view pattern) {
        const auto matches = code.Find(pattern);
        return matches.size() == 1 ? matches.front() : 0;
    };

    const auto submenu = unique("48 8B 81 B0 00 00 00 48 89 02 4C 8B 81 B8 00 00 00 4C 89 42 08 4D 85 C0 74 "
        "0B B8 01 00 00 00 F0 41 0F C1 40 08 48 8B C2 C3");
    const auto count = unique("48 8B 41 10 48 2B 41 08 48 C1 F8 04 C3");
    const auto readItem = unique("4D 63 C8 49 C1 E1 04 4C 03 49 08 49 8B 01 48 89 02 49 8B 49 08 48 89 4A 08");
    if (!submenu || !count || !readItem)
        return fail("Read");

    const auto create = unique("48 8B 03 44 89 70 08 48 8B 0B 48 83 C1 10 48 8B D5 E8 ?? ?? ?? ?? 48 8B 0B "
        "48 83 C1 48 8B D6 E8 ?? ?? ?? ?? 48 8B C3");
    const auto makeItem = code.Containing(create);
    if (!makeItem.begin || !code.Match(makeItem.begin,
            "48 89 5C 24 10 48 89 6C 24 18 48 89 4C 24 08 56 57 41 56 48 83 EC 30 "
            "41 8B F1 49 8B E8 44 8B F2 48 8B D9 33 FF 89 7C 24 20 B9 C0 00 00 00 "
            "E8 ?? ?? ?? ?? 48 89 44 24 28 48 85 C0 74 0B 48 8B C8 E8"))
        return fail("MakeItem");
    const auto itemCtor = code.Call(makeItem.begin + 64);
    if (!code.Match(itemCtor,
            "48 89 4C 24 08 53 48 83 EC 20 48 8B D9 48 8D 05 ?? ?? ?? ?? 48 89 01 48 83 C1 10 E8"))
        return fail("ItemLayout");
    const auto itemVtable = code.Relative(itemCtor + 13, 3, 7);

    // The filter plug-in menu builds native items and appends them to PWMenu.
    NativeCode::FunctionRange pluginMenu{};
    for (auto at : code.Find("33 D2 48 8D 4C 24 70 E8 ?? ?? ?? ?? 90 48 8B CF E8 ?? ?? ?? ?? "
             "0F B7 D0 48 8B 4C 24 70 E8"))
    {
        if (code.Call(at + 7) != makeItem.begin)
            continue;
        if (pluginMenu.begin)
            return fail("PluginMenuBuilder");
        pluginMenu = code.Containing(at);
    }
    if (!pluginMenu.begin)
        return fail("PluginMenuBuilder");
    const auto calls = code.Find("E8 ?? ?? ?? ??", pluginMenu.begin, pluginMenu.end);
    const auto callsTarget = [&](uint32_t target) {
        return std::any_of(calls.begin(), calls.end(), [&](auto at) { return code.Call(at) == target; });
    };
    if (!callsTarget(count) || !callsTarget(readItem))
        return fail("MenuLayout");

    uint32_t menuVtable = 0;
    for (auto ctor : code.Find("48 8D 05 ?? ?? ?? ?? 48 89 01 33 C0 48 89 41 08 48 89 41 10 48 89 41 18 48 "
        "8B C1 C3"))
    {
        if (!callsTarget(ctor))
            continue;
        if (menuVtable)
            return fail("MenuLayout");
        menuVtable = code.Relative(ctor, 3, 7);
    }

    uint32_t append = 0;
    for (auto at : code.Find("48 89 5C 24 08 48 89 54 24 10 57 48 83 EC 20 "
             "48 8B DA 48 83 C1 08 4C 8B 49 08 4C 3B 49 10 74 24 "
             "48 8B 02 49 89 01 48 8B 52 08 49 89 51 08 48 85 D2 74 0A B8 01 00 00 00 "
             "F0 0F C1 42 08 48 83 41 08 10 EB 0C 4C 8B C3 49 8B D1 E8 ?? ?? ?? ?? "
             "90 48 8B 5B 08 48 85 DB 74 2D"))
    {
        if (!callsTarget(at))
            continue;
        if (append)
            return fail("Append");
        append = at;
    }
    if (!itemVtable || !menuVtable || !append)
        return fail("Append");

    const auto select = code.Find("49 8B CF E8 ?? ?? ?? ?? 8B D0 44 8B C3 48 8B CE FF 97 B8 00 00 00");
    const auto getCommand = select.empty() ? 0 : code.Call(select.front() + 3);
    if (select.size() != 3 || !code.Match(getCommand, "48 83 C1 48 E9 ?? ?? ?? ??") ||
        std::any_of(select.begin(), select.end(), [&](auto at) { return code.Call(at + 3) != getCommand; }))
        return fail("Selection");
    const auto sendCommand = unique("40 55 53 56 57 41 54 41 56 41 57 48 8D AC 24 70 FF FF FF 48 81 EC 90 01 00 "
        "00 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 85 80 00 00 00 45 8B E0 44 8B FA 4C "
        "8B F1 85 D2");
    if (!sendCommand)
        return fail("SendCommand");

    const auto getEventCommand = code.Relative(getCommand + 4, 1, 5);
    if (!code.Match(getEventCommand,
            "48 89 5C 24 10 57 48 83 EC 30 48 8B 41 08 48 8B D9 48 85 C0 74 ?? 8B 49 18"))
        return fail("QueryCommand");
    std::vector<uint32_t> statusReturns;
    const auto findStatusQueries = [&](std::string_view pattern, uint32_t setterOffset) {
        for (auto at : code.Find(pattern))
            if (code.Call(at) == getCommand && code.Match(code.Call(at + setterOffset), "89 51 08 C3"))
                statusReturns.push_back(at + 5);
    };
    findStatusQueries("E8 ?? ?? ?? ?? 8B D0 48 8D 4D ?? E8 ?? ?? ?? ?? 48 8B 4C 24 ?? E8 ?? ?? ?? ?? 8B D0", 11);
    findStatusQueries("E8 ?? ?? ?? ?? 8B D0 48 8D 8D ?? ?? 00 00 E8 ?? ?? ?? ?? 48 8B 4D ?? E8 ?? ?? ?? ?? 8B D0", 14);
    findStatusQueries("E8 ?? ?? ?? ?? 8B D0 48 8D 8D ?? ?? 00 00 E8 ?? ?? ?? ?? 48 8B 4C 24 ?? E8 ?? ?? ?? ?? 8B D0", 14);
    if (statusReturns.size() != 3)
        return fail("StatusQueries");

    const auto stringSet = unique("48 8B FA 48 C7 C3 FF FF FF FF 48 FF C3 80 3C 1A 00 75 F7 8B D3 E8 ?? ?? ?? "
        "?? 45 33 C9 4C 8B C0");
    const auto resize = stringSet ? code.Call(stringSet + 21) : 0;
    const auto resizeBody = code.Containing(resize);
    if (!resizeBody.begin || resizeBody.begin != resize ||
        !code.Match(resize, "48 89 5C 24 18 55 56 57 41 56 41 57 48 83 EC 60 4C 63 F2 48 8B F9") ||
        code.Find("44 89 77 18 33 D2 66 42 89 14 70", resizeBody.begin, resizeBody.end).size() != 1)
        return fail("Label");

    // The native command order identifies the submenu across translations.
    constexpr std::array<std::string_view, NativeCorrectionCommandCount> commandNames{
        "layeradjustmentbrightnessandcontrastnew", "layeradjustmenthsvnew", "layeradjustmentposterizenew",
        "layeradjustmentinvertnew", "layeradjustmentlevelnew", "layeradjustmenttonecurvenew",
        "layeradjustmentcolorbalancenew", "layeradjustmentbinarizationnew", "layeradjustmentgradientmapnew"};
    std::array<uint32_t, NativeCorrectionCommandCount> tokens{};
    for (auto at : code.Find("41 B9 01 00 00 00 4C 8D 05 ?? ?? ?? ?? 48 8D 15 ?? ?? ?? ?? "
             "48 8D 0D ?? ?? ?? ?? E9 ?? ?? ?? ??"))
    {
        const auto name = code.Relative(at + 6, 3, 7);
        if (!name)
            continue;
        for (size_t i = 0; i < tokens.size(); ++i)
        {
            const auto text = commandNames[i];
            if (text.size() >= imageSize - name || image[name + text.size()] ||
                memcmp(image + name, text.data(), text.size()) != 0)
                continue;
            if (tokens[i])
                return fail("CommandTokens");
            tokens[i] = code.Relative(at + 20, 3, 7);
        }
    }
    if (tokens[3] != entries["CustomAdjustment_CommandToken"].asUInt() ||
        std::any_of(tokens.begin(), tokens.end(), [](auto token) { return !token; }))
        return fail("CommandTokens");

    entries["CustomAdjustment_MenuGetSubmenu"] = submenu;
    entries["CustomAdjustment_MenuItemCount"] = count;
    entries["CustomAdjustment_MenuGetItem"] = readItem;
    entries["CustomAdjustment_MenuGetCommand"] = getCommand;
    entries["CustomAdjustment_MenuGetEventCommand"] = getEventCommand;
    entries["CustomAdjustment_MenuSendCommand"] = sendCommand;
    entries["CustomAdjustment_MenuMakeItem"] = makeItem.begin;
    entries["CustomAdjustment_MenuAppendItem"] = append;
    entries["CustomAdjustment_MenuResizeLabel"] = resize;
    entries["CustomAdjustment_MenuVtable"] = menuVtable;
    entries["CustomAdjustment_MenuItemVtable"] = itemVtable;
    for (size_t i = 0; i < tokens.size(); ++i)
        entries["CustomAdjustment_MenuToken_" + std::to_string(i)] = tokens[i];
    for (size_t i = 0; i < statusReturns.size(); ++i)
        entries["CustomAdjustment_MenuStatusReturn_" + std::to_string(i)] = statusReturns[i];
    log << "CustomAdjustment: verified native correction layer menu and command selection\n";
    return true;
}

bool LocateCommandCatalog(const uint8_t* image, uint32_t imageSize, uint32_t textRva, uint32_t textSize,
    uint32_t unwindRva, uint32_t unwindSize, Json::Value& entries, std::ostream& log)
{
    const auto fail = [&](const char* feature) {
        log << "ERROR: CustomAdjustment_Catalog" << feature << " missing, ambiguous or incompatible\n";
        return false;
    };
    if (!image || textRva >= imageSize || textSize > imageSize - textRva || unwindRva >= imageSize ||
        !unwindSize || unwindSize > imageSize - unwindRva || unwindSize % sizeof(NativeCode::FunctionRange))
        return fail("Image");
    const NativeCode::Code code(image, imageSize, textRva, textSize,
        reinterpret_cast<const NativeCode::FunctionRange*>(image + unwindRva),
        unwindSize / sizeof(NativeCode::FunctionRange));

    const auto reader = code.Find("48 8B E9 48 8B 3D ?? ?? ?? ?? 48 85 FF 0F 84 ?? ?? ?? ?? "
        "0F 57 C0 F3 0F 7F 44 24 30 48 C7 44 24 40 00 00 00 00 48 8B 37 48 8B 7F 08 48 3B F7");
    const auto settings = code.Find("48 8B 49 50 48 85 C9 74 05 E8 ?? ?? ?? ?? 48 8B 4B 60 BE 01 00 00 00 "
        "48 85 C9 74 07 8B D6 E8 ?? ?? ?? ??");
    if (reader.size() != 1 || settings.size() != 1)
        return fail("Read");
    const auto readBody = code.Containing(reader.front());
    const auto root = code.Relative(reader.front() + 3, 3, 7);
    if (!root || !readBody.begin || code.Call(settings.front() + 9) != readBody.begin ||
        !code.Match(readBody.begin, "48 89 5C 24 08 48 89 6C 24 18 48 89 74 24 20 57 48 83 EC 50"))
        return fail("Read");
    const auto menuCommands = code.Find("85 D2 75 0B 48 8D 4D E0 E8 ?? ?? ?? ?? EB 6C 3B D3 75 0B");
    if (std::count_if(menuCommands.begin(), menuCommands.end(), [&](auto at) {
            return code.Call(at + 8) == readBody.begin;
        }) != 1)
        return fail("CommandBarRead");

    const uint32_t lookup = entries["CustomAdjustment_CommandLookup"].asUInt();
    const uint32_t gradient = entries["CustomAdjustment_MenuToken_8"].asUInt();
    uint32_t wrapper = 0;
    uint32_t gradientReturn = 0;
    NativeCode::FunctionRange builder{};
    for (auto at : code.Find("48 8D 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 90 48 89 7C 24 30 "
             "C7 44 24 28 ?? 00 0D 00 89 44 24 20 45 33 C9 41 8D ?? 01 "
             "45 8D ?? 02 48 8B CB E8 ?? ?? ?? ??"))
    {
        if (code.Relative(at, 3, 7) != gradient || code.Call(at + 7) != lookup ||
            (!code.Match(at + 33, "41 8D 56 01 45 8D 46 02") &&
                !code.Match(at + 33, "41 8D 51 01 45 8D 41 02")))
            continue;
        if (wrapper)
            return fail("GradientRow");
        wrapper = code.Call(at + 44);
        gradientReturn = at + 49;
        builder = code.Containing(at);
    }
    if (!wrapper || !builder.begin || !code.Match(wrapper, "40 53 55 56 57 41 56 48 81 EC 80 00 00 00"))
        return fail("GradientRow");

    bool initBuildsCatalog = false;
    for (auto at : code.Find("48 8B 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 45 33 C0 48 8B D5 48 8B 0D ?? ?? ?? ?? E8"))
    {
        if (code.Call(at + 7) != builder.begin || code.Relative(at, 3, 7) != root)
            continue;
        const auto initializer = code.Containing(at);
        if (!initializer.begin || initBuildsCatalog)
            return fail("Initialize");
        const auto guards = code.Find("48 83 3D ?? ?? ?? ?? 00 0F 85 ?? ?? ?? ?? B9 B8 00 00 00",
            initializer.begin, initializer.end);
        if (guards.size() != 1 || code.Relative(guards.front(), 3, 8) != root)
            return fail("Initialize");
        initBuildsCatalog = true;
    }
    if (!initBuildsCatalog)
        return fail("Initialize");

    const auto wrapperBody = code.Containing(wrapper);
    const auto calls = code.Find("48 8D 4C 24 48 E8 ?? ?? ?? ?? 90", wrapper, wrapperBody.end);
    const auto appendSite = code.Find("89 44 24 20 45 8B CE 44 8B C5 8B D6 48 8B CF E8 ?? ?? ?? ?? 90",
        wrapper, wrapperBody.end);
    if (calls.size() != 2 || appendSite.size() != 1 || calls[0] >= appendSite.front() || calls[1] <= appendSite.front())
        return fail("AppendCalls");
    const auto stringCtor = code.Call(calls[0] + 5);
    const auto stringDtor = code.Call(calls[1] + 5);
    const auto append = code.Call(appendSite.front() + 15);
    if (!code.Match(stringCtor, "40 53 48 83 EC 20 48 8D 05 ?? ?? ?? ?? 48 8B D9 48 89 01 33 C0 "
            "48 89 41 08 48 89 41 10 E8") ||
        !code.Match(stringDtor, "40 53 48 83 EC 20 48 8D 05 ?? ?? ?? ?? 48 89 01 48 8B 59 10") ||
        code.Relative(stringCtor + 6, 3, 7) != code.Relative(stringDtor + 6, 3, 7) ||
        !code.Match(append, "48 89 5C 24 20 44 89 44 24 18 89 54 24 10 55 56 57 41 54 41 55 41 56 41 57 "
            "48 8B EC 48 81 EC 80 00 00 00 45 8B E9 4C 8B F9 48 8B 71 08 48 2B 31 48 C1 FE 05 33 D2 "
            "44 8D 62 01 44 8B 75 60 48 8B 05 ?? ?? ?? ??") || code.Relative(append + 62, 3, 7) != root)
        return fail("AppendABI");

    entries["CustomAdjustment_CatalogRead"] = readBody.begin;
    entries["CustomAdjustment_CatalogRoot"] = root;
    entries["CustomAdjustment_CatalogWrapAppend"] = wrapper;
    entries["CustomAdjustment_CatalogGradientReturn"] = gradientReturn;
    entries["CustomAdjustment_CatalogAppend"] = append;
    entries["CustomAdjustment_CatalogLabelConstruct"] = stringCtor;
    entries["CustomAdjustment_CatalogLabelDestroy"] = stringDtor;
    log << "CustomAdjustment: verified shared menu command catalog for shortcut and command bar settings\n";
    return true;
}

bool LocateCommandBindings(const uint8_t* image, uint32_t imageSize, uint32_t textRva, uint32_t textSize,
    uint32_t unwindRva, uint32_t unwindSize, Json::Value& entries, std::ostream& log)
{
    const auto fail = [&](const char* feature) {
        log << "ERROR: CustomAdjustment_Binding" << feature << " missing, ambiguous or incompatible\n";
        return false;
    };
    if (!image || textRva >= imageSize || textSize > imageSize - textRva || unwindRva >= imageSize ||
        !unwindSize || unwindSize > imageSize - unwindRva || unwindSize % sizeof(NativeCode::FunctionRange))
        return fail("Image");
    const NativeCode::Code code(image, imageSize, textRva, textSize,
        reinterpret_cast<const NativeCode::FunctionRange*>(image + unwindRva),
        unwindSize / sizeof(NativeCode::FunctionRange));
    const auto invert = entries["CustomAdjustment_CommandToken"].asUInt();
    const auto lookup = entries["CustomAdjustment_CommandLookup"].asUInt();
    uint32_t commandNamespace = 0;
    for (const auto at : code.Find("41 B9 01 00 00 00 4C 8D 05 ?? ?? ?? ?? 48 8D 15 ?? ?? ?? ?? "
             "48 8D 0D ?? ?? ?? ?? E9 ?? ?? ?? ??"))
    {
        if (code.Relative(at + 20, 3, 7) != invert)
            continue;
        if (commandNamespace)
            return fail("Namespace");
        commandNamespace = code.Relative(at + 13, 3, 7);
    }
    constexpr char advancedCommandNamespace[] = "742DEA58-ED6B-4402-BC11-20DFC6D08040";
    if (!commandNamespace || sizeof(advancedCommandNamespace) > imageSize - commandNamespace ||
        memcmp(image + commandNamespace, advancedCommandNamespace, sizeof(advancedCommandNamespace)) != 0)
        return fail("Namespace");
    const auto namespaceChecks = code.Find("48 8D 15 ?? ?? ?? ?? 48 8D 4C 24 20 E8 ?? ?? ?? ?? 90 "
        "45 33 C0 48 8D 54 24 20 48 8D 4C 24 50 E8 ?? ?? ?? ?? 8B D8");
    if (std::count_if(namespaceChecks.begin(), namespaceChecks.end(), [&](auto at) {
            return code.Relative(at, 3, 7) == commandNamespace;
        }) != 1)
        return fail("ShortcutSerialization");

    const auto dispatch = entries["CustomAdjustment_CommandDispatch"].asUInt();
    if (!code.Match(dispatch, "33 D2 E9 ?? ?? ?? ??"))
        return fail("DispatchWrapper");
    const auto dispatchWrapper = code.Relative(dispatch + 2, 1, 5);
    if (!code.Match(dispatchWrapper, "48 83 EC 28 48 8B 05 ?? ?? ?? ?? 48 85 C0 74 ?? 83 3D ?? ?? ?? ?? 00 "
            "7F ?? 85 C9 74 ?? 45 33 C9 44 8B C2 8B D1 48 8B C8 E8"))
        return fail("DispatchWrapper");
    const auto dispatchCore = code.Call(dispatchWrapper + 40);
    if (!code.Match(dispatchCore, "40 55 53 56 57 41 56 48 8D 6C 24 90 48 81 EC 70 01 00 00 "
            "48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 45 60 41 8B F1 45 8B F0 8B FA 48 8B D9"))
        return fail("DispatchABI");
    const auto shortcutCalls = code.Find("8B 53 FC 8B 4B F8 E8 ?? ?? ?? ?? 48 8B 55 E8 85 C0 74");
    if (std::count_if(shortcutCalls.begin(), shortcutCalls.end(), [&](auto at) {
            const auto target = code.Call(at + 6);
            return code.Match(target, "48 83 EC 28 48 8B 05 ?? ?? ?? ?? 48 85 C0 74 ?? 83 3D ?? ?? ?? ?? 00 "
                       "7F ?? 85 C9 74 ?? 41 B9 01 00 00 00 44 8B C2 8B D1 48 8B C8 E8") &&
                code.Call(target + 43) == dispatchCore;
        }) != 1)
        return fail("ShortcutDispatch");

    uint32_t iconLookup = 0;
    NativeCode::FunctionRange buttonFactory{};
    for (auto at : code.Find("8B 85 F0 00 00 00 85 C0 0F 84 ?? ?? ?? ?? 8B C8 E8 ?? ?? ?? ?? 44 8B ??"))
    {
        if (!code.Match(at + 21, "44 8B F0 0F 28 44 24 20") &&
            !code.Match(at + 21, "44 8B F8 48 8B 4C 24 30 48 89 4D 90 48 89 5D 98"))
            continue;
        const auto target = code.Call(at + 16);
        if (!code.Match(target, "48 83 EC 58 4C 8B 0D ?? ?? ?? ?? 0F 57 C0 F3 0F 7F 44 24 30 "
                "4D 85 C9 0F 84 ?? ?? ?? ?? 4D 8B 01"))
            continue;
        if (iconLookup)
            return fail("ToolbarIconLookup");
        iconLookup = target;
        buttonFactory = code.Containing(at);
    }
    if (!iconLookup)
        return fail("ToolbarIconLookup");
    const auto iconRoot = code.Relative(iconLookup + 4, 3, 7);
    const auto savedButtonCalls = code.Find("44 8B 44 24 7C 8B 95 ?? 01 00 00 48 8D 4D C0 E8 ?? ?? ?? ?? E9");
    const bool toolbarRestoreReady = buttonFactory.begin &&
        std::count_if(savedButtonCalls.begin(), savedButtonCalls.end(), [&](auto at) {
            return code.Call(at + 15) == buttonFactory.begin &&
                (code.Match(at + 7, "00 01 00 00") || code.Match(at + 7, "20 01 00 00"));
        }) == 1 &&
        code.Match(buttonFactory.begin, "48 89 5C 24 10 44 89 44 24 18 48 89 4C 24 08 55 56 57 41 54 41 55 "
            "41 56 41 57 48 8D AC 24 60 FF FF FF 48 81 EC A0 01 00 00");
    uint32_t toolbarMetadata = 0;
    for (auto at : code.Find("48 83 EC 28 4C 8B 05 ?? ?? ?? ?? 4C 8B D2 4D 85 C0 0F 84 ?? ?? ?? ?? 4D 8B 08"))
    {
        if (code.Relative(at + 4, 3, 7) != iconRoot)
            continue;
        if (toolbarMetadata)
            return fail("ToolbarMetadataRead");
        toolbarMetadata = at;
    }
    const auto metadataBody = code.Containing(toolbarMetadata);
    if (!metadataBody.begin || code.Find("48 83 C2 28 49 8B CA E8 ?? ?? ?? ?? B8 01 00 00 00",
            metadataBody.begin, metadataBody.end).size() != 1)
        return fail("ToolbarMetadataRead");

    uint32_t iconWrapper = 0;
    uint32_t nativeIconReturn = 0;
    for (auto at : code.Find("48 8D 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 8B D0 44 89 ?? 24 28 C7 44 24 20 ?? ?? ?? ?? "
             "41 B9 43 01 00 00"))
    {
        if (code.Relative(at, 3, 7) != invert || code.Call(at + 7) != lookup)
            continue;
        const auto call = code.Match(at + 33, "44 8D 46 01 48 8B CB E8") ? at + 40 :
            (code.Match(at + 33, "41 B8 01 00 00 00 48 8B CB E8") ? at + 42 : 0);
        if (!call)
            continue;
        if (iconWrapper)
            return fail("ToolbarNativeIcon");
        iconWrapper = code.Call(call);
        nativeIconReturn = call + 5;
    }
    if (!iconRoot || !code.Match(iconWrapper, "48 89 5C 24 18 55 56 57 48 81 EC 90 00 00 00"))
        return fail("ToolbarNativeIcon");
    const auto wrapperBody = code.Containing(iconWrapper);
    const auto iconAddCalls = code.Find("89 5C 24 28 89 7C 24 20 45 33 C9 44 8B C5 8B D5 48 8B CE E8 ?? ?? ?? ??",
        iconWrapper, wrapperBody.end);
    const auto iconAdd = iconAddCalls.size() == 1 ? code.Call(iconAddCalls.front() + 19) : 0;
    if (!code.Match(iconAdd, "48 89 5C 24 10 44 89 4C 24 20 44 89 44 24 18 48 89 4C 24 08 "
            "55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 F9 48 81 EC E0 00 00 00"))
        return fail("ToolbarRegisterABI");

    entries["CustomAdjustment_BindingNamespace"] = commandNamespace;
    entries["CustomAdjustment_BindingDispatch"] = dispatchCore;
    entries["CustomAdjustment_BindingIconRoot"] = iconRoot;
    entries["CustomAdjustment_BindingToolbarMetadata"] = toolbarMetadata;
    entries["CustomAdjustment_BindingIconWrapper"] = iconWrapper;
    entries["CustomAdjustment_BindingNativeIconReturn"] = nativeIconReturn;
    entries["CustomAdjustment_BindingIconAdd"] = iconAdd;
    log << "CustomAdjustment: verified persistent command namespace, shortcut execution and command bar icon registry\n";

    const auto statusCalls = code.Find("8B 55 48 48 8B 0F E8 ?? ?? ?? ?? 48 8B 0F 48 89 4C 24 60 48 8B 57 08 "
        "48 89 54 24 68");
    const uint32_t statusSite = statusCalls.size() == 1 ? statusCalls.front() : 0;
    const uint32_t statusQuery = statusSite ? code.Call(statusSite + 6) : 0;
    const auto toolbarUpdate = code.Containing(statusSite);
    if (!toolbarUpdate.begin || statusSite - toolbarUpdate.begin < 0x87 ||
        !code.Match(statusSite - 0x87, "48 8D 05 ?? ?? ?? ?? 48 89 44 24 30 45 33 FF") ||
        !code.Match(statusSite + 0x3a, "48 8D 05 ?? ?? ?? ?? 48 89 44 24 30 44 89 7C 24 38") ||
        !code.Match(statusQuery, "48 89 5C 24 18 57 48 81 EC D0 00 00 00 48 8B 05 ?? ?? ?? ?? 48 33 C4") ||
        !code.Match(statusQuery + 0x45, "8B D3 48 8D 4C 24 50 E8") ||
        !code.Match(code.Call(statusQuery + 0x4c), "89 51 08 C3"))
    {
        log << "ERROR: CustomAdjustment_BindingToolbarStatus missing, ambiguous or incompatible\n";
        return false;
    }
    const auto readCommand = code.Relative(statusSite - 0x87, 3, 7);
    const auto setButtonState = code.Relative(statusSite + 0x3a, 3, 7);
    if (!code.Match(readCommand, "48 81 C1 A0 02 00 00 E9 ?? ?? ?? ??") ||
        code.Relative(readCommand + 7, 1, 5) != entries["CustomAdjustment_MenuGetEventCommand"].asUInt() ||
        !code.Match(setButtonState, "83 B9 C0 02 00 00 00 74 ?? 85 D2 75 ?? 89 91 C0 02 00 00"))
    {
        log << "ERROR: CustomAdjustment_BindingToolbarButtonLayout missing, ambiguous or incompatible\n";
        return false;
    }
    entries["CustomAdjustment_BindingToolbarStatus"] = statusQuery;
    entries["CustomAdjustment_BindingToolbarStatusReturn"] = statusSite + 11;
    log << "CustomAdjustment: verified command bar button availability query\n";
    if (!toolbarRestoreReady)
    {
        log << "ERROR: CustomAdjustment_BindingToolbarRestore missing, ambiguous or incompatible\n";
        return false;
    }
    entries["CustomAdjustment_BindingToolbarRestore"] = buttonFactory.begin;
    return true;
}
}
