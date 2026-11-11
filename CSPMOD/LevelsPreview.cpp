#include "pch.h"
#include "LevelsPreview.h"
#include "AddressTable.h"
#include "CSPMOD.h"
#include "NativeCode.h"

#include <chrono>
#include <atomic>
#include <limits>

namespace
{
constexpr LONG originalDebounceMs = 200;
constexpr auto previewInterval = std::chrono::milliseconds(33);
using EventHandler = unsigned(*)(void*, const void*);
using EventIdFunc = unsigned(*)(const void*);
EventHandler originalPixelEvent = nullptr;
EventHandler originalLayerEvent = nullptr;
EventIdFunc getEventId = nullptr;
const void* layerChangedToken = nullptr;
const void* timerToken = nullptr;
std::atomic<bool> installed{false};
std::atomic<bool> previewEnabled{false};

unsigned ReadEventId(const void* event)
{
    unsigned type;
    memcpy(&type, static_cast<const uint8_t*>(event) + 8, sizeof(type));
    return type;
}

unsigned OnPixelEvent(void* dialog, const void* event)
{
    if (!previewEnabled || !event || ReadEventId(event) != getEventId(timerToken))
        return originalPixelEvent(dialog, event);

    int pending;
    memcpy(&pending, static_cast<const uint8_t*>(dialog) + 0x190, sizeof(pending));
    if (!pending)
        return originalPixelEvent(dialog, event);

    using Clock = std::chrono::steady_clock;
    static thread_local void* lastDialog = nullptr;
    static thread_local Clock::time_point lastPreview;
    const auto now = Clock::now();
    if (dialog == lastDialog && now - lastPreview < previewInterval)
    {
        // Only the native handler cancels this recurring timer. Leaving it armed
        // lets the next tick publish the latest value, even after input stops.
        return 1;
    }
    lastDialog = dialog;
    lastPreview = now;

    // The native timer adds 200 ms to the last input timestamp. Bypass that
    // wait for this invocation while retaining any new timestamp from a retry.
    auto* inputTimestamp = static_cast<uint8_t*>(dialog) + 0x20;
    int64_t previous;
    memcpy(&previous, inputTimestamp, sizeof(previous));
    constexpr int64_t advance = originalDebounceMs + 1;
    if (previous < (std::numeric_limits<int64_t>::min)() + advance)
        return originalPixelEvent(dialog, event);
    const auto eligible = previous - advance;
    memcpy(inputTimestamp, &eligible, sizeof(eligible));
    const unsigned result = originalPixelEvent(dialog, event);
    int64_t current;
    memcpy(&current, inputTimestamp, sizeof(current));
    if (current == eligible)
        memcpy(inputTimestamp, &previous, sizeof(previous));
    lastPreview = Clock::now();
    return result;
}

unsigned OnLayerEvent(void* dialog, const void* event)
{
    const unsigned result = originalLayerEvent(dialog, event);
    if (!previewEnabled || !result || !event)
        return result;

    if (ReadEventId(event) != getEventId(layerChangedToken) || !(GetAsyncKeyState(VK_LBUTTON) & 0x8000))
        return result;

    using Clock = std::chrono::steady_clock;
    static thread_local void* lastDialog = nullptr;
    static thread_local Clock::time_point lastPreview;
    const auto now = Clock::now();
    if (dialog == lastDialog && now - lastPreview < previewInterval)
        return result;
    lastDialog = dialog;
    lastPreview = now;

    // The native change event saves the latest parameters and arms the trailing
    // timer. Run the native timer branch at the same cadence as the curve preview.
    // This branch reads only the event ID and cancels the pending timer.
    struct TimerEvent
    {
        uintptr_t unused;
        unsigned type;
    };
    const TimerEvent timer{0, getEventId(timerToken)};
    originalLayerEvent(dialog, &timer);
    return result;
}
}

