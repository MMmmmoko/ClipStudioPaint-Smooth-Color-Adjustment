#include "CustomAdjustmentLayer.h"

#include "AddressTable.h"
#include "CSPMOD.h"
#include "CspData.h"
#include "NativeCanvasCoordinates.h"
#include "duilib/duilib.h"

#include <CommCtrl.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <deque>
#include <intrin.h>
#include <iterator>
#include <memory>
#include <mutex>
#include <string>

#pragma comment(lib, "Comctl32.lib")

namespace CustomAdjustmentLayer
{
namespace
{
struct SharedInfo
{
    uint8_t* object;
    void* control;
};

void Retain(const SharedInfo& reference)
{
    if (reference.control)
        InterlockedIncrement(reinterpret_cast<LONG*>(static_cast<uint8_t*>(reference.control) + 8));
}

void Release(const SharedInfo& reference)
{
    if (!reference.control)
        return;
    auto* control = static_cast<uint8_t*>(reference.control);
    if (InterlockedDecrement(reinterpret_cast<LONG*>(control + 8)) != 0)
        return;
    using ReleaseFunction = void (*)(void*);
    const auto* vtable = *reinterpret_cast<ReleaseFunction**>(control);
    vtable[1](control);
    if (InterlockedDecrement(reinterpret_cast<LONG*>(control + 12)) == 0)
        vtable[2](control);
}

struct OwnedInfo
{
    SharedInfo value{};
    OwnedInfo() = default;
    OwnedInfo(const OwnedInfo&) = delete;
    OwnedInfo& operator=(const OwnedInfo&) = delete;
    ~OwnedInfo() { Release(value); }
};

using ManageCallable = void (*)(void*, void*, int);
template <typename Invoke>
struct alignas(8) CallableTable
{
    ManageCallable manage = nullptr;
    Invoke invoke;
};

// The host copies 32 bytes inline for callbacks whose table pointer has bit 0 set.
struct Callable
{
    uintptr_t table;
    uint32_t data[8]{};
};
static_assert(sizeof(Callable) == 40);

struct RowLayout
{
    int destinationStride;
    int unused;
    int sourceStride;
    int alphaStride;
    int width;
};

struct Creation
{
    const AdjustmentLayer& adjustment;
    bool created = false;
};

using Factory = SharedInfo* (*)(SharedInfo* output, uint32_t type, SharedInfo* source);
using Lookup = uint32_t (*)(const void* token);
using Dispatch = int (*)(uint32_t command);
using SetStreams = void (*)(void*, const Callable*, const Callable*);
using SetRender = void (*)(void*, const Callable*);
using RenderRows = int (*)(void*, const void*, void*, Callable*);
using GetFormat = int (*)(const void*);
using Read32 = uint32_t (*)(void*);
using Write32 = void (*)(void*, uint32_t);
using GetLayer = SharedInfo* (*)(void*, SharedInfo*);
using GetLayerProperty = void (*)(void*, int, SharedInfo*);
using CreateUndo = SharedInfo* (*)(SharedInfo*, SharedInfo*);
using SubmitUndo = int (*)(void*, SharedInfo*);
using NotifyChange = void (*)(SharedInfo*, SharedInfo*);
using QueryType = uint32_t (*)(SharedInfo*);
using GetToolManager = SharedInfo* (*)(SharedInfo*);
using BeginCanvasNavigation = void (*)(void*, int);
using EndCanvasNavigation = void (*)(void*, SharedInfo*);
using BeginModalMode = void* (*)(void*, int, int);
using EndModalMode = void (*)(void*);

constexpr uint32_t nativeInvert = 6;
constexpr UINT_PTR subclassId = UserStart;
constexpr UINT_PTR previewTimer = UserStart;
constexpr int filterLayerProperty = 700;
constexpr unsigned previewIntervalMs = 33;
Factory originalFactory = nullptr;
void* expectedVtable = nullptr;
void* createReturn = nullptr;
const void* commandToken = nullptr;
Lookup lookupCommand = nullptr;
Dispatch dispatchCommand = nullptr;
Dispatch dispatchUndo = nullptr;
const void* undoToken = nullptr;
SetStreams setStreams = nullptr;
SetRender setRender = nullptr;
RenderRows renderRows = nullptr;
GetFormat getFormat = nullptr;
Read32 read32 = nullptr;
Write32 write32 = nullptr;
GetLayer getLayer = nullptr;
GetLayerProperty getLayerProperty = nullptr;
CreateUndo createUndo = nullptr;
SubmitUndo submitUndo = nullptr;
NotifyChange notifyChange = nullptr;
const void* controllerRoot = nullptr;
QueryType originalQueryType = nullptr;
const void* paletteTypeReturn = nullptr;
const void* dialogTypeReturn = nullptr;
GetToolManager getToolManager = nullptr;
BeginCanvasNavigation beginCanvasNavigation = nullptr;
EndCanvasNavigation endCanvasNavigation = nullptr;
BeginModalMode beginModalMode = nullptr;
EndModalMode endModalMode = nullptr;
UINT createAdjustmentMessage = 0;
UINT editAdjustmentMessage = 0;
thread_local Creation* pendingCreation = nullptr;
thread_local bool dialogOpen = false;

using GetSubmenu = SharedInfo* (*)(const void*, SharedInfo*);
using MenuCount = size_t (*)(const void*);
using MenuGetItem = SharedInfo* (*)(const void*, SharedInfo*, int);
using MenuGetCommand = uint32_t (*)(const void*);
using MenuMakeItem = SharedInfo* (*)(SharedInfo*, uint32_t, const void*, uint32_t);
using MenuAppend = void (*)(void*, SharedInfo*);
using MenuResizeLabel = wchar_t* (*)(void*, int);
using MenuSendCommand = int (*)(void*, uint32_t, int);

GetSubmenu originalGetSubmenu = nullptr;
MenuSendCommand originalMenuSendCommand = nullptr;
MenuCount menuCount = nullptr;
MenuGetItem menuGetItem = nullptr;
MenuGetCommand menuGetCommand = nullptr;
MenuGetCommand originalGetMenuEventCommand = nullptr;
MenuMakeItem menuMakeItem = nullptr;
MenuAppend menuAppend = nullptr;
MenuResizeLabel menuResizeLabel = nullptr;
uint32_t layerNameOffset = 0;
const void* layerNameVtable = nullptr;
const void* menuVtable = nullptr;
const void* menuItemVtable = nullptr;
std::array<const void*, NativeCorrectionCommandCount> correctionMenuTokens{};
std::array<uint32_t, NativeCorrectionCommandCount> correctionMenuCommands{};
std::array<const void*, 3> menuStatusReturns{};

// The prefixed command name is independent of the serialized layer type.
struct MenuToken
{
    uint32_t kind = 1;
    const char* nameSpace;
    const char* name;
    uintptr_t once = 0;
    uint32_t id = 0;
};
static_assert(offsetof(MenuToken, once) == 0x18 && offsetof(MenuToken, id) == 0x20);
struct RegisteredAdjustment
{
    RegisteredAdjustment(const AdjustmentLayer& adjustment, bool supportsCreation, CreationGate gate)
        : adjustment(adjustment), supportsCreation(supportsCreation), gate(gate),
          token{1, "23EC37C1-9E46-4356-8ACB-4BB9DB41D96C", adjustment.commandName}
    {
    }

    bool CanCreate() const { return supportsCreation && (!gate || gate()); }

