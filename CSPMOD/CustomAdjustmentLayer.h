#pragma once

#include <Windows.h>
#include <cstdint>
#include <functional>
#include <iosfwd>

namespace Json { class Value; }

namespace CustomAdjustmentLayer
{
// CSPMOD's own allocation. The host does not reserve this range for plug-ins.
inline constexpr uint32_t UserStart = 0x43530000;
inline constexpr size_t NativeCorrectionCommandCount = 9;

using Parameters = uint32_t;
using Preview = std::function<void(Parameters)>;

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
    virtual Parameters DefaultParameters() const = 0;
    virtual Parameters NormalizeParameters(Parameters parameters) const = 0;
    virtual void RenderRow(const PixelRow& row, Parameters parameters) const = 0;
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
void Install();
bool PrepareMenu(HWND host);
bool CanEdit(uint32_t type);
void RequestCreate(HWND host, uint32_t type);
void RequestEdit(HWND host, uint32_t type);
}