bool LevelsPreview::Locate(const uint8_t* image, uint32_t imageSize, uint32_t textRva, uint32_t textSize,
    uint32_t unwindRva, uint32_t unwindSize, uint32_t dataRva, uint32_t dataSize,
    Json::Value& table, std::ostream& log)
{
    using NativeCode::Code;
    using NativeCode::FunctionRange;
    const auto fail = [&log](const char* feature) {
        log << "ERROR: " << feature << " missing, ambiguous, or failed its call/layout checks.\n";
        return false;
    };
    if (!image || !textRva || textRva >= imageSize || !textSize || textSize > imageSize - textRva ||
        !unwindRva || unwindRva >= imageSize || unwindSize > imageSize - unwindRva ||
        unwindSize < sizeof(FunctionRange) || unwindSize % sizeof(FunctionRange) ||
        !dataRva || dataRva >= imageSize || dataSize > imageSize - dataRva)
        return fail("LevelsPreview PE sections");

    const auto inData = [dataRva, dataSize](uint32_t rva, size_t size) {
        return rva >= dataRva && rva - dataRva < dataSize && size <= dataSize - (rva - dataRva);
    };
    const auto hasMilliseconds = [&](uint32_t rva, LONG expected) {
        if (rva % alignof(LONG) || !inData(rva, sizeof(LONG)))
            return false;
        LONG value;
        memcpy(&value, image + rva, sizeof(value));
        return value == expected;
    };
    const Code code(image, imageSize, textRva, textSize,
        reinterpret_cast<const FunctionRange*>(image + unwindRva), unwindSize / sizeof(FunctionRange));

    // The correction layer receives a typed SAdjustmentLevels event and registers
    // itself as the timer callback using the same native function.
    const auto layerTimers = code.Find(
        "8B 15 ?? ?? ?? ?? 48 8D 4C 24 20 E8 ?? ?? ?? ?? 8B 9F 90 01 00 00 48 8D 05 ?? ?? ?? ??");
    if (layerTimers.size() != 1)
        return fail("LevelsPreview layer timer");
    const auto layerTimer = layerTimers.front();
    const auto layer = code.Containing(layerTimer);
    if (!layer.begin || code.Relative(layerTimer + 22, 3, 7) != layer.begin)
        return fail("LevelsPreview layer timer callback");
    const auto types = code.Find("48 8D 48 08 48 8D 15 ?? ?? ?? ?? E8 ?? ?? ?? ?? 85 C0 0F 85 ?? ?? ?? ?? 48 8B 0E",
        layer.begin, layerTimer);
    const auto type = types.size() == 1 ? code.Relative(types.front() + 4, 3, 7) : 0;
    constexpr char levelsType[] = ".?AUSAdjustmentLevels@Planeswalker@@";
    if (!inData(type, 8 + sizeof(levelsType)) || memcmp(image + type + 8, levelsType, sizeof(levelsType)) != 0)
        return fail("LevelsPreview SAdjustmentLevels event");
    const auto debounce = code.Relative(layerTimer, 2, 6);
    if (!hasMilliseconds(debounce, originalDebounceMs))
        return fail("LevelsPreview shared debounce value");

    // Curve preview has a similar gate but uses a separate delay. The Levels
    // callback must read the same delay as the correction layer.
    uint32_t pixelGate = 0;
    for (const auto gate : code.Find(
        "8B 0D ?? ?? ?? ?? 48 03 4B 20 48 3B C8 7D ?? 48 8D 4B 08 BA 40 E2 01 00 E8"))
    {
        if (code.Relative(gate, 2, 6) != debounce)
            continue;
        if (pixelGate)
            return fail("LevelsPreview pixel debounce gate");
        pixelGate = gate;
    }
    const auto pixel = code.Containing(pixelGate);
    if (!code.Match(pixel.begin, "48 89 5C 24 18 55 56 57 48 81 EC F0 02 00 00") ||
        code.Find("83 BB 90 01 00 00 00 74 ?? 48 8B CB E8 ?? ?? ?? ?? C7 83 90 01 00 00 00 00 00 00",
            pixelGate, pixel.end).size() != 1)
        return fail(PixelEvent);
    const auto waits = code.Find(
        "8B 0D ?? ?? ?? ?? 48 3B C1 7D ?? 48 8B 4B 30 E8 ?? ?? ?? ?? 85 C0 75 ?? B8 01 00 00 00 EB ?? 48 8B CB E8",
        pixelGate, pixel.end);
    const auto schedule = waits.size() == 1 ? code.Call(waits.front() + 34) : 0;
    if (!schedule || code.Containing(schedule).begin != schedule ||
        !code.Match(schedule, "40 53 48 83 EC 70 48 8B D9 E8 ?? ?? ?? ?? 48 89 43 20"))
        return fail("LevelsPreview pixel worker retry");

    const auto starts = code.Find("8B 15 ?? ?? ?? ?? 48 8D 4C 24 20 E8 ?? ?? ?? ?? 48 8D 05 ?? ?? ?? ?? 48 89 44 24 30",
        schedule, code.Containing(schedule).end);
    const auto milliseconds = code.Call(layerTimer + 11);
    if (starts.size() != 1 || code.Call(starts.front() + 11) != milliseconds ||
        code.Relative(starts.front() + 16, 3, 7) != pixel.begin ||
        code.Relative(starts.front(), 2, 6) != code.Relative(waits.front(), 2, 6) ||
        !code.Match(milliseconds, "48 69 C2 40 42 0F 00 48 89 41 08 C3"))
        return fail("LevelsPreview pixel timer registration");

    const auto changes = code.Find(
        "48 8B CA E8 ?? ?? ?? ?? 8B D8 48 8D 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 3B D8 0F 85 ?? ?? ?? ?? 48 8D 0D",
        layer.begin, layerTimer);
    if (changes.size() != 1 ||
        !code.Match(layer.begin, "48 89 5C 24 08 48 89 74 24 10 57 48 81 EC B0 01 00 00"))
        return fail(LayerEvent);
    const auto change = changes.front();
    const auto timerPath = code.Relative(change + 24, 2, 6);
    if (timerPath <= layerTimer || timerPath >= layer.end ||
        !code.Match(timerPath,
            "48 8B CE E8 ?? ?? ?? ?? 8B C8 E8 ?? ?? ?? ?? 85 C0 0F 84 ?? ?? ?? ?? 48 8D 8F 78 01 00 00"))
        return fail("LevelsPreview layer timer event path");
    const auto eventType = code.Call(change + 3);
    const auto lookupId = code.Call(change + 17);
    const auto isTimer = code.Call(timerPath + 10);
    if (eventType != code.Call(timerPath + 3) || !code.Match(eventType, "8B 41 08 C3") ||
        !lookupId || code.Containing(lookupId).begin != lookupId ||
        !code.Match(isTimer, "40 53 48 83 EC 20 8B D9 48 8D 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 33 D2 3B D8 0F 94 C2 8B C2") ||
        code.Call(isTimer + 15) != lookupId)
        return fail(EventId);
    const auto changeToken = code.Relative(change + 10, 3, 7);
    const auto tickToken = code.Relative(isTimer + 8, 3, 7);
    if (changeToken == tickToken || !inData(changeToken, 0x20) || !inData(tickToken, 0x20))
        return fail("LevelsPreview layer change/timer event tokens");

    const auto pixelTicks = code.Find("8B CD E8 ?? ?? ?? ?? 85 C0 74 ?? E8 ?? ?? ?? ?? 48 8B F8 8B 0D",
        pixel.begin, pixelGate + 2);
    if (pixelTicks.size() != 1 || code.Call(pixelTicks.front() + 2) != isTimer ||
        code.Call(pixelTicks.front() + 11) != code.Call(schedule + 9))
        return fail("LevelsPreview pixel timer event type");
    // Confirm that the scheduler records the tick time before invoking the
    // callback and discards its return value. The callback removes its own timer.
    const auto dispatches = code.Find(
        "48 8D 4B 58 48 8D 54 24 30 E8 ?? ?? ?? ?? 48 8D 4D ?? E8 ?? ?? ?? ?? 90 48 8D 55 ?? 48 8D 4D ?? E8 ?? ?? ?? ?? "
        "48 8B 43 20 48 85 C0 0F 84 ?? ?? ?? ?? 48 83 E0 FE 48 8D 4B 28 48 8D 55 ?? FF 50 08 90 48 8D 4D");
    if (dispatches.size() != 1 ||
        !code.Match(code.Call(dispatches.front() + 9), "48 8B 42 08 48 89 41 08 48 8B C1 C3"))
        return fail("LevelsPreview recurring native timer");

    table[PixelGate] = pixelGate;
    table[PixelEvent] = pixel.begin;
    table[LayerEvent] = layer.begin;
    table[EventId] = lookupId;
    table[LayerChangedToken] = changeToken;
    table[TimerToken] = tickToken;
    log << "Located " << PixelGate << " using the delay shared by both native preview paths.\n";
    log << "Located " << LayerEvent << " and verified its native change and timer events.\n";
    return true;
}