    const AdjustmentLayer& adjustment;
    const bool supportsCreation;
    const CreationGate gate;
    MenuToken token;
    std::atomic<uint32_t> command{0};
};

std::deque<RegisteredAdjustment> adjustments;

const RegisteredAdjustment* FindRegistration(uint32_t type)
{
    const auto entry = std::find_if(std::begin(adjustments), std::end(adjustments),
        [type](const auto& candidate) { return candidate.adjustment.type == type; });
    return entry == std::end(adjustments) ? nullptr : &*entry;
}

const AdjustmentLayer* FindLayer(uint32_t type)
{
    const auto* entry = FindRegistration(type);
    return entry ? &entry->adjustment : nullptr;
}

bool ValidateRegistry()
{
    for (auto entry = adjustments.cbegin(); entry != adjustments.cend(); ++entry)
    {
        const auto& adjustment = entry->adjustment;
        if (adjustment.type <= UserStart || !adjustment.commandName || !*adjustment.commandName ||
            std::any_of(adjustments.cbegin(), entry, [&](const auto& other) {
                return other.adjustment.type == adjustment.type ||
                    strcmp(other.adjustment.commandName, adjustment.commandName) == 0;
            }))
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Custom adjustment: invalid or duplicate type or command.");
            return false;
        }
    }
    return true;
}

struct CatalogRow
{
    int flags;
    int depth;
    int mode;
    uint32_t command;
    SharedInfo label;
};
static_assert(sizeof(CatalogRow) == 0x20);

struct CatalogRows
{
    CatalogRow* begin;
    CatalogRow* end;
    CatalogRow* capacity;
};

using CatalogRead = void (*)(CatalogRows*);
using CatalogWrapAppend = void (*)(CatalogRows*, int, int, int, uint32_t, uint32_t, SharedInfo*);
using CatalogAppend = void (*)(CatalogRows*, int, int, int, uint32_t, const void*);
using ConstructLabel = void* (*)(void*);
using DestroyLabel = void (*)(void*);

CatalogRead originalCatalogRead = nullptr;
CatalogWrapAppend originalCatalogWrapAppend = nullptr;
CatalogAppend catalogAppend = nullptr;
ConstructLabel constructCatalogLabel = nullptr;
DestroyLabel destroyCatalogLabel = nullptr;
CatalogRows* const* catalogRoot = nullptr;
const void* catalogGradientReturn = nullptr;

using DispatchBinding = int (*)(void*, uint32_t, int, int);
using QueryToolbarStatus = int (*)(void*, uint32_t);
using RestoreToolbarButton = void (*)(SharedInfo*, int, uint32_t, int, int, int, const void*, int, int);
struct ToolbarMetadata
{
    int depth;
    uint32_t icon;
    uint32_t option;
    SharedInfo label;
    uint32_t reserved;
    int flags;
    uint32_t command;
    uint32_t alternateCommand;

    ~ToolbarMetadata() { Release(label); }
};
static_assert(sizeof(ToolbarMetadata) == 0x30 && offsetof(ToolbarMetadata, label) == 0x10);

using LookupToolbarMetadata = int (*)(uint32_t, ToolbarMetadata*);
using WrapToolbarItem = void (*)(void*, uint32_t, int, uint32_t, uint32_t, int);
using AddToolbarItem = void (*)(void*, uint32_t, uint32_t, int, int, uint32_t, const void*, int);

DispatchBinding originalDispatchBinding = nullptr;
QueryToolbarStatus originalToolbarStatus = nullptr;
RestoreToolbarButton originalRestoreToolbarButton = nullptr;
const void* toolbarStatusReturn = nullptr;
WrapToolbarItem originalWrapToolbarItem = nullptr;
LookupToolbarMetadata lookupToolbarMetadata = nullptr;
AddToolbarItem addToolbarItem = nullptr;
void* const* toolbarRoot = nullptr;
const void* nativeIconReturn = nullptr;

struct SourceSample
{
    SourceSample(const void* renderIdentity, uint32_t width, uint32_t height)
        : renderIdentity(renderIdentity), width(width), height(height) {}

    void Request(int x, int y)
    {
        const std::lock_guard<std::mutex> lock(mutex);
        column = x;
        ready = false;
        value.reset();
        scanline.store(y, std::memory_order_release);
    }

    void Capture(int x, int y, const PixelRow& row)
    {
        if (y < 0 || y != scanline.load(std::memory_order_acquire))
            return;
        const std::lock_guard<std::mutex> lock(mutex);
        const int64_t offset = int64_t(column) - x;
        if (y != scanline.load(std::memory_order_relaxed) || offset < 0 || offset >= row.width)
            return;
        if (row.source && row.alpha && (row.sourceStride >= 3 || row.sourceStride <= -3) && row.alpha[offset * row.alphaStride])
        {
            const auto* pixel = row.source + offset * row.sourceStride;
            value = SourceColor{pixel[0], pixel[1], pixel[2]};
        }
        ready = true;
        scanline.store(-1, std::memory_order_relaxed);
    }

    bool Read(std::optional<SourceColor>& color)
    {
        const std::lock_guard<std::mutex> lock(mutex);
        if (!ready)
            return false;
        ready = false;
        color = value;
        return true;
    }

    void Cancel()
    {
        const std::lock_guard<std::mutex> lock(mutex);
        scanline.store(-1, std::memory_order_relaxed);
        ready = false;
    }

    const void* const renderIdentity;
    const uint32_t width, height;

private:
    std::mutex mutex;
    std::atomic<int> scanline{-1};
    int column = 0;
    bool ready = false;
    std::optional<SourceColor> value;
};

std::shared_ptr<SourceSample> activeSample;

struct RowContext
{
    const AdjustmentLayer* adjustment;
    const Parameters* parameters;
    void* source;
    SourceRowObserver* observer;
    SourceSample* sample;
    bool bypass;
    int x = 0, y = 0, row = 0;
    bool first = true;
};

// The host passes the destination auxiliary plane before the source color plane.
void RenderRow(const void* closure, const RowLayout* layout, uint8_t* destination,
    const void* /* destinationAuxiliary */, const uint8_t* source, const uint8_t* alpha)
{
    auto& context = **static_cast<RowContext* const*>(closure);
    const PixelRow row{destination, source, alpha, layout->destinationStride,
        layout->sourceStride, layout->alphaStride, layout->width};
    if (context.observer || context.sample)
    {
        // SelectTile records the current source origin. The native loop advances the planes between rows.
        int origin[2];
        memcpy(origin, static_cast<const uint8_t*>(context.source) + 0x40, sizeof(origin));
        if (context.first || context.x != origin[0] || context.y != origin[1])
        {
            context.x = origin[0];
            context.y = origin[1];
            context.row = 0;
            context.first = false;
        }
        const int y = context.y + context.row++;
        if (context.observer)
            context.observer->CaptureRow(context.x, y, row);
        if (context.sample)
            context.sample->Capture(context.x, y, row);
    }
    if (context.bypass)
        row.TransformBgr([](uint8_t* output, const uint8_t* input) { std::copy_n(input, 3, output); });
    else
        context.adjustment->RenderRow(row, *context.parameters);
}

struct RenderState
{
    explicit RenderState(const Parameters& initial)
        : parameters(std::make_shared<const Parameters>(initial)) {}

