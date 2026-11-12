#include "pch.h"
#include "FreeTransform.h"
#include "FreeTransformLocator.h"
#include "AddressTable.h"
#include "CSPMOD.h"
#include "FreeTransformCS.h"

#include <d3d11.h>
#include <intrin.h>
#include <wrl/client.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <vector>

#pragma comment(lib, "d3d11.lib")
#pragma intrinsic(_ReturnAddress)

namespace
{
using Microsoft::WRL::ComPtr;

struct Rect
{
    int left, top, right, bottom;
    int Width() const { return right - left; }
    int Height() const { return bottom - top; }
};

struct SharedImage
{
    void* image;
    void* control;
};

struct SharedCanvas
{
    void* canvas;
    uint8_t* control;
};

using BeginFunc = unsigned(*)(void*, int, void*);
using EndFunc = void(*)(void*, void*, void*, int);
using DragFunc = void(*)(void*, const void*);
using CanvasRefreshFunc = void(*)(SharedCanvas*, const Rect*);
using TransformFunc = int(*)(SharedImage*, SharedImage*, const Rect*, const double*, const Rect*, int, int);
using NearestFunc = void(*)(void*, void*, int, int, int, int, const double*);
using LinearFunc = void(*)(void*, void*, int, int, int, int, const double*, int);
using SelectTileFunc = int(*)(void*, int, int);
using PixelFunc = uint8_t*(*)(void*);
using SizeFunc = int(*)(const void*);

BeginFunc originalBegin = nullptr;
EndFunc originalEnd = nullptr;
DragFunc originalDrag = nullptr;
DragFunc originalDerivedDrag = nullptr;
DragFunc originalMove = nullptr;
DragFunc originalDerivedMove = nullptr;
CanvasRefreshFunc refreshCanvas = nullptr;
TransformFunc originalTransform = nullptr;
NearestFunc originalNearest = nullptr;
NearestFunc originalOversample = nullptr;
LinearFunc originalLinear = nullptr;
SelectTileFunc selectTile = nullptr;
PixelFunc alphaPixels = nullptr;
PixelFunc colorPixels = nullptr;
SizeFunc alphaPitch = nullptr;
SizeFunc colorPitch = nullptr;
SizeFunc alphaBytes = nullptr;
SizeFunc colorBytes = nullptr;
SizeFunc tileWidth = nullptr;
SizeFunc tileHeight = nullptr;

const uint8_t* quadReturn = nullptr;
const uint8_t* toolSlot = nullptr;
uint32_t modeOffset = 0;
uint32_t manipulatorModeOffset = 0;
uint32_t sessionOffset = 0;
std::atomic<bool> enabled{false};
std::atomic<bool> installed{false};
std::atomic<bool> beginPending{false};
std::atomic<const void*> activeSession{nullptr};
std::atomic<bool> freeDragMode{false};

struct DragRefresh
{
    Rect dirty{};
    std::chrono::steady_clock::time_point next{};
    bool refreshing = false;
};

thread_local DragRefresh dragRefresh;
thread_local unsigned derivedDragDepth = 0;

constexpr int maxDimension = 8192;
constexpr int maxCoordinate = 1 << 20;
constexpr int64_t maxPixels = 8 * 1024 * 1024;
constexpr int64_t minOutputPixels = 16 * 1024;

template<typename T>
T Read(const void* base, size_t offset = 0)
{
    T value;
    memcpy(&value, static_cast<const uint8_t*>(base) + offset, sizeof(value));
    return value;
}

// Opt-in measurement. Keep file I/O off the preview path and write on release.
using PreviewClock = std::chrono::steady_clock;
struct PreviewCounts
{
    unsigned refreshes = 0;
    unsigned quadCalls = 0;
    unsigned eligibleCalls = 0;
    unsigned gpuBatches = 0;
    unsigned cpuFallbackTiles = 0;
};

wchar_t previewLogPath[1024]{};
std::mutex previewMeasurementMutex;
std::atomic<const void*> measuredSession{nullptr};
PreviewClock::time_point measurementStart;
const char* measuredGesture = nullptr;
unsigned measurementId = 0;
PreviewCounts measurementTotal;
enum class PreviewFailure { DestinationFormat, DestinationPlanes, OutputSize, SamplerLayout, SourceBounds,
    SourcePlanes, Device, Compute, Allocation, OutputMismatch, Quality, Count };
constexpr const char* previewFailureNames[] = {"destination_format", "destination_planes", "output_size",
    "sampler_layout", "source_bounds", "source_planes", "device", "compute", "allocation", "output_mismatch", "quality"};
static_assert(std::size(previewFailureNames) == static_cast<size_t>(PreviewFailure::Count));
std::array<unsigned, std::size(previewFailureNames)> measurementFailures;
struct SamplerLayout
{
    int boundsMode, flags, channels, defaultAlpha, destinationBytes;
};
SamplerLayout rejectedSampler{};
PreviewClock::duration totalRefreshTime;
PreviewClock::duration longestRefresh;
std::array<PreviewCounts, 60> measurementSeconds;

void StartPreviewMeasurement(const void* session, const char* gesture, PreviewClock::time_point now)
{
    if (!previewLogPath[0] || measuredSession.load())
        return;
    std::lock_guard lock(previewMeasurementMutex);
    if (measuredSession.load())
        return;
    measuredGesture = gesture;
    measurementStart = now;
    measurementTotal = {};
    totalRefreshTime = {};
    longestRefresh = {};
    measurementFailures.fill(0);
    rejectedSampler = {};
    measurementSeconds.fill({});
    measuredSession.store(session);
}

void CountPreview(const void* session, unsigned PreviewCounts::*counter, unsigned count = 1,
    PreviewClock::duration refreshTime = {})
{
    if (!session || measuredSession.load() != session)
        return;
    std::lock_guard lock(previewMeasurementMutex);
    if (measuredSession.load() != session)
        return;
    const auto elapsed = PreviewClock::now() - measurementStart;
    const auto second = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
    measurementTotal.*counter += count;
    totalRefreshTime += refreshTime;
    longestRefresh = (std::max)(longestRefresh, refreshTime);
    if (second >= 0 && static_cast<size_t>(second) < measurementSeconds.size())
        measurementSeconds[static_cast<size_t>(second)].*counter += count;
}

void CountPreviewFailure(const void* session, PreviewFailure reason, const SamplerLayout* sampler = nullptr)
{
    if (measuredSession.load() != session)
        return;
    std::lock_guard lock(previewMeasurementMutex);
    if (measuredSession.load() == session)
    {
        ++measurementFailures[static_cast<size_t>(reason)];
        if (sampler)
            rejectedSampler = *sampler;
    }
}

void FinishPreviewMeasurement()
{
    if (!measuredSession.load())
        return;
    const auto end = PreviewClock::now();
    std::lock_guard lock(previewMeasurementMutex);
    if (!measuredSession.exchange(nullptr))
        return;
    const double seconds = std::chrono::duration<double>(end - measurementStart).count();
    if (seconds <= 0)
        return;
    FILE* log = nullptr;
    if (_wfopen_s(&log, previewLogPath, L"a") != 0)
    {
        SDL_Log("Could not write the Free Transform preview measurement.");
        return;
    }
    const unsigned id = ++measurementId;
    fprintf(log, "drag=%u gesture=%s elapsed=%.6f refreshes=%u gpu_batches=%u quad_calls=%u eligible_calls=%u "
        "cpu_fallback_tiles=%u refresh_total_ms=%.2f refresh_max_ms=%.2f\n",
        id, measuredGesture, seconds, measurementTotal.refreshes, measurementTotal.gpuBatches,
        measurementTotal.quadCalls, measurementTotal.eligibleCalls, measurementTotal.cpuFallbackTiles,
        std::chrono::duration<double, std::milli>(totalRefreshTime).count(),
        std::chrono::duration<double, std::milli>(longestRefresh).count());
    for (size_t i = 0; i < measurementFailures.size(); ++i)
        if (measurementFailures[i])
            fprintf(log, "drag=%u fallback=%s count=%u\n", id, previewFailureNames[i], measurementFailures[i]);
    if (measurementFailures[static_cast<size_t>(PreviewFailure::SamplerLayout)])
        fprintf(log, "drag=%u sampler_bounds_mode=%d sampler_flags=%d sampler_channels=%d sampler_default_alpha=%d "
            "destination_bytes=%d\n", id, rejectedSampler.boundsMode, rejectedSampler.flags, rejectedSampler.channels,
            rejectedSampler.defaultAlpha, rejectedSampler.destinationBytes);
    for (size_t i = 0; i < measurementSeconds.size() && static_cast<double>(i) < seconds; ++i)
    {
        const auto& sample = measurementSeconds[i];
        fprintf(log, "drag=%u second=%zu elapsed=%.6f refreshes=%u gpu_batches=%u quad_calls=%u eligible_calls=%u "
            "cpu_fallback_tiles=%u\n", id, i + 1, (std::min)(1.0, seconds - static_cast<double>(i)),
            sample.refreshes, sample.gpuBatches, sample.quadCalls, sample.eligibleCalls, sample.cpuFallbackTiles);
    }
    fclose(log);
}

void CheckPreviewDragEnd(const void* event, bool move)
{
    if (measuredSession.load() && event && derivedDragDepth == 0 &&
        (Read<int>(event, move ? 0x28 : 0x68) != 0 || !(GetAsyncKeyState(VK_LBUTTON) & 0x8000)))
        FinishPreviewMeasurement();
}

template<typename T>
bool TryRead(const void* base, size_t offset, T& value)
{
    SIZE_T bytesRead = 0;
    return ReadProcessMemory(GetCurrentProcess(), static_cast<const uint8_t*>(base) + offset,
        &value, sizeof(value), &bytesRead) && bytesRead == sizeof(value);
}

const void* GetSession(int* currentMode = nullptr)
{
    const uint8_t* tool = Read<const uint8_t*>(toolSlot);
    const void* session = nullptr;
    int mode = 0;
    if (tool && TryRead(tool, modeOffset, mode))
        TryRead(tool, sessionOffset, session);
    if (currentMode)
        *currentMode = mode;
    return session;
}

bool ValidRect(const Rect& rect, int64_t minimumPixels = 1)
{
    if (rect.left < -maxCoordinate || rect.top < -maxCoordinate ||
        rect.right > maxCoordinate || rect.bottom > maxCoordinate ||
        rect.right <= rect.left || rect.bottom <= rect.top)
        return false;

    const int64_t area = static_cast<int64_t>(rect.Width()) * rect.Height();
    return rect.Width() <= maxDimension && rect.Height() <= maxDimension &&
        area >= minimumPixels && area <= maxPixels;
}

Rect Intersect(const Rect& left, const Rect& right)
{
    return {(std::max)(left.left, right.left), (std::max)(left.top, right.top),
        (std::min)(left.right, right.right), (std::min)(left.bottom, right.bottom)};
}

bool Contains(const Rect& outer, const Rect& inner)
{
    return outer.left <= inner.left && outer.top <= inner.top &&
        outer.right >= inner.right && outer.bottom >= inner.bottom;
}

struct Pixel
{
    uint32_t color;
    uint32_t alpha;
};

static_assert(sizeof(Pixel) == 8);

struct TransformConstants
{
    float sourceX[4]{};
    float sourceY[4]{};
    float denominator[4]{};
    int sourceOrigin[2]{};
    unsigned sourceSize[2]{};
    unsigned outputSize[2]{};
    unsigned bilinear = 0;
    unsigned padding = 0;
};

static_assert(sizeof(TransformConstants) == 80);

bool SourceBounds(const Rect& source, const Rect& output, const double* matrix,
    Rect& input, TransformConstants& constants)
{
    double minX = maxCoordinate, maxX = -maxCoordinate;
    double minY = maxCoordinate, maxY = -maxCoordinate;
    int sign = 0;
    const int xs[] = {output.left, output.right - 1};
    const int ys[] = {output.top, output.bottom - 1};
    for (const int x : xs)
    {
        for (const int y : ys)
        {
            const double w = matrix[6] * x + matrix[7] * y + 1;
            if (!std::isfinite(w) || std::abs(w) < 1e-3 || (sign && sign != (w > 0 ? 1 : -1)))
                return false;
            sign = w > 0 ? 1 : -1;
            const double u = (matrix[0] * x + matrix[1] * y + matrix[2]) / w;
            const double v = (matrix[3] * x + matrix[4] * y + matrix[5]) / w;
            if (!std::isfinite(u) || !std::isfinite(v) ||
                std::abs(u) >= maxCoordinate - 4 || std::abs(v) >= maxCoordinate - 4)
                return false;
            minX = (std::min)(minX, u);
            minY = (std::min)(minY, v);
            maxX = (std::max)(maxX, u);
            maxY = (std::max)(maxY, v);
        }
    }
    input = Intersect(source, {static_cast<int>(std::floor(minX)) - 1,
        static_cast<int>(std::floor(minY)) - 1, static_cast<int>(std::ceil(maxX)) + 2,
        static_cast<int>(std::ceil(maxY)) + 2});
    if (!ValidRect(input))
        return false;

    const auto setRow = [&output](float* row, double x, double y, double offset) {
        row[0] = static_cast<float>(x);
        row[1] = static_cast<float>(y);
        row[2] = static_cast<float>(x * output.left + y * output.top + offset);
        return std::isfinite(row[0]) && std::isfinite(row[1]) && std::isfinite(row[2]);
    };
    if (!setRow(constants.sourceX, matrix[0], matrix[1], matrix[2]) ||
        !setRow(constants.sourceY, matrix[3], matrix[4], matrix[5]) ||
        !setRow(constants.denominator, matrix[6], matrix[7], 1))
        return false;
    constants.sourceOrigin[0] = input.left;
    constants.sourceOrigin[1] = input.top;
    constants.sourceSize[0] = static_cast<unsigned>(input.Width());
    constants.sourceSize[1] = static_cast<unsigned>(input.Height());
    constants.outputSize[0] = static_cast<unsigned>(output.Width());
    constants.outputSize[1] = static_cast<unsigned>(output.Height());
    return true;
}

bool BufferRange(const void* data, size_t size, bool writable)
{
    if (!data || !size)
        return false;
    MEMORY_BASIC_INFORMATION region{};
    if (!VirtualQuery(data, &region, sizeof(region)) || region.State != MEM_COMMIT ||
        (region.Protect & (PAGE_GUARD | PAGE_NOACCESS)))
        return false;

    const DWORD protection = region.Protect & 0xff;
    if (writable && protection != PAGE_READWRITE && protection != PAGE_WRITECOPY &&
        protection != PAGE_EXECUTE_READWRITE && protection != PAGE_EXECUTE_WRITECOPY)
        return false;

    const auto offset = reinterpret_cast<uintptr_t>(data) - reinterpret_cast<uintptr_t>(region.BaseAddress);
    return offset <= region.RegionSize && size <= region.RegionSize - offset;
}

bool Planes(void* view, int width, int height, int colorSize, bool writable,
    uint8_t*& alpha, uint8_t*& color, int& aPitch, int& cPitch)
{
    alpha = alphaPixels(view);
    color = colorPixels(view);
    aPitch = alphaPitch(view);
    cPitch = colorPitch(view);
    return aPitch >= width && cPitch >= colorSize * width &&
        BufferRange(alpha, static_cast<size_t>(height - 1) * aPitch + width, writable) &&
        BufferRange(color, static_cast<size_t>(height - 1) * cPitch + colorSize * width, writable);
}

bool ReadSource(void* view, const Rect& bounds, int colorSize, std::vector<Pixel>& pixels)
{
    const void* image = Read<const void*>(view, 8);
    if (!image || bounds.left < 0 || bounds.top < 0)
        return false;
    const int stepX = tileWidth(image), stepY = tileHeight(image);
    if (stepX <= 0 || stepY <= 0 || stepX > maxDimension || stepY > maxDimension)
        return false;

    pixels.assign(static_cast<size_t>(bounds.Width()) * bounds.Height(), {0xffffffffu, 0});
    for (int y = bounds.top; y < bounds.bottom;)
    {
        const int bottom = (std::min)(bounds.bottom, y + stepY - y % stepY);
        for (int x = bounds.left; x < bounds.right;)
        {
            const int right = (std::min)(bounds.right, x + stepX - x % stepX);
            const int width = right - x, height = bottom - y;
            selectTile(view, x, y);
            if (alphaPixels(view))
            {
                uint8_t* alpha;
                uint8_t* color;
                int aPitch, cPitch;
                const Rect tile = Read<Rect>(view, 0x70);
                if (alphaBytes(view) != 1 || colorBytes(view) != colorSize ||
                    !Contains(tile, {x, y, right, bottom}) ||
                    !Planes(view, width, height, colorSize, false, alpha, color, aPitch, cPitch))
                    return false;

                for (int row = 0; row < height; ++row)
                {
                    auto* output = pixels.data() + static_cast<size_t>(y + row - bounds.top) *
                        bounds.Width() + (x - bounds.left);
                    for (int col = 0; col < width; ++col)
                    {
                        const auto* source = color + colorSize * col;
                        uint32_t packed = 0;
                        if (colorSize == 4)
                            memcpy(&packed, source, sizeof(packed));
                        else
                            packed = source[0] | (static_cast<uint32_t>(source[1]) << 8) |
                                (static_cast<uint32_t>(source[2]) << 16);
                        output[col] = {packed, alpha[col]};
                    }
                    alpha += aPitch;
                    color += cPitch;
                }
            }
            x = right;
        }
        y = bottom;
    }
    return true;
}

class Compute
{
public:
    bool Available()
    {
        std::call_once(initialization, [this] { ready = Initialize(); });
        return ready && SUCCEEDED(device->GetDeviceRemovedReason());
    }

