#include "TestInvertAdjustmentDialog.h"

#include "DUI/CatDui.h"

namespace
{
using TestInvertAdjustment::Strengths;

class Dialog final : public NativeModalWindow
{
public:
    Dialog(Strengths& result, bool& accepted, const std::function<void(Strengths)>& preview)
        : result(result), accepted(accepted), preview(preview), strengths(result)
    {
    }

    DString GetSkinFolder() override { return L"cspHelper_default"; }
    DString GetSkinFile() override { return L"CustomAdjustmentDialog.xml"; }

    void OnInitWindow() override
    {
        NativeModalWindow::OnInitWindow();
        constexpr const wchar_t* sliderNames[] = {L"red", L"green", L"blue"};
        constexpr const wchar_t* valueNames[] = {L"redValue", L"greenValue", L"blueValue"};
        for (size_t channel = 0; channel < sliders.size(); ++channel)
        {
            sliders[channel] = static_cast<ui::Slider*>(FindControl(sliderNames[channel]));
            labels[channel] = static_cast<ui::Label*>(FindControl(valueNames[channel]));
            if (!sliders[channel] || !labels[channel])
            {
                CloseWnd();
                return;
            }
            sliders[channel]->SetValue(strengths[channel]);
            sliders[channel]->AttachValueChanged([this, channel](const ui::EventArgs&) {
                strengths[channel] = static_cast<uint8_t>(sliders[channel]->GetValue());
                UpdatePreview();
                return true;
            });
        }
        for (size_t index = 0; index < swatches.size(); ++index)
            swatches[index] = FindControl(L"sample" + std::to_wstring(index));

        auto* apply = FindControl(L"apply");
        auto* cancel = FindControl(L"cancel");
        auto* reset = FindControl(L"reset");
        auto* close = FindControl(L"close");
        if (!apply || !cancel || !reset || !close)
        {
            CloseWnd();
            return;
        }
        apply->AttachClick([this](const ui::EventArgs&) {
            result = strengths;
            accepted = true;
            CloseWnd();
            return true;
        });
        const auto dismiss = [this](const ui::EventArgs&) {
            CloseWnd();
            return true;
        };
        cancel->AttachClick(dismiss);
        close->AttachClick(dismiss);
        reset->AttachClick([this](const ui::EventArgs&) {
            strengths = TestInvertAdjustment::FullInvert;
            for (size_t channel = 0; channel < sliders.size(); ++channel)
                sliders[channel]->SetValue(strengths[channel]);
            UpdatePreview();
            return true;
        });
        UpdatePreview();
    }

private:
    void UpdatePreview()
    {
        using TestInvertAdjustment::InvertChannel;
        for (size_t channel = 0; channel < labels.size(); ++channel)
            labels[channel]->SetText(std::to_wstring(strengths[channel]) + L"%");

        constexpr Strengths samples[] = {{36, 36, 36}, {210, 57, 46}, {52, 156, 77},
            {51, 109, 200}, {219, 219, 219}};
        for (size_t index = 0; index < swatches.size(); ++index)
        {
            if (swatches[index])
            {
                const auto& sample = samples[index];
                swatches[index]->SetBkColor(ui::UiColor(InvertChannel(sample[0], strengths[0]),
                    InvertChannel(sample[1], strengths[1]), InvertChannel(sample[2], strengths[2])));
            }
        }
        if (preview)
            preview(strengths);
    }

    Strengths& result;
    bool& accepted;
    const std::function<void(Strengths)>& preview;
    Strengths strengths;
    std::array<ui::Slider*, 3> sliders{};
    std::array<ui::Label*, 3> labels{};
    std::array<ui::Control*, 5> swatches{};
};
}

bool TestInvertAdjustmentDialog::Show(HWND parent, Strengths& strengths, const std::function<void(Strengths)>& preview,
    bool allowCanvasNavigation)
{
    bool accepted = false;
    auto* dialog = new Dialog(strengths, accepted, preview);
    dialog->DoModalNative(reinterpret_cast<uintptr_t>(parent), !allowCanvasNavigation);
    return accepted;
}