    std::atomic<unsigned> references{1};
    std::atomic<bool> bypass{false};
    std::shared_ptr<const Parameters> parameters;
};

void ManageRenderState(void* from, void* to, int operation)
{
    auto*& source = *static_cast<RenderState**>(from);
    switch (operation)
    {
    case 0: // Copy.
        *static_cast<RenderState**>(to) = source;
        if (source)
            ++source->references;
        break;
    case 1: // Move.
        *static_cast<RenderState**>(to) = source;
        source = nullptr;
        break;
    case 2: // Destroy.
        if (source && --source->references == 0)
            delete source;
        source = nullptr;
        break;
    }
}

RenderState& GetRenderState(uint8_t* info)
{
    return **reinterpret_cast<RenderState**>(info + 0x88);
}

std::shared_ptr<const Parameters> GetParameters(uint8_t* info)
{
    return std::atomic_load(&GetRenderState(info).parameters);
}

bool SetParameters(uint8_t* info, const Parameters& parameters)
{
    if (*GetParameters(info) == parameters)
        return false;
    std::atomic_store(&GetRenderState(info).parameters, std::make_shared<const Parameters>(parameters));
    return true;
}

uint32_t GetInfoType(const uint8_t* info)
{
    uint32_t type = 0;
    memcpy(&type, info + sizeof(void*), sizeof(type));
    return type;
}

int Render(const void*, uint8_t* info, void* destination, const void* region, void* source)
{
    const int format = getFormat(destination);
    const auto type = GetInfoType(info);
    const auto* adjustment = FindLayer(type);
    if (!adjustment || (format != 1 && format != 2))
        return 0;

    static const CallableTable<decltype(&RenderRow)> table{nullptr, RenderRow};
    const bool bypass = GetRenderState(info).bypass.load();
    const auto parameters = GetParameters(info);
    const auto observer = adjustment->ObserveSource(info);
    const auto sample = adjustment->WantsCanvasSample() ? std::atomic_load(&activeSample) : nullptr;
    RowContext context{adjustment, parameters.get(), source, observer.get(),
        sample && sample->renderIdentity == &GetRenderState(info) ? sample.get() : nullptr, bypass};
    auto* contextPointer = &context;
    Callable row{reinterpret_cast<uintptr_t>(&table) | 1u};
    memcpy(row.data, &contextPointer, sizeof(contextPointer));
    return renderRows(destination, region, source, &row);
}

void WriteParameters(const void*, uint8_t* info, void* stream)
{
    const auto parameters = GetParameters(info);
    for (const auto word : *parameters)
        write32(stream, word);
}

void ReadParameters(const void*, uint8_t* info, void* stream)
{
    // Old files have no payload. The host calls this callback only for a positive length.
    const auto* streamBytes = static_cast<const uint8_t*>(stream);
    int32_t position = 0;
    int32_t end = 0;
    memcpy(&position, streamBytes + 0x58, sizeof(position));
    memcpy(&end, streamBytes + 0x5c, sizeof(end));
    const auto* adjustment = FindLayer(GetInfoType(info));
    if (!adjustment || position < 0 || end <= position || (end - position) % sizeof(uint32_t))
        return;
    const size_t words = (end - position) / sizeof(uint32_t);
    if (words > adjustment->ParameterWordLimit())
        return;
    Parameters parameters(words);
    for (auto& word : parameters)
        word = read32(stream);
    SetParameters(info, adjustment->NormalizeParameters(parameters));
}

const CallableTable<decltype(&Render)> renderTable{ManageRenderState, Render};

const AdjustmentLayer* GetAdjustment(const uint8_t* info)
{
    if (!info)
        return nullptr;
    void* vtable = nullptr;
    uintptr_t renderer = 0;
    memcpy(&vtable, info, sizeof(vtable));
    if (vtable != expectedVtable)
        return nullptr;
    memcpy(&renderer, info + 0x80, sizeof(renderer));
    return renderer == reinterpret_cast<uintptr_t>(&renderTable) ?
        FindLayer(GetInfoType(info)) : nullptr;
}

bool GetController(SharedInfo& controller)
{
    uintptr_t app = 0;
    if (!controllerRoot || !CSPMOD::TryGetValue(controllerRoot, &app) || !app ||
        !CSPMOD::TryGetValue(app + 0x1d0, &controller) || !controller.object || !controller.control)
        return false;

    // The same shared controller is passed to CSP's native layer actions.
    return CSPMOD::IsPtrValid(controller.object, 0x108) && CSPMOD::IsPtrValid(controller.control, 16);
}

const AdjustmentLayer* GetSelectedAdjustment(const SharedInfo& controller, OwnedInfo& layer, OwnedInfo& info)
{
    getLayer(controller.object, &layer.value);
    if (!layer.value.object || !layer.value.control)
        return nullptr;
    getLayerProperty(layer.value.object, filterLayerProperty, &info.value);
    return GetAdjustment(info.value.object);
}

bool PrepareHostWindow(HWND host);

uint32_t QueryLayerType(SharedInfo* layer)
{
    const auto caller = _ReturnAddress();
    const auto identity = layer ? reinterpret_cast<LPARAM>(layer->object) : 0;
    const auto type = originalQueryType(layer);
    if (FindLayer(type) && identity && !dialogOpen &&
        (caller == paletteTypeReturn || caller == dialogTypeReturn))
    {
        const auto host = reinterpret_cast<HWND>(CspData::GetNativeWindowHandle());
        if (PrepareHostWindow(host))
            PostMessageW(host, editAdjustmentMessage, type, identity);
    }
    return type;
}

SharedInfo* BuildInfo(SharedInfo* output, uint32_t type, SharedInfo* source)
{
    const bool create = type == nativeInvert && pendingCreation && !pendingCreation->created &&
        _ReturnAddress() == createReturn;
    const auto* adjustment = create ? &pendingCreation->adjustment : FindLayer(type);
    if (!adjustment)
        return originalFactory(output, type, source);

    // Initialize the native object and preserve the preparation callback. The host
    // setters replace the native render and stream callbacks with our inline ones.
    auto* result = originalFactory(output, nativeInvert, source);
    if (result && result->object)
    {
        void* vtable = nullptr;
        uint32_t actualType = 0;
        memcpy(&vtable, result->object, sizeof(vtable));
        memcpy(&actualType, result->object + sizeof(vtable), sizeof(actualType));
        if (vtable == expectedVtable && actualType == nativeInvert)
        {
            static const CallableTable<decltype(&WriteParameters)> writer{nullptr, WriteParameters};
            static const CallableTable<decltype(&ReadParameters)> reader{nullptr, ReadParameters};
            const auto parameters = !create && source && GetAdjustment(source->object) == adjustment ?
                *GetParameters(source->object) : adjustment->DefaultParameters();
            const Callable write{reinterpret_cast<uintptr_t>(&writer) | 1u};
            const Callable read{reinterpret_cast<uintptr_t>(&reader) | 1u};
            Callable render{reinterpret_cast<uintptr_t>(&renderTable)};
            auto* state = new RenderState(parameters);
            memcpy(render.data, &state, sizeof(state));
            setStreams(result->object, &write, &read);
            setRender(result->object, &render);
            if (render.table)
                ManageRenderState(render.data, render.data, 2);
            memcpy(result->object + sizeof(vtable), &adjustment->type, sizeof(adjustment->type));
            if (create)
                pendingCreation->created = true;
        }
    }
    return result;
}

struct EditSession
{
    HWND window;
    SharedInfo controller;
    SharedInfo layer;
    uint8_t* info;
    Parameters pending;
    bool armed = false;

    void Notify()
    {
        auto ownerArg = controller;
        auto layerArg = layer;
        Retain(ownerArg);
        Retain(layerArg);
        notifyChange(&ownerArg, &layerArg);
    }

    void Apply(const Parameters& parameters)
    {
        if (SetParameters(info, parameters))
            Notify();
    }

    void Queue(const Parameters& parameters)
    {
        pending = parameters;
        if (GetRenderState(info).bypass.load())
            return;
        if (!armed)
            armed = SetTimer(window, previewTimer, previewIntervalMs, nullptr) != 0;
        if (!armed)
            Apply(pending);
    }

    void SetPreview(bool enabled)
    {
        CancelTimer();
        const bool changed = enabled && SetParameters(info, pending);
        const bool wasBypassed = GetRenderState(info).bypass.exchange(!enabled);
        if (changed || wasBypassed == enabled)
            Notify();
    }