    bool Render(const TransformConstants& parameters, const std::vector<Pixel>& source,
        std::vector<Pixel>& result)
    {
        if (!Available())
            return false;

        std::unique_lock<std::timed_mutex> lock(mutex, std::defer_lock);
        if (!lock.try_lock_for(std::chrono::milliseconds(2)))
            return false;
        if (!Textures(parameters))
            return false;

        const D3D11_BOX sourceBox{0, 0, 0, parameters.sourceSize[0], parameters.sourceSize[1], 1};
        context->UpdateSubresource(input.Get(), 0, &sourceBox, source.data(), parameters.sourceSize[0] * sizeof(Pixel), 0);
        context->UpdateSubresource(constants.Get(), 0, nullptr, &parameters, 0, 0);
        context->CSSetShader(shader.Get(), nullptr, 0);
        context->CSSetConstantBuffers(0, 1, constants.GetAddressOf());
        context->CSSetShaderResources(0, 1, sourceView.GetAddressOf());
        context->CSSetUnorderedAccessViews(0, 1, outputView.GetAddressOf(), nullptr);
        context->Dispatch((parameters.outputSize[0] + 15) / 16, (parameters.outputSize[1] + 15) / 16, 1);
        ID3D11UnorderedAccessView* noOutput = nullptr;
        context->CSSetUnorderedAccessViews(0, 1, &noOutput, nullptr);
        const D3D11_BOX outputBox{0, 0, 0, parameters.outputSize[0], parameters.outputSize[1], 1};
        context->CopySubresourceRegion(staging.Get(), 0, 0, 0, 0, output.Get(), 0, &outputBox);

        const size_t width = parameters.outputSize[0], height = parameters.outputSize[1];
        result.resize(width * height);
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (FAILED(context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped)))
            return false;
        const auto* row = static_cast<const uint8_t*>(mapped.pData);
        for (size_t y = 0; y < height; ++y)
            memcpy(result.data() + y * width, row + y * mapped.RowPitch, width * sizeof(Pixel));
        context->Unmap(staging.Get(), 0);
        return true;
    }

private:
    bool Initialize()
    {
        const D3D_FEATURE_LEVEL required = D3D_FEATURE_LEVEL_11_0;
        if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, &required, 1,
            D3D11_SDK_VERSION, device.GetAddressOf(), nullptr, context.GetAddressOf())))
        {
            SDL_Log("Free Transform GPU preview: D3D11 device unavailable.");
            return false;
        }
        D3D11_BUFFER_DESC desc{};
        desc.ByteWidth = sizeof(TransformConstants);
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        if (FAILED(device->CreateBuffer(&desc, nullptr, constants.GetAddressOf())) ||
            FAILED(device->CreateComputeShader(FreeTransformCS, sizeof(FreeTransformCS), nullptr, shader.GetAddressOf())))
        {
            SDL_Log("Free Transform GPU preview: compute shader unavailable.");
            return false;
        }
        return true;
    }

    bool Textures(const TransformConstants& parameters)
    {
        const auto dimension = [](unsigned length) {
            unsigned rounded = 64;
            while (rounded < length)
                rounded *= 2;
            return rounded;
        };
        D3D11_TEXTURE2D_DESC desc{};
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R32G32_UINT;
        desc.SampleDesc.Count = 1;
        desc.Width = dimension(parameters.sourceSize[0]);
        desc.Height = dimension(parameters.sourceSize[1]);
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        if (!sourceView || inputWidth != desc.Width || inputHeight != desc.Height)
        {
            ComPtr<ID3D11Texture2D> texture;
            ComPtr<ID3D11ShaderResourceView> view;
            if (FAILED(device->CreateTexture2D(&desc, nullptr, texture.GetAddressOf())) ||
                FAILED(device->CreateShaderResourceView(texture.Get(), nullptr, view.GetAddressOf())))
                return false;
            input = std::move(texture);
            sourceView = std::move(view);
            inputWidth = desc.Width;
            inputHeight = desc.Height;
        }

        desc.Width = dimension(parameters.outputSize[0]);
        desc.Height = dimension(parameters.outputSize[1]);
        desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
        if (!outputView || outputWidth != desc.Width || outputHeight != desc.Height)
        {
            ComPtr<ID3D11Texture2D> texture;
            ComPtr<ID3D11Texture2D> readback;
            ComPtr<ID3D11UnorderedAccessView> view;
            if (FAILED(device->CreateTexture2D(&desc, nullptr, texture.GetAddressOf())) ||
                FAILED(device->CreateUnorderedAccessView(texture.Get(), nullptr, view.GetAddressOf())))
                return false;
            desc.BindFlags = 0;
            desc.Usage = D3D11_USAGE_STAGING;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
            if (FAILED(device->CreateTexture2D(&desc, nullptr, readback.GetAddressOf())))
                return false;
            output = std::move(texture);
            staging = std::move(readback);
            outputView = std::move(view);
            outputWidth = desc.Width;
            outputHeight = desc.Height;
        }
        return true;
    }

    std::timed_mutex mutex;
    std::once_flag initialization;
    bool ready = false;
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    ComPtr<ID3D11ComputeShader> shader;
    ComPtr<ID3D11Buffer> constants;
    ComPtr<ID3D11Texture2D> input, output, staging;
    ComPtr<ID3D11ShaderResourceView> sourceView;
    ComPtr<ID3D11UnorderedAccessView> outputView;
    unsigned inputWidth = 0, inputHeight = 0, outputWidth = 0, outputHeight = 0;
};

