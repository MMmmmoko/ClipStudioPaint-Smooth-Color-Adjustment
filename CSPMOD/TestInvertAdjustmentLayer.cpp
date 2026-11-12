#include "TestInvertAdjustmentLayer.h"

#include "DUI/Dialog/TestInvertAdjustmentDialog.h"

namespace TestInvertAdjustment
{
namespace
{
using namespace CustomAdjustmentLayer;

constexpr Parameters Pack(Strengths values)
{
    return 0x01000000u | (uint32_t{values[0]} << 16) | (uint32_t{values[1]} << 8) | values[2];
}

Strengths Unpack(Parameters value)
{
    const Strengths strengths{static_cast<uint8_t>(value >> 16), static_cast<uint8_t>(value >> 8),
        static_cast<uint8_t>(value)};
    return (value >> 24) == 1 && strengths[0] <= 100 && strengths[1] <= 100 && strengths[2] <= 100 ?
        strengths : FullInvert;
}

class TestInvertAdjustmentLayer final : public AdjustmentLayer
{
public:
    TestInvertAdjustmentLayer()
        : AdjustmentLayer(Type, "cspmod.create.testinvert", L"STRID_HELPERMENU_CUSTOMADJUSTMENT")
    {
    }

    const wchar_t* Title(LANGID language) const override
    {
        switch (PRIMARYLANGID(language))
        {
        case LANG_CHINESE: return L"新建试验反相调整图层";
        case LANG_JAPANESE: return L"反転補正レイヤーを新規作成（試験）";
        default: return L"New Test Invert Correction Layer";
        }
    }

    Parameters DefaultParameters() const override { return Pack(FullInvert); }
    Parameters NormalizeParameters(Parameters parameters) const override { return Pack(Unpack(parameters)); }

    void RenderRow(const PixelRow& row, Parameters parameters) const override
    {
        static const auto table = [] {
            std::array<std::array<uint8_t, 256>, 101> values{};
            for (unsigned strength = 0; strength <= 100; ++strength)
                for (unsigned value = 0; value < 256; ++value)
                    values[strength][value] = InvertChannel(static_cast<uint8_t>(value), static_cast<uint8_t>(strength));
            return values;
        }();

        const auto strengths = Unpack(parameters);
        const auto& blue = table[strengths[2]];
        const auto& green = table[strengths[1]];
        const auto& red = table[strengths[0]];
        row.TransformBgr([&](uint8_t* destination, const uint8_t* source) {
            destination[0] = blue[source[0]];
            destination[1] = green[source[1]];
            destination[2] = red[source[2]];
        });
    }

    bool Edit(HWND owner, Parameters& parameters, const Preview& preview, bool allowCanvasNavigation) const override
    {
        auto strengths = Unpack(parameters);
        if (!TestInvertAdjustmentDialog::Show(owner, strengths, [&preview](Strengths next) { preview(Pack(next)); },
                allowCanvasNavigation))
            return false;
        parameters = Pack(strengths);
        return true;
    }
};
}

const CustomAdjustmentLayer::AdjustmentLayer& Definition()
{
    static const TestInvertAdjustmentLayer definition;
    return definition;
}
}