    void Finish(const Parameters& parameters)
    {
        CancelTimer();
        const bool changed = SetParameters(info, parameters);
        const bool wasBypassed = GetRenderState(info).bypass.exchange(false);
        if (changed || wasBypassed)
            Notify();
    }

    void CancelTimer()
    {
        if (armed)
            KillTimer(window, previewTimer);
        armed = false;
    }
};

thread_local EditSession* activeEdit = nullptr;

struct CanvasView
{
    OwnedInfo reference;
    const MSG* motion = nullptr;
    HWND window = nullptr;
    HWND hitWindow = nullptr;
    double offset[2]{};
};

thread_local CanvasView* activeCanvas = nullptr;

void ObserveCanvasPosition(const void* view, const double* position)
{
    if (!activeCanvas || !activeCanvas->motion || !view || !position)
        return;
    auto& canvas = *activeCanvas;
    const auto& reference = *static_cast<const SharedInfo*>(view);
    if (!reference.object || !reference.control || !std::isfinite(position[0]) || !std::isfinite(position[1]))
        return;

    const auto screen = canvas.motion->pt;
    const HWND hit = ::WindowFromPoint(screen);
    if (!hit || ::GetAncestor(hit, GA_ROOT) != activeEdit->window)
        return;
    POINT hitPoint = screen;
    ::ScreenToClient(hit, &hitPoint);
    const HWND child = ::ChildWindowFromPointEx(hit, hitPoint, CWP_SKIPINVISIBLE | CWP_SKIPDISABLED);
    for (const HWND window : {child, hit, canvas.motion->hwnd})
    {
        POINT client = screen;
        if (!window || !::ScreenToClient(window, &client))
            continue;
        const double dx = position[0] - client.x, dy = position[1] - client.y;
        if (std::abs(dx) > 1.0 || std::abs(dy) > 1.0)
            continue;
        Retain(reference);
        Release(canvas.reference.value);
        canvas.reference.value = reference;
        canvas.window = window;
        canvas.hitWindow = hit;
        canvas.offset[0] = dx;
        canvas.offset[1] = dy;
        canvas.motion = nullptr;
        break;
    }
}

class DialogCanvasNavigation
{
public:
    DialogCanvasNavigation()
    {
        if (!getToolManager || !beginCanvasNavigation || !endCanvasNavigation || !beginModalMode || !endModalMode)
            return;
        getToolManager(&tools.value);
        if (!tools.value.object || !tools.value.control)
            return;

        // The native curve dialog uses modal mode (0, 4) and canvas mode 1.
        beginModalMode(modal, 0, 4);
        beginCanvasNavigation(tools.value.object, 1);
        active = true;
    }

    ~DialogCanvasNavigation()
    {
        if (!active)
            return;
        SharedInfo noNativeDialog{};
        endCanvasNavigation(tools.value.object, &noNativeDialog);
        endModalMode(modal);
    }

    bool IsActive() const { return active; }

private:
    OwnedInfo tools;
    alignas(16) std::byte modal[0x60]{};
    bool active = false;
};

bool WriteTitle(void* label, const wchar_t* text)
{
    const auto length = static_cast<int>(wcslen(text));
    auto* buffer = menuResizeLabel(label, length);
    if (!buffer)
        return false;
    memcpy(buffer, text, static_cast<size_t>(length) * sizeof(wchar_t));
    return true;
}

bool SetCreatedLayerName(uint8_t* layer, const AdjustmentLayer& adjustment)
{
    if (!layerNameOffset || !layerNameVtable || !menuResizeLabel)
        return false;
    auto* field = layer + layerNameOffset;
    if (!CSPMOD::IsPtrValid(field, 0x30))
        return false;
    const void* vtable = nullptr;
    memcpy(&vtable, field, sizeof(vtable));
    const auto* name = adjustment.DefaultLayerName(GetUserDefaultUILanguage());
    return vtable == layerNameVtable && name && *name && WriteTitle(field, name);
}

bool EditAdjustment(HWND window, const SharedInfo& controller, const SharedInfo& layer, uint8_t* info,
    const AdjustmentLayer& adjustment, bool newlyCreated)
{
    OwnedInfo undo;
    if (!newlyCreated)
    {
        auto layerArg = layer;
        Retain(layerArg);
        createUndo(&undo.value, &layerArg);
        if (!undo.value.object)
            return false;
    }

    const auto original = *GetParameters(info);
    auto parameters = original;
    EditSession session{window, controller, layer, info, original};
    if (newlyCreated)
    {
        // Keep the name on the layer created by the native command and use the same undo entry.
        if (SetCreatedLayerName(layer.object, adjustment))
            session.Notify();
        else
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Custom adjustment: unable to set the created layer name.");
    }
    activeEdit = &session;
    bool accepted = false;
    {
        const DialogCanvasNavigation navigation;
        const bool observing = adjustment.BeginSourceObservation(info);
        if (observing)
            session.Notify();
        CanvasView canvas;
        if (adjustment.WantsCanvasSample())
        {
            uint32_t width = 0, height = 0;
            if (navigation.IsActive() && CspData::GetCurrentProjCanvasSize(&width, &height) && width && height &&
                width <= INT_MAX && height <= INT_MAX && NativeCanvasCoordinates::SetObserver(ObserveCanvasPosition))
            {
                std::atomic_store(&activeSample, std::make_shared<SourceSample>(&GetRenderState(info), width, height));
                activeCanvas = &canvas;
            }
        }
        accepted = adjustment.Edit(window, parameters, {
            [&](const Parameters& next) { session.Queue(adjustment.NormalizeParameters(next)); },
            [&](bool enabled) { session.SetPreview(enabled); }
        }, navigation.IsActive());
        if (observing)
            adjustment.EndSourceObservation();
        if (adjustment.WantsCanvasSample())
        {
            activeCanvas = nullptr;
            NativeCanvasCoordinates::SetObserver(nullptr);
            std::atomic_store(&activeSample, std::shared_ptr<SourceSample>{});
        }
    }
    activeEdit = nullptr;
    parameters = accepted ? adjustment.NormalizeParameters(parameters) : original;
    session.Finish(parameters);

    if (accepted && parameters != original && undo.value.object)
    {
        auto command = undo.value;
        undo.value = {};
        submitUndo(controller.object, &command);
    }
    return accepted;
}

bool RetainController(OwnedInfo& controller)
{
    SharedInfo current{};
    if (!GetController(current))
        return false;
    Retain(current);
    controller.value = current;
    return true;
}

void EditOnHostThread(HWND window, uint32_t type, const uint8_t* expectedLayer)
{
    OwnedInfo controller;
    OwnedInfo layer;
    OwnedInfo info;
    if (!RetainController(controller))
        return;
    const auto* adjustment = GetSelectedAdjustment(controller.value, layer, info);
    if (adjustment && adjustment->type == type && (!expectedLayer || layer.value.object == expectedLayer))
        EditAdjustment(window, controller.value, layer.value, info.value.object, *adjustment, false);
}

void CreateOnHostThread(HWND window, const RegisteredAdjustment& entry)
{
    if (pendingCreation || !entry.CanCreate())
        return;

    const auto& adjustment = entry.adjustment;
    const uint32_t command = lookupCommand(commandToken);
    const uint32_t cancelCreation = lookupCommand(undoToken);
    OwnedInfo controller;
    if (!command || !cancelCreation || !RetainController(controller))
        return;

    OwnedInfo previousLayer;
    getLayer(controller.value.object, &previousLayer.value);
    struct Request
    {
        Creation creation;
        explicit Request(const AdjustmentLayer& adjustment) : creation{adjustment} { pendingCreation = &creation; }
        ~Request() { pendingCreation = nullptr; }
    };
    bool created = false;
    {
        Request request(adjustment);
        dispatchCommand(command);
        created = request.creation.created;
    }

    OwnedInfo layer;
    OwnedInfo info;
    if (!created || GetSelectedAdjustment(controller.value, layer, info) != &adjustment ||
        layer.value.object == previousLayer.value.object)
        return;

    if (!EditAdjustment(window, controller.value, layer.value, info.value.object, adjustment, true))
    {
        // Only undo this creation while its document and layer are still current.
        SharedInfo current{};
        OwnedInfo currentLayer;
        OwnedInfo currentInfo;
        if (GetController(current) && current.object == controller.value.object &&
            GetSelectedAdjustment(current, currentLayer, currentInfo) == &adjustment &&
            currentLayer.value.object == layer.value.object && currentInfo.value.object == info.value.object)
            dispatchUndo(cancelCreation);
    }
}

LRESULT CALLBACK HostWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam,
    UINT_PTR id, DWORD_PTR)
{
    if (message == createAdjustmentMessage || message == editAdjustmentMessage)
    {
        if (!dialogOpen)
        {
            dialogOpen = true;
            const auto type = static_cast<uint32_t>(wParam);
            if (message == editAdjustmentMessage)
                EditOnHostThread(window, type, reinterpret_cast<const uint8_t*>(lParam));
            else if (const auto* entry = FindRegistration(type))
                CreateOnHostThread(window, *entry);
            dialogOpen = false;
        }
        return 0;
    }
    if (message == WM_TIMER && wParam == previewTimer && activeEdit && activeEdit->window == window)
    {
        if (activeEdit->armed)
        {
            activeEdit->CancelTimer();
            activeEdit->Apply(activeEdit->pending);
        }
        return 0;
    }
    if (message == WM_NCDESTROY)
        RemoveWindowSubclass(window, HostWindowProc, id);
    return DefSubclassProc(window, message, wParam, lParam);
}