Compute& PreviewDevice()
{
    static Compute gpu;
    return gpu;
}

struct Preview
{
    const void* session = nullptr;
    Rect output{};
    void* nativeContext = nullptr;
    bool attempted = false;
    bool rendered = false;
    bool bilinear = false;
    bool reducedQuality = false;
    int colorSize = 0;
    int writtenColorBytes = 0;
    unsigned cpuFallbackTiles = 0;
    std::vector<Pixel> pixels;
};

thread_local Preview* preview = nullptr;

class PreviewScope
{
public:
    explicit PreviewScope(Preview* next) : previous(preview) { preview = next; }
    ~PreviewScope() { preview = previous; }
private:
    Preview* previous;
};

bool PreparePreview(Preview& frame, void* state, const double* matrix, bool bilinear)
{
    const auto fail = [&](PreviewFailure reason) {
        CountPreviewFailure(frame.session, reason);
        return false;
    };
    // Layout checked by the sampler constructor and pixel-view call signatures.
    const SamplerLayout layout{Read<int>(state, 0x288), Read<int>(state, 0x28c), Read<int>(state, 0x294),
        Read<int>(state, 0x298), frame.colorSize};
    const bool paddedColor = bilinear && layout.channels == 3 && frame.colorSize == 4 &&
        manipulatorModeOffset == FreeTransform::earlierManipulatorModeOffset;
    if (layout.boundsMode != 1 || layout.flags != 0 || (layout.channels != 3 && layout.channels != 4) ||
        (bilinear && layout.channels != frame.colorSize && !paddedColor) || layout.defaultAlpha != 0)
    {
        CountPreviewFailure(frame.session, PreviewFailure::SamplerLayout, &layout);
        return false;
    }

    auto* sourceView = static_cast<uint8_t*>(state) + 8;
    const int sourceColorBytes = paddedColor ? colorBytes(sourceView) : frame.colorSize;
    if (sourceColorBytes != 3 && sourceColorBytes != 4)
        return fail(PreviewFailure::SourcePlanes);
    frame.writtenColorBytes = paddedColor ? layout.channels : frame.colorSize;
    const Rect clip = Intersect(Read<Rect>(state, 0x278), Read<Rect>(sourceView, 0x28));
    Rect input{};
    TransformConstants constants{};
    if (!SourceBounds(clip, frame.output, matrix, input, constants))
        return fail(PreviewFailure::SourceBounds);
    auto& gpu = PreviewDevice();
    if (!gpu.Available())
        return fail(PreviewFailure::Device);

    constants.bilinear = bilinear ? 1u : 0u;
    std::vector<Pixel> source;
    if (!ReadSource(sourceView, input, sourceColorBytes, source))
        return fail(PreviewFailure::SourcePlanes);
    return gpu.Render(constants, source, frame.pixels) || fail(PreviewFailure::Compute);
}

