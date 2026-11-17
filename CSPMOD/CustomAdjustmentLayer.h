#pragma once

#include <Windows.h>
#include <array>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <optional>
#include <memory>
#include <vector>

namespace Json { class Value; }
namespace ui { class Window; }

namespace CustomAdjustmentLayer
{
// CSPMOD's own allocation. The host does not reserve this range for plug-ins.
inline constexpr uint32_t UserStart = 0x43530000;
inline constexpr size_t NativeCorrectionCommandCount = 9;

using Parameters = std::vector<uint32_t>;
using SetPreviewEnabled = std::function<void(bool)>;
struct Preview
{
    std::function<void(const Parameters&)> update;
    SetPreviewEnabled setEnabled;
};
void BindPreviewCheckbox(ui::Window& dialog, const SetPreviewEnabled& setEnabled);
using SourceColor = std::array<uint8_t, 3>; // BGR order.

struct PixelRow
{
    uint8_t* destination;
    const uint8_t* source;
    const uint8_t* alpha;
    int destinationStride;
    int sourceStride;
    int alphaStride;
    int width;

    template<class Transform>
    void TransformBgr(Transform&& transform) const
    {
        auto* output = destination;
        auto* input = source;
        auto* opacity = alpha;
        for (int x = 0; x < width; ++x)
        {
            if (*opacity)
                transform(output, input);
            if (x + 1 < width)
            {
                output += destinationStride;
                input += sourceStride;
                opacity += alphaStride;
            }
        }
    }
};

class SourceRowObserver
{
public:
    virtual ~SourceRowObserver() = default;
    virtual void CaptureRow(int x, int y, const PixelRow& row) = 0;
};

// One immutable definition per registered type. The host can render on multiple threads.
class AdjustmentLayer
{
public:
    AdjustmentLayer(uint32_t type, const char* commandName, const wchar_t* menuLabelId)
        : type(type), commandName(commandName), menuLabelId(menuLabelId)
    {
    }
    virtual ~AdjustmentLayer() = default;

    const uint32_t type;
    const char* const commandName;
    const wchar_t* const menuLabelId;

    virtual const wchar_t* Title(LANGID language) const = 0;
    virtual const wchar_t* DefaultLayerName(LANGID language) const { return Title(language); }
    virtual Parameters DefaultParameters() const = 0;
    virtual Parameters NormalizeParameters(const Parameters& parameters) const = 0;
    virtual size_t ParameterWordLimit() const = 0;
    virtual void RenderRow(const PixelRow& row, const Parameters& parameters) const = 0;
    virtual bool BeginSourceObservation(const void*) const { return false; }
    virtual void EndSourceObservation() const {}
    virtual std::unique_ptr<SourceRowObserver> ObserveSource(const void*) const { return {}; }
    virtual bool WantsCanvasSample() const { return false; }
    // Keep the dialog and its preview calls within this synchronous edit.
    virtual bool Edit(HWND owner, Parameters& parameters, const Preview& preview, bool allowCanvasNavigation) const = 0;
};

bool Locate(const uint8_t* image, uint32_t imageSize, uint32_t textRva, uint32_t textSize,
    uint32_t unwindRva, uint32_t unwindSize, Json::Value& entries, std::ostream& log);
bool LocateDialogNavigation(const uint8_t* image, uint32_t imageSize, uint32_t textRva, uint32_t textSize,
    uint32_t unwindRva, uint32_t unwindSize, Json::Value& entries, std::ostream& log);
bool LocateNativeMenu(const uint8_t* image, uint32_t imageSize, uint32_t textRva, uint32_t textSize,
    uint32_t unwindRva, uint32_t unwindSize, Json::Value& entries, std::ostream& log);
bool LocateCommandCatalog(const uint8_t* image, uint32_t imageSize, uint32_t textRva, uint32_t textSize,
    uint32_t unwindRva, uint32_t unwindSize, Json::Value& entries, std::ostream& log);
bool LocateCommandBindings(const uint8_t* image, uint32_t imageSize, uint32_t textRva, uint32_t textSize,
    uint32_t unwindRva, uint32_t unwindSize, Json::Value& entries, std::ostream& log);
// Register definitions with process lifetime before Install(). A null gate allows creation.
using CreationGate = bool (*)();
void Register(const AdjustmentLayer& adjustment, bool supportsCreation = true, CreationGate gate = nullptr);
void Install();
bool TrackCanvasMouse(const MSG& message);
bool CanSampleCanvas();
bool RequestCanvasSample(const MSG& message);
bool ReadCanvasSample(std::optional<SourceColor>& color);
void CancelCanvasSample();
}