bool PrepareHostWindow(HWND host)
{
    return createAdjustmentMessage && editAdjustmentMessage && host &&
        GetWindowThreadProcessId(host, nullptr) == GetCurrentThreadId() &&
        SetWindowSubclass(host, HostWindowProc, subclassId, 0);
}

bool HasLayerForCreation()
{
    OwnedInfo controller;
    if (!RetainController(controller))
        return false;
    OwnedInfo layer;
    getLayer(controller.value.object, &layer.value);
    return layer.value.object && layer.value.control;
}

bool HasVtable(const uint8_t* object, const void* expected)
{
    const void* actual = nullptr;
    if (object)
        memcpy(&actual, object, sizeof(actual));
    return actual == expected;
}

uint32_t ResolveCommand(RegisteredAdjustment& entry)
{
    auto command = entry.command.load(std::memory_order_relaxed);
    if (!command)
    {
        command = lookupCommand(&entry.token);
        entry.command.store(command, std::memory_order_relaxed);
    }
    return command;
}

RegisteredAdjustment* FindCommand(uint32_t command, bool resolve = false)
{
    if (!command)
        return nullptr;
    const auto entry = std::find_if(std::begin(adjustments), std::end(adjustments), [=](auto& candidate) {
        const auto id = resolve ? ResolveCommand(candidate) : candidate.command.load(std::memory_order_relaxed);
        return id == command;
    });
    return entry == std::end(adjustments) ? nullptr : &*entry;
}

uint32_t GetMenuEventCommand(const void* eventId)
{
    const auto caller = _ReturnAddress();
    const auto command = originalGetMenuEventCommand(eventId);
    if (std::find(menuStatusReturns.begin(), menuStatusReturns.end(), caller) != menuStatusReturns.end())
        if (const auto* entry = FindCommand(command); entry && entry->CanCreate())
            return lookupCommand(commandToken);
    return command;
}

bool InvokeAdjustment(const RegisteredAdjustment& entry)
{
    if (!entry.CanCreate() || dialogOpen)
        return false;
    const auto host = reinterpret_cast<HWND>(CspData::GetNativeWindowHandle());
    return host && IsWindowEnabled(host) && PrepareHostWindow(host) && HasLayerForCreation() &&
        PostMessageW(host, createAdjustmentMessage, entry.adjustment.type, 0);
}

int SendMenuCommand(void* sender, uint32_t command, int mode)
{
    if (const auto* entry = FindCommand(command, originalDispatchBinding != nullptr))
        return InvokeAdjustment(*entry);
    return originalMenuSendCommand(sender, command, mode);
}

int DispatchCommandBinding(void* dispatcher, uint32_t command, int parameter, int mode)
{
    if (const auto* entry = FindCommand(command, true))
        return InvokeAdjustment(*entry);
    return originalDispatchBinding(dispatcher, command, parameter, mode);
}

int GetToolbarCommandStatus(void* view, uint32_t command)
{
    if (_ReturnAddress() != toolbarStatusReturn)
        return originalToolbarStatus(view, command);
    const auto* entry = FindCommand(command, true);
    if (!entry)
        return originalToolbarStatus(view, command);

    const auto host = reinterpret_cast<HWND>(CspData::GetNativeWindowHandle());
    // CSP caches this bit. A refresh during a parameter edit can outlive the
    // dialog, so check temporary input restrictions when invoking the command.
    const bool available = entry->CanCreate() && host && HasLayerForCreation();
    const int nativeStatus = originalToolbarStatus(view, lookupCommand(commandToken));
    constexpr int enabled = 1;
    return available ? nativeStatus | enabled : nativeStatus & ~enabled;
}

CatalogRow* FindCorrectionGroupEnd(const CatalogRows& rows)
{
    if (!rows.begin || rows.end - rows.begin < static_cast<ptrdiff_t>(NativeCorrectionCommandCount))
        return nullptr;

    std::array<uint32_t, NativeCorrectionCommandCount> commands{};
    for (size_t i = 0; i < commands.size(); ++i)
    {
        commands[i] = lookupCommand(correctionMenuTokens[i]);
        if (!commands[i])
            return nullptr;
    }
    CatalogRow* match = nullptr;
    for (auto* row = rows.begin + commands.size() - 1; row != rows.end; ++row)
    {
        if (row->command != commands.back())
            continue;
        const auto* first = row - (commands.size() - 1);
        if (!std::equal(commands.begin(), commands.end(), first,
                [&](uint32_t command, const CatalogRow& item) {
                    return item.command == command && item.depth == row->depth;
                }))
            continue;
        if (match)
            return nullptr;
        match = row;
    }
    return match;
}

struct CatalogLabel
{
    alignas(void*) std::byte storage[0x30];
    CatalogLabel() { constructCatalogLabel(storage); }
    ~CatalogLabel() { destroyCatalogLabel(storage); }

    bool SetTitle(const AdjustmentLayer& adjustment)
    {
        return WriteTitle(storage, adjustment.Title(GetUserDefaultUILanguage()));
    }
};

void RegisterToolbarCommand(uint32_t command, int depth, uint32_t icon, int flags, const CatalogLabel& label)
{
    ToolbarMetadata existing{};
    if (!toolbarRoot || !*toolbarRoot || !icon || lookupToolbarMetadata(command, &existing))
        return;
    addToolbarItem(*toolbarRoot, command, command, 0, depth, icon, label.storage, flags);
}

void EnsureToolbarCommand(uint32_t command, const CatalogLabel& label)
{
    ToolbarMetadata native{};
    if (lookupToolbarMetadata && lookupToolbarMetadata(lookupCommand(commandToken), &native))
        RegisterToolbarCommand(command, native.depth, native.icon, native.flags, label);
}