bool TryBlit(void* state, void* destination, int x, int y, int width, int height,
    const double* matrix, bool bilinear)
{
    if (!enabled || !preview || preview->session != activeSession.load() ||
        !destination || !state || !matrix || width <= 0 || height <= 0 ||
        width > maxDimension || height > maxDimension || x < -maxCoordinate || x > maxCoordinate ||
        y < -maxCoordinate || y > maxCoordinate)
        return false;
    const Rect tile{x, y, x + width, y + height};
    auto& frame = *preview;
    const auto fail = [&](PreviewFailure reason) {
        CountPreviewFailure(frame.session, reason);
        return false;
    };
    const int colorSize = colorBytes(destination);
    if (alphaBytes(destination) != 1 || (colorSize != 3 && colorSize != 4))
        return fail(PreviewFailure::DestinationFormat);

    uint8_t* alpha;
    uint8_t* color;
    int aPitch, cPitch;
    if (!Planes(destination, width, height, colorSize, true, alpha, color, aPitch, cPitch))
        return fail(PreviewFailure::DestinationPlanes);

    if (!frame.attempted)
    {
        // The native destination view already contains the clipped output region.
        frame.output = Read<Rect>(destination, 0x28);
        if (!ValidRect(frame.output, minOutputPixels) || !Contains(frame.output, tile))
            frame.output = tile;
        if (!ValidRect(frame.output, minOutputPixels))
            return fail(PreviewFailure::OutputSize);
        frame.attempted = true;
        frame.colorSize = colorSize;
        frame.nativeContext = state;
        frame.bilinear = bilinear;
        try
        {
            frame.rendered = PreparePreview(frame, state, matrix, bilinear);
        }
        catch (const std::bad_alloc&)
        {
            frame.rendered = false;
            CountPreviewFailure(frame.session, PreviewFailure::Allocation);
        }
        if (frame.rendered)
        {
            CountPreview(frame.session, &PreviewCounts::gpuBatches);
            static std::atomic<bool> logged{false};
            if (!logged.exchange(true))
                SDL_Log("Free Transform GPU preview: rendered native raster tiles with D3D11.");
        }
    }
    if (!frame.rendered)
        return false;
    if (frame.nativeContext != state || frame.bilinear != bilinear ||
        frame.colorSize != colorSize || !Contains(frame.output, tile) ||
        !enabled || frame.session != activeSession.load())
        return fail(PreviewFailure::OutputMismatch);

    for (int row = 0; row < height; ++row)
    {
        const auto* source = frame.pixels.data() + static_cast<size_t>(y - frame.output.top + row) *
            frame.output.Width() + (x - frame.output.left);
        for (int col = 0; col < width; ++col)
        {
            const auto& pixel = source[col];
            auto* output = color + colorSize * col;
            if (frame.writtenColorBytes == 4)
                memcpy(output, &pixel.color, sizeof(pixel.color));
            else
            {
                output[0] = static_cast<uint8_t>(pixel.color);
                output[1] = static_cast<uint8_t>(pixel.color >> 8);
                output[2] = static_cast<uint8_t>(pixel.color >> 16);
            }
            alpha[col] = static_cast<uint8_t>(pixel.alpha);
        }
        alpha += aPitch;
        color += cPitch;
    }
    return true;
}