void LevelsPreview::Install()
{
    auto* pixelGate = static_cast<uint8_t*>(AddressTable::GetAddress(PixelGate));
    void* pixelReceiver = AddressTable::GetAddress(PixelEvent);
    void* receiver = AddressTable::GetAddress(LayerEvent);
    getEventId = reinterpret_cast<EventIdFunc>(AddressTable::GetAddress(EventId));
    layerChangedToken = AddressTable::GetAddress(LayerChangedToken);
    timerToken = AddressTable::GetAddress(TimerToken);
    if (!pixelGate || !pixelReceiver || !receiver || !getEventId || !layerChangedToken || !timerToken)
        return;
    if (pixelGate[0] != 0x8b || pixelGate[1] != 0x0d ||
        !CSPMOD::Hook(receiver, reinterpret_cast<void*>(OnLayerEvent),
            reinterpret_cast<void**>(&originalLayerEvent), CSPMOD::_HOOKREG_R11) ||
        !CSPMOD::Hook(pixelReceiver, reinterpret_cast<void*>(OnPixelEvent),
            reinterpret_cast<void**>(&originalPixelEvent), CSPMOD::_HOOKREG_R11))
    {
        SDL_Log("Levels live preview disabled: native preview path changed or hook failed.");
        return;
    }

    installed = true;
}

bool LevelsPreview::Available()
{
    return installed;
}

void LevelsPreview::SetEnabled(bool value)
{
    previewEnabled = installed && value;
}