void RestoreCorrectionToolbarButton(SharedInfo* bar, int index, uint32_t command, int style,
    int color, int option, const void* title, int flag, int update)
{
    if (const auto* entry = FindCommand(command, true))
    {
        ToolbarMetadata existing{};
        if (!lookupToolbarMetadata(command, &existing))
        {
            CatalogLabel label;
            if (label.SetTitle(entry->adjustment))
                EnsureToolbarCommand(command, label);
        }
    }
    originalRestoreToolbarButton(bar, index, command, style, color, option, title, flag, update);
}

void WrapToolbarRegistration(void* root, uint32_t command, int depth, uint32_t icon, uint32_t resource, int flags)
{
    const auto caller = _ReturnAddress();
    originalWrapToolbarItem(root, command, depth, icon, resource, flags);
    if (caller != nativeIconReturn || root != *toolbarRoot)
        return;

    for (auto& entry : adjustments)
    {
        // Saved buttons need their command metadata even before the license is loaded.
        const auto custom = ResolveCommand(entry);
        if (!custom || custom == command)
            continue;
        CatalogLabel label;
        if (label.SetTitle(entry.adjustment))
            RegisterToolbarCommand(custom, depth, icon, flags, label);
    }
}

void RegisterCatalogCommands(CatalogRows* rows, CatalogRow gradient)
{
    // Appending can reallocate the host vector. Retain a copy of the gradient row's metadata.
    for (auto& entry : adjustments)
    {
        if (!entry.supportsCreation)
            continue;
        const uint32_t command = ResolveCommand(entry);
        if (!command || std::any_of(rows->begin, rows->end,
                [&](const auto& row) { return row.command == command; }))
            continue;

        CatalogLabel label;
        if (!label.SetTitle(entry.adjustment))
            continue;
        EnsureToolbarCommand(command, label);
        catalogAppend(rows, gradient.flags, gradient.depth, gradient.mode, command, label.storage);
        SDL_Log("Custom adjustment: registered %s in the menu command catalog.", entry.adjustment.commandName);
    }
}

void WrapCatalogAppend(CatalogRows* rows, int flags, int depth, int mode, uint32_t command,
    uint32_t resource, SharedInfo* source)
{
    const auto caller = _ReturnAddress();
    originalCatalogWrapAppend(rows, flags, depth, mode, command, resource, source);
    if (caller == catalogGradientReturn && rows == *catalogRoot)
    {
        const auto* gradient = FindCorrectionGroupEnd(*rows);
        if (gradient && gradient == rows->end - 1)
            RegisterCatalogCommands(rows, *gradient);
    }
}

void ReadCommandCatalog(CatalogRows* output)
{
    auto* rows = *catalogRoot;
    const auto host = reinterpret_cast<HWND>(CspData::GetNativeWindowHandle());
    if (rows && rows != output && host && GetWindowThreadProcessId(host, nullptr) == GetCurrentThreadId())
    {
        if (const auto* gradient = FindCorrectionGroupEnd(*rows))
            RegisterCatalogCommands(rows, *gradient);
    }

    originalCatalogRead(output);
    if (output == rows)
        return;

    // On a late attachment the host appends to its indexed catalog. Reorder only
    // this independent copy so the canonical command index remains valid.
    auto* gradient = FindCorrectionGroupEnd(*output);
    if (!gradient)
        return;
    auto* insert = gradient + 1;
    for (const auto& entry : adjustments)
    {
        const auto command = entry.command.load(std::memory_order_relaxed);
        if (!command)
            continue;
        const auto found = std::find_if(insert, output->end, [&](const auto& row) { return row.command == command; });
        if (found != output->end)
        {
            std::rotate(insert, found, found + 1);
            ++insert;
        }
    }
}

SharedInfo* ReadSubmenu(const void* parent, SharedInfo* output)
{
    auto* result = originalGetSubmenu(parent, output);
    if (!HasVtable(static_cast<const uint8_t*>(parent), menuItemVtable) ||
        !HasVtable(output->object, menuVtable))
        return result;
    const auto count = menuCount(output->object);
    if (count < correctionMenuTokens.size() || count > correctionMenuTokens.size() + std::size(adjustments))
        return result;
    const auto host = reinterpret_cast<HWND>(CspData::GetNativeWindowHandle());
    if (!host || GetWindowThreadProcessId(host, nullptr) != GetCurrentThreadId())
        return result;

    if (!correctionMenuCommands.back())
    {
        std::array<uint32_t, NativeCorrectionCommandCount> commands{};
        for (size_t i = 0; i < correctionMenuTokens.size(); ++i)
        {
            commands[i] = lookupCommand(correctionMenuTokens[i]);
            if (!commands[i])
                return result;
        }
        correctionMenuCommands = commands;
    }

    // Verify the native prefix and any already appended custom commands.
    OwnedInfo nativeItem;
    for (size_t i = 0; i < correctionMenuCommands.size(); ++i)
    {
        OwnedInfo item;
        menuGetItem(output->object, &item.value, static_cast<int>(i));
        if (!HasVtable(item.value.object, menuItemVtable) ||
            menuGetCommand(item.value.object) != correctionMenuCommands[i])
            return result;
        if (i + 1 == correctionMenuCommands.size())
            std::swap(item.value, nativeItem.value);
    }
    std::vector<const RegisteredAdjustment*> present;
    present.reserve(adjustments.size());
    for (size_t i = correctionMenuCommands.size(); i < count; ++i)
    {
        OwnedInfo item;
        menuGetItem(output->object, &item.value, static_cast<int>(i));
        if (!HasVtable(item.value.object, menuItemVtable))
            return result;
        const auto* entry = FindCommand(menuGetCommand(item.value.object));
        if (!entry || std::find(present.begin(), present.end(), entry) != present.end())
            return result;
        present.push_back(entry);
    }
    if (!HasLayerForCreation())
        return result;

    constexpr size_t labelField = 0x10;
    for (auto& entry : adjustments)
    {
        if (!entry.CanCreate() || std::find(present.begin(), present.end(), &entry) != present.end())
            continue;
        const auto command = ResolveCommand(entry);
        if (!command || std::find(correctionMenuCommands.begin(), correctionMenuCommands.end(),
                command) != correctionMenuCommands.end())
            break;

        const auto& adjustment = entry.adjustment;
        const auto label = adjustment.menuLabelId ?
            ui::GlobalManager::Instance().Lang().GetStringViaID(adjustment.menuLabelId) : DString{};
        const auto* text = label.empty() ? adjustment.Title(MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT)) : label.c_str();
        OwnedInfo customItem;
        menuMakeItem(&customItem.value, 0, nativeItem.value.object + labelField, command);
        if (!HasVtable(customItem.value.object, menuItemVtable) ||
            !WriteTitle(customItem.value.object + labelField, text))
            break;

        // The host append wrapper consumes the incoming reference after inserting it.
        auto transferred = customItem.value;
        customItem.value = {};
        menuAppend(output->object, &transferred);
        SDL_Log("Custom adjustment: appended %s to the native correction menu.", adjustment.commandName);
    }
    return result;
}