unsigned BeginHook(void* canvas, int mode, void* event)
{
    const unsigned result = originalBegin(canvas, mode, event);
    if (installed && result)
    {
        const auto session = GetSession();
        dragRefresh = {};
        beginPending.store(!session);
        activeSession.store(session);
        freeDragMode.store(false);
        if (enabled && mode == 4)
            PreviewDevice().Available();
    }
    return result;
}

void EndHook(void* document, void* canvas, void* data, int cancel)
{
    FinishPreviewMeasurement();
    activeSession.store(nullptr);
    beginPending.store(false);
    freeDragMode.store(false);
    originalEnd(document, canvas, data, cancel);
}

void RequestDragRefresh(const void* session, void* manipulator, const void* event, bool move)
{
    if (!enabled || !event || dragRefresh.refreshing)
        return;
    int mode = 0;
    const auto currentSession = GetSession(&mode);
    if (!session && currentSession && beginPending.exchange(false))
    {
        session = currentSession;
        activeSession.store(session);
    }
    if (!session || session != activeSession.load() || session != currentSession)
        return;
    // The tool may open in its generic mode before the palette switches its manipulator
    // to Free Transform. Scope the preview to the current tool or manipulator mode.
    const auto manipulatorMode = Read<int>(manipulator, manipulatorModeOffset);
    const bool isFree = mode == 4 || manipulatorMode == 4;
    freeDragMode.store(isFree);
    const bool earlierLayout = manipulatorModeOffset == FreeTransform::earlierManipulatorModeOffset;
    const bool held = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    if (Read<int>(event, move ? 0x28 : 0x68) != 0 || (earlierLayout && !held))
    {
        if (earlierLayout)
            dragRefresh = {};
        return;
    }
    // The native scale branch handles actions 5 through 12, including corner
    // drags with the opposite corner fixed. It uses the common transform modes.
    const int handle = move ? 0 : Read<int>(event, 0x30);
    const bool moving = move && mode == 1 && manipulatorMode == 1 && held;
    const bool rotating = !move && handle == 4 && held && (mode == 1 || mode == 3) &&
        (manipulatorMode == 1 || manipulatorMode == 3);
    const bool scaling = !move && handle >= 5 && handle <= 12 && held &&
        (mode == 1 || mode == 2) && (manipulatorMode == 1 || manipulatorMode == 2);
    if (!isFree && !moving && !rotating && !scaling)
        return;
    const auto dirty = Read<Rect>(event, move ? 0x30 : 0x70);
    if (dirty.left < -maxCoordinate || dirty.top < -maxCoordinate ||
        dirty.right > maxCoordinate || dirty.bottom > maxCoordinate ||
        dirty.left >= dirty.right || dirty.top >= dirty.bottom)
        return;
    auto canvas = Read<SharedCanvas>(event);
    if (!canvas.canvas || !canvas.control)
        return;

    auto& pending = dragRefresh.dirty;
    pending = pending.left == pending.right ? dirty :
        Rect{(std::min)(pending.left, dirty.left), (std::min)(pending.top, dirty.top),
             (std::max)(pending.right, dirty.right), (std::max)(pending.bottom, dirty.bottom)};
    const auto now = std::chrono::steady_clock::now();
    if (held)
        StartPreviewMeasurement(session, move ? "move" : (handle == 4 ? "rotate" : (isFree ? "quad" : "scale")), now);
    if (now < dragRefresh.next)
        return;

    const auto region = pending;
    pending = {};
    const auto minimumInterval = std::chrono::milliseconds(earlierLayout ? 33 : 16);
    dragRefresh.next = now + minimumInterval;
    dragRefresh.refreshing = true;
    // The release path takes a by-value native shared pointer and consumes its reference.
    InterlockedIncrement(reinterpret_cast<volatile LONG*>(canvas.control + 8));
    refreshCanvas(&canvas, &region);
    dragRefresh.refreshing = false;
    if (earlierLayout || previewLogPath[0])
    {
        const auto finished = PreviewClock::now();
        // Leave time for pointer events even when the synchronous refresh took a full interval.
        if (earlierLayout)
            dragRefresh.next = (std::max)(dragRefresh.next, finished + std::chrono::milliseconds(8));
        CountPreview(session, &PreviewCounts::refreshes, 1, finished - now);
    }
}