bool InstallNativeMenu()
{
    void* readSubmenu = AddressTable::GetAddress("CustomAdjustment_MenuGetSubmenu");
    void* sendCommand = AddressTable::GetAddress("CustomAdjustment_MenuSendCommand");
    void* getEventCommand = AddressTable::GetAddress("CustomAdjustment_MenuGetEventCommand");
    menuCount = reinterpret_cast<MenuCount>(AddressTable::GetAddress("CustomAdjustment_MenuItemCount"));
    menuGetItem = reinterpret_cast<MenuGetItem>(AddressTable::GetAddress("CustomAdjustment_MenuGetItem"));
    menuGetCommand = reinterpret_cast<MenuGetCommand>(AddressTable::GetAddress("CustomAdjustment_MenuGetCommand"));
    menuMakeItem = reinterpret_cast<MenuMakeItem>(AddressTable::GetAddress("CustomAdjustment_MenuMakeItem"));
    menuAppend = reinterpret_cast<MenuAppend>(AddressTable::GetAddress("CustomAdjustment_MenuAppendItem"));
    menuResizeLabel = reinterpret_cast<MenuResizeLabel>(AddressTable::GetAddress("CustomAdjustment_MenuResizeLabel"));
    layerNameVtable = AddressTable::GetAddress("CustomAdjustment_LayerNameVtable");
    if (const auto* nameSite = static_cast<const uint8_t*>(AddressTable::GetAddress("CustomAdjustment_LayerNameSite")))
    {
        int32_t displacement = 0;
        memcpy(&displacement, nameSite + 3, sizeof(displacement));
        if (displacement > 0 && displacement % sizeof(void*) == 0)
            layerNameOffset = static_cast<uint32_t>(displacement);
    }
    menuVtable = AddressTable::GetAddress("CustomAdjustment_MenuVtable");
    menuItemVtable = AddressTable::GetAddress("CustomAdjustment_MenuItemVtable");
    if (!readSubmenu || !sendCommand || !getEventCommand || !menuCount || !menuGetItem || !menuGetCommand ||
        !menuMakeItem || !menuAppend || !menuResizeLabel || !menuVtable || !menuItemVtable)
        return false;
    for (size_t i = 0; i < correctionMenuTokens.size(); ++i)
    {
        const auto key = "CustomAdjustment_MenuToken_" + std::to_string(i);
        correctionMenuTokens[i] = AddressTable::GetAddress(key.c_str());
        if (!correctionMenuTokens[i])
            return false;
    }

    for (size_t i = 0; i < menuStatusReturns.size(); ++i)
    {
        const auto key = "CustomAdjustment_MenuStatusReturn_" + std::to_string(i);
        menuStatusReturns[i] = AddressTable::GetAddress(key.c_str());
        if (!menuStatusReturns[i])
            return false;
    }

    if (!CSPMOD::Hook(getEventCommand, reinterpret_cast<void*>(GetMenuEventCommand),
            reinterpret_cast<void**>(&originalGetMenuEventCommand), CSPMOD::_HOOKREG_R11) ||
        !CSPMOD::Hook(sendCommand, reinterpret_cast<void*>(SendMenuCommand),
            reinterpret_cast<void**>(&originalMenuSendCommand), CSPMOD::_HOOKREG_R11) ||
        !CSPMOD::Hook(readSubmenu, reinterpret_cast<void*>(ReadSubmenu),
            reinterpret_cast<void**>(&originalGetSubmenu), CSPMOD::_HOOKREG_R11))
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Custom adjustment: unable to install the native menu.");
        return false;
    }
    return true;
}

void InstallCommandBindings()
{
    auto* commandNamespace = static_cast<const char*>(AddressTable::GetAddress("CustomAdjustment_BindingNamespace"));
    void* dispatch = AddressTable::GetAddress("CustomAdjustment_BindingDispatch");
    void* iconWrapper = AddressTable::GetAddress("CustomAdjustment_BindingIconWrapper");
    auto* iconRoot = static_cast<void* const*>(AddressTable::GetAddress("CustomAdjustment_BindingIconRoot"));
    const auto metadataLookup = reinterpret_cast<LookupToolbarMetadata>(
        AddressTable::GetAddress("CustomAdjustment_BindingToolbarMetadata"));
    const auto iconAdd = reinterpret_cast<AddToolbarItem>(AddressTable::GetAddress("CustomAdjustment_BindingIconAdd"));
    const auto* iconReturn = AddressTable::GetAddress("CustomAdjustment_BindingNativeIconReturn");
    if (!commandNamespace || !dispatch || !iconWrapper || !iconRoot || !metadataLookup || !iconAdd || !iconReturn)
        return;

    if (!CSPMOD::Hook(dispatch, reinterpret_cast<void*>(DispatchCommandBinding),
            reinterpret_cast<void**>(&originalDispatchBinding), CSPMOD::_HOOKREG_R11))
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Custom adjustment: unable to enable native command bindings.");
        return;
    }
    toolbarRoot = iconRoot;
    lookupToolbarMetadata = metadataLookup;
    addToolbarItem = iconAdd;
    nativeIconReturn = iconReturn;
    if (!CSPMOD::Hook(iconWrapper, reinterpret_cast<void*>(WrapToolbarRegistration),
            reinterpret_cast<void**>(&originalWrapToolbarItem), CSPMOD::_HOOKREG_R11))
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Custom adjustment: command bar registration deferred until catalog read.");

    void* status = AddressTable::GetAddress("CustomAdjustment_BindingToolbarStatus");
    toolbarStatusReturn = AddressTable::GetAddress("CustomAdjustment_BindingToolbarStatusReturn");
    if (status && toolbarStatusReturn &&
        !CSPMOD::Hook(status, reinterpret_cast<void*>(GetToolbarCommandStatus),
            reinterpret_cast<void**>(&originalToolbarStatus), CSPMOD::_HOOKREG_R11))
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Custom adjustment: unable to enable command bar button status.");

    void* restore = AddressTable::GetAddress("CustomAdjustment_BindingToolbarRestore");
    if (restore && !CSPMOD::Hook(restore, reinterpret_cast<void*>(RestoreCorrectionToolbarButton),
            reinterpret_cast<void**>(&originalRestoreToolbarButton), CSPMOD::_HOOKREG_R11))
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Custom adjustment: unable to prepare saved command bar buttons.");
}

void InstallCommandCatalog()
{
    void* read = AddressTable::GetAddress("CustomAdjustment_CatalogRead");
    void* appendWrapper = AddressTable::GetAddress("CustomAdjustment_CatalogWrapAppend");
    catalogRoot = static_cast<CatalogRows* const*>(AddressTable::GetAddress("CustomAdjustment_CatalogRoot"));
    catalogGradientReturn = AddressTable::GetAddress("CustomAdjustment_CatalogGradientReturn");
    catalogAppend = reinterpret_cast<CatalogAppend>(AddressTable::GetAddress("CustomAdjustment_CatalogAppend"));
    constructCatalogLabel = reinterpret_cast<ConstructLabel>(AddressTable::GetAddress("CustomAdjustment_CatalogLabelConstruct"));
    destroyCatalogLabel = reinterpret_cast<DestroyLabel>(AddressTable::GetAddress("CustomAdjustment_CatalogLabelDestroy"));
    if (!read || !appendWrapper || !catalogRoot || !catalogGradientReturn || !catalogAppend ||
        !constructCatalogLabel || !destroyCatalogLabel)
        return;
    InstallCommandBindings();

    if (!CSPMOD::Hook(read, reinterpret_cast<void*>(ReadCommandCatalog),
            reinterpret_cast<void**>(&originalCatalogRead), CSPMOD::_HOOKREG_R11))
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Custom adjustment: unable to extend the menu command lists.");
        return;
    }
    if (!CSPMOD::Hook(appendWrapper, reinterpret_cast<void*>(WrapCatalogAppend),
            reinterpret_cast<void**>(&originalCatalogWrapAppend), CSPMOD::_HOOKREG_R11))
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Custom adjustment: command registration deferred until the first list read.");
}
}

void BindPreviewCheckbox(ui::Window& dialog, const SetPreviewEnabled& setEnabled)
{
    if (auto* checkbox = static_cast<ui::CheckBox*>(dialog.FindControl(L"preview")))
    {
        checkbox->SetSelected(true);
        checkbox->AttachSelect([setEnabled](const ui::EventArgs&) { setEnabled(true); return true; });
        checkbox->AttachUnSelect([setEnabled](const ui::EventArgs&) { setEnabled(false); return true; });
    }
}