void DragHook(void* manipulator, const void* event)
{
    CheckPreviewDragEnd(event, false);
    const auto session = activeSession.load();
    originalDrag(manipulator, event);
    if (derivedDragDepth == 0)
        RequestDragRefresh(session, manipulator, event, false);
}

void DerivedDragHook(void* manipulator, const void* event)
{
    CheckPreviewDragEnd(event, false);
    const auto session = activeSession.load();
    ++derivedDragDepth;
    originalDerivedDrag(manipulator, event);
    --derivedDragDepth;
    RequestDragRefresh(session, manipulator, event, false);
}

void MoveHook(void* manipulator, const void* event)
{
    CheckPreviewDragEnd(event, true);
    const auto session = activeSession.load();
    originalMove(manipulator, event);
    if (derivedDragDepth == 0)
        RequestDragRefresh(session, manipulator, event, true);
}

void DerivedMoveHook(void* manipulator, const void* event)
{
    CheckPreviewDragEnd(event, true);
    const auto session = activeSession.load();
    ++derivedDragDepth;
    originalDerivedMove(manipulator, event);
    --derivedDragDepth;
    RequestDragRefresh(session, manipulator, event, true);
}

int TransformHook(SharedImage* destination, SharedImage* source, const Rect* sourceClip,
    const double* quad, const Rect* outputClip, int quality, int flags)
{
    Preview frame;
    const auto session = activeSession.load();
    const bool fromQuad = _ReturnAddress() == quadReturn;
    if (fromQuad)
        CountPreview(session, &PreviewCounts::quadCalls);
    int mode = 0;
    if (enabled && session && fromQuad && flags == 0 && quality >= 0 && quality <= 3 &&
        destination && source && destination->image != source->image && quad && outputClip &&
        session == GetSession(&mode) && (mode == 4 || freeDragMode.load()))
    {
        // For expensive interpolation modes, use bilinear while dragging. The native
        // quality is still passed to the dispatcher and retained on every CPU fallback.
        frame.reducedQuality = quality >= 2 && (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
        if (quality <= 1 || frame.reducedQuality)
            frame.session = session;
    }

    const bool measure = frame.session && measuredSession.load() == frame.session;
    if (measure)
        CountPreview(session, &PreviewCounts::eligibleCalls);
    PreviewScope scope(frame.session ? &frame : nullptr);
    const int result = originalTransform(destination, source, sourceClip, quad, outputClip, quality, flags);
    if (measure && frame.cpuFallbackTiles)
        CountPreview(session, &PreviewCounts::cpuFallbackTiles, frame.cpuFallbackTiles);
    return result;
}

void NearestHook(void* state, void* destination, int x, int y, int width, int height, const double* matrix)
{
    if (!TryBlit(state, destination, x, y, width, height, matrix, false))
    {
        if (preview)
            ++preview->cpuFallbackTiles;
        originalNearest(state, destination, x, y, width, height, matrix);
    }
}

void LinearHook(void* state, void* destination, int x, int y, int width, int height,
    const double* matrix, int quality)
{
    const bool previewQuality = quality == 1 || (quality == 2 && preview && preview->reducedQuality);
    if (!previewQuality || !TryBlit(state, destination, x, y, width, height, matrix, true))
    {
        if (preview)
        {
            ++preview->cpuFallbackTiles;
            if (!previewQuality)
                CountPreviewFailure(preview->session, PreviewFailure::Quality);
        }
        originalLinear(state, destination, x, y, width, height, matrix, quality);
    }
}

void OversampleHook(void* state, void* destination, int x, int y, int width, int height, const double* matrix)
{
    if (!preview || !preview->reducedQuality || !TryBlit(state, destination, x, y, width, height, matrix, true))
    {
        if (preview)
            ++preview->cpuFallbackTiles;
        originalOversample(state, destination, x, y, width, height, matrix);
    }
}
}