bool TrackCanvasMouse(const MSG& message)
{
    if (!activeCanvas || message.message != WM_MOUSEMOVE)
        return false;
    activeCanvas->motion = &message;
    ::DispatchMessageW(&message);
    activeCanvas->motion = nullptr;
    return true;
}

bool CanSampleCanvas() { return activeCanvas != nullptr; }

bool RequestCanvasSample(const MSG& message)
{
    const auto sample = std::atomic_load(&activeSample);
    if (!sample || !activeEdit || !activeCanvas)
        return false;

    POINT screen = message.pt;
    const auto& canvas = *activeCanvas;
    const auto viewUnderPointer = [&] {
        return canvas.reference.value.object && ::IsWindow(canvas.window) && canvas.hitWindow &&
            ::WindowFromPoint(screen) == canvas.hitWindow;
    };
    if (!viewUnderPointer())
    {
        // Let the native canvas process a move when the click is the first event over this view.
        MSG motion = message;
        motion.message = WM_MOUSEMOVE;
        motion.wParam &= ~WPARAM(MK_LBUTTON);
        TrackCanvasMouse(motion);
        if (!viewUnderPointer())
            return false;
    }
    if (!::ScreenToClient(canvas.window, &screen))
        return false;
    RECT client{};
    if (!::GetClientRect(canvas.window, &client) || !::PtInRect(&client, screen))
        return false;
    double point[2]{screen.x + canvas.offset[0], screen.y + canvas.offset[1]};
    double mapped[2]{};
    auto argument = canvas.reference.value;
    Retain(argument); // The host consumes this shared reference.
    NativeCanvasCoordinates::Convert(mapped, &argument, point);
    if (!std::isfinite(mapped[0]) || !std::isfinite(mapped[1]) ||
        mapped[0] < 0.0 || mapped[1] < 0.0 || mapped[0] >= sample->width || mapped[1] >= sample->height)
        return false;
    sample->Request(static_cast<int>(mapped[0]), static_cast<int>(mapped[1]));
    activeEdit->Notify();
    return true;
}

bool ReadCanvasSample(std::optional<SourceColor>& color)
{
    const auto sample = std::atomic_load(&activeSample);
    return sample && sample->Read(color);
}

void CancelCanvasSample()
{
    if (const auto sample = std::atomic_load(&activeSample))
        sample->Cancel();
}

void Register(const AdjustmentLayer& adjustment, bool supportsCreation, CreationGate gate)
{
    adjustments.emplace_back(adjustment, supportsCreation, gate);
}

void Install()
{
    if (adjustments.empty() || !ValidateRegistry())
        return;
    if (const auto* commandNamespace = static_cast<const char*>(
            AddressTable::GetAddress("CustomAdjustment_BindingNamespace")))
        for (auto& entry : adjustments)
            entry.token.nameSpace = commandNamespace;
    void* factory = AddressTable::GetAddress("CustomAdjustment_Factory");
    expectedVtable = AddressTable::GetAddress("CustomAdjustment_InfoVtable");
    createReturn = AddressTable::GetAddress("CustomAdjustment_CreateReturn");
    commandToken = AddressTable::GetAddress("CustomAdjustment_CommandToken");
    lookupCommand = reinterpret_cast<Lookup>(AddressTable::GetAddress("CustomAdjustment_CommandLookup"));
    dispatchCommand = reinterpret_cast<Dispatch>(AddressTable::GetAddress("CustomAdjustment_CommandDispatch"));
    dispatchUndo = reinterpret_cast<Dispatch>(AddressTable::GetAddress("CustomAdjustment_UndoDispatch"));
    undoToken = AddressTable::GetAddress("CustomAdjustment_UndoToken");
    void* queryType = AddressTable::GetAddress("CustomAdjustment_QueryType");
    paletteTypeReturn = AddressTable::GetAddress("CustomAdjustment_PaletteTypeReturn");
    dialogTypeReturn = AddressTable::GetAddress("CustomAdjustment_DialogTypeReturn");
    getToolManager = reinterpret_cast<GetToolManager>(
        AddressTable::GetAddress("CustomAdjustment_DialogGetToolManager"));
    beginCanvasNavigation = reinterpret_cast<BeginCanvasNavigation>(
        AddressTable::GetAddress("CustomAdjustment_DialogBeginCanvas"));
    endCanvasNavigation = reinterpret_cast<EndCanvasNavigation>(
        AddressTable::GetAddress("CustomAdjustment_DialogEndCanvas"));
    beginModalMode = reinterpret_cast<BeginModalMode>(
        AddressTable::GetAddress("CustomAdjustment_DialogBeginModal"));
    endModalMode = reinterpret_cast<EndModalMode>(
        AddressTable::GetAddress("CustomAdjustment_DialogEndModal"));
    setStreams = reinterpret_cast<SetStreams>(AddressTable::GetAddress("CustomAdjustment_SetStreams"));
    setRender = reinterpret_cast<SetRender>(AddressTable::GetAddress("CustomAdjustment_SetRender"));
    renderRows = reinterpret_cast<RenderRows>(AddressTable::GetAddress("CustomAdjustment_RenderRows"));
    getFormat = reinterpret_cast<GetFormat>(AddressTable::GetAddress("CustomAdjustment_GetFormat"));
    read32 = reinterpret_cast<Read32>(AddressTable::GetAddress("CustomAdjustment_Read32"));
    write32 = reinterpret_cast<Write32>(AddressTable::GetAddress("CustomAdjustment_Write32"));
    getLayer = reinterpret_cast<GetLayer>(AddressTable::GetAddress("CustomAdjustment_GetLayer"));
    getLayerProperty = reinterpret_cast<GetLayerProperty>(AddressTable::GetAddress("CustomAdjustment_GetLayerProperty"));
    createUndo = reinterpret_cast<CreateUndo>(AddressTable::GetAddress("CustomAdjustment_CreateUndo"));
    submitUndo = reinterpret_cast<SubmitUndo>(AddressTable::GetAddress("CustomAdjustment_SubmitUndo"));
    notifyChange = reinterpret_cast<NotifyChange>(AddressTable::GetAddress("CustomAdjustment_NotifyChange"));
    controllerRoot = AddressTable::GetAddress("CustomAdjustment_ControllerRoot");
    if (!factory || !expectedVtable || !createReturn || !commandToken || !lookupCommand || !dispatchCommand ||
        !dispatchUndo || !undoToken || !queryType || !paletteTypeReturn || !dialogTypeReturn ||
        !setStreams || !setRender || !renderRows || !getFormat || !read32 || !write32 || !getLayer ||
        !getLayerProperty || !createUndo || !submitUndo || !notifyChange || !controllerRoot)
        return;

    const UINT createMessage = RegisterWindowMessageW(L"CSPMOD.CustomAdjustment.Create");
    const UINT editMessage = RegisterWindowMessageW(L"CSPMOD.CustomAdjustment.Edit");
    if (!createMessage || !editMessage || !CSPMOD::Hook(factory, reinterpret_cast<void*>(BuildInfo),
            reinterpret_cast<void**>(&originalFactory), CSPMOD::_HOOKREG_R11))
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Custom adjustment: unable to install the layer factory.");
        return;
    }
    createAdjustmentMessage = createMessage;
    editAdjustmentMessage = editMessage;
    if (!CSPMOD::Hook(queryType, reinterpret_cast<void*>(QueryLayerType),
            reinterpret_cast<void**>(&originalQueryType), CSPMOD::_HOOKREG_R11))
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Custom adjustment: unable to install thumbnail editing.");
    if (InstallNativeMenu())
        InstallCommandCatalog();
}

}