void FreeTransform::Install()
{
    const DWORD logPathLength = GetEnvironmentVariableW(L"CSPMOD_PREVIEW_STATS",
        previewLogPath, _countof(previewLogPath));
    if (!logPathLength || logPathLength >= _countof(previewLogPath))
        previewLogPath[0] = L'\0';

    const char* features[] = {Begin, End, Drag, DerivedDrag, Move, DerivedMove, CanvasRefresh,
        Wrapper, QuadCall, Nearest, Bilinear, Oversample, ToolSlot,
        ToolModeInstruction, ToolSessionInstruction, ManipulatorModeInstruction, SelectTile, Alpha, Color, AlphaPitch,
        ColorPitch, AlphaBytes, ColorBytes, TileWidth, TileHeight};
    for (const auto* key : features)
    {
        if (!AddressTable::GetAddress(key))
        {
            SDL_Log("Free Transform GPU preview disabled: %s unavailable.", key);
            return;
        }
    }

    quadReturn = static_cast<const uint8_t*>(AddressTable::GetAddress(QuadCall)) + 5;
    toolSlot = static_cast<const uint8_t*>(AddressTable::GetAddress(ToolSlot));
    modeOffset = Read<uint32_t>(AddressTable::GetAddress(ToolModeInstruction), 9);
    manipulatorModeOffset = Read<uint32_t>(AddressTable::GetAddress(ManipulatorModeInstruction), 2);
    sessionOffset = Read<uint32_t>(AddressTable::GetAddress(ToolSessionInstruction), 19);
    refreshCanvas = reinterpret_cast<CanvasRefreshFunc>(AddressTable::GetAddress(CanvasRefresh));
    selectTile = reinterpret_cast<SelectTileFunc>(AddressTable::GetAddress(SelectTile));
    alphaPixels = reinterpret_cast<PixelFunc>(AddressTable::GetAddress(Alpha));
    colorPixels = reinterpret_cast<PixelFunc>(AddressTable::GetAddress(Color));
    alphaPitch = reinterpret_cast<SizeFunc>(AddressTable::GetAddress(AlphaPitch));
    colorPitch = reinterpret_cast<SizeFunc>(AddressTable::GetAddress(ColorPitch));
    alphaBytes = reinterpret_cast<SizeFunc>(AddressTable::GetAddress(AlphaBytes));
    colorBytes = reinterpret_cast<SizeFunc>(AddressTable::GetAddress(ColorBytes));
    tileWidth = reinterpret_cast<SizeFunc>(AddressTable::GetAddress(TileWidth));
    tileHeight = reinterpret_cast<SizeFunc>(AddressTable::GetAddress(TileHeight));

    const auto hook = [](const char* key, auto replacement, auto& original) {
        if (CSPMOD::Hook(AddressTable::GetAddress(key), reinterpret_cast<void*>(replacement),
            reinterpret_cast<void**>(&original), CSPMOD::_HOOKREG_R11))
            return true;
        SDL_Log("Free Transform GPU preview disabled: %s hook failed.", key);
        return false;
    };
    installed = hook(End, EndHook, originalEnd) &&
        hook(Drag, DragHook, originalDrag) &&
        hook(DerivedDrag, DerivedDragHook, originalDerivedDrag) &&
        hook(Move, MoveHook, originalMove) &&
        hook(DerivedMove, DerivedMoveHook, originalDerivedMove) &&
        hook(Wrapper, TransformHook, originalTransform) &&
        hook(Nearest, NearestHook, originalNearest) &&
        hook(Bilinear, LinearHook, originalLinear) &&
        hook(Oversample, OversampleHook, originalOversample) &&
        hook(Begin, BeginHook, originalBegin);
}

bool FreeTransform::Available()
{
    return installed;
}

void FreeTransform::SetEnabled(bool value)
{
    enabled = installed && value;
}
