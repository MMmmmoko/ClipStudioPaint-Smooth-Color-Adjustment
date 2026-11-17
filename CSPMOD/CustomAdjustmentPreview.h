#pragma once

#include "CustomAdjustmentLayer.h"

#include <atomic>
#include <cstring>
#include <mutex>
#include <utility>

namespace CustomAdjustmentLayer::ParallelPreview
{
inline constexpr int TileSide = 256;
inline constexpr size_t TileLimit = 16;
inline constexpr size_t WorkerLimit = 4;

using BgrRow = std::array<uint8_t, TileSide * 3>;

struct InputRow
{
    BgrRow bgr{};
    std::array<uint8_t, TileSide> alpha{};
    bool fullColor = false;

    void Save(const PixelRow& row)
    {
        if (row.alphaStride == 1)
            std::memcpy(alpha.data(), row.alpha, alpha.size());
        else
            for (int x = 0; x < TileSide; ++x)
                alpha[x] = row.alpha[x * row.alphaStride];

        fullColor = std::find(alpha.begin(), alpha.end(), 0) == alpha.end();
        if (fullColor && row.sourceStride == 3)
            std::memcpy(bgr.data(), row.source, bgr.size());
        else
            for (int x = 0; x < TileSide; ++x)
                if (alpha[x])
                    std::memcpy(bgr.data() + x * 3, row.source + x * row.sourceStride, 3);
    }

    bool Matches(const PixelRow& row) const
    {
        if (fullColor && row.sourceStride == 3 && row.alphaStride == 1)
            return std::memcmp(alpha.data(), row.alpha, alpha.size()) == 0 &&
                std::memcmp(bgr.data(), row.source, bgr.size()) == 0;
        for (int x = 0; x < TileSide; ++x)
        {
            if (alpha[x] != row.alpha[x * row.alphaStride] ||
                (alpha[x] && std::memcmp(bgr.data() + x * 3, row.source + x * row.sourceStride, 3)))
                return false;
        }
        return true;
    }

    void CopyOutput(const PixelRow& row, const BgrRow& output) const
    {
        if (fullColor && row.destinationStride == 3)
            std::memcpy(row.destination, output.data(), output.size());
        else
            for (int x = 0; x < TileSide; ++x)
                if (alpha[x])
                    std::memcpy(row.destination + x * row.destinationStride, output.data() + x * 3, 3);
    }
};

struct InputTile
{
    InputTile(int x, int y) : x(x), y(y) {}

    const int x, y;
    std::array<InputRow, TileSide> rows;
};

using Inputs = std::vector<std::shared_ptr<const InputTile>>;

struct OutputTile
{
    explicit OutputTile(const std::shared_ptr<const InputTile>& input) : input(input) {}

    const std::shared_ptr<const InputTile> input;
    std::array<BgrRow, TileSide> rows;
    std::atomic<bool> ready{false};
};

class Frame
{
public:
    explicit Frame(const Inputs& inputs)
    {
        outputs.reserve(inputs.size());
        for (const auto& input : inputs)
            outputs.emplace_back(std::make_unique<OutputTile>(input));
        observed.reserve(TileLimit);
    }

    const OutputTile* Find(int x, int y) const
    {
        for (const auto& output : outputs)
            if (output->input->x == x && output->input->y == y)
                return output.get();
        return nullptr;
    }

    bool CanRemember() const { return !full.load(std::memory_order_relaxed); }

    void Remember(std::shared_ptr<const InputTile> input)
    {
        const std::lock_guard<std::mutex> lock(mutex);
        for (auto& previous : observed)
        {
            if (previous->x == input->x && previous->y == input->y)
            {
                previous = std::move(input);
                return;
            }
        }
        if (observed.size() < TileLimit)
        {
            observed.emplace_back(std::move(input));
            full.store(observed.size() == TileLimit, std::memory_order_relaxed);
        }
    }

    Inputs ObservedInputs()
    {
        const std::lock_guard<std::mutex> lock(mutex);
        return observed;
    }

private:
    friend class Work;
    std::vector<std::unique_ptr<OutputTile>> outputs;
    std::mutex mutex;
    Inputs observed;
    std::atomic<bool> full{false};
};

// One native region at a time. The native planes are borrowed only for the current row.
class Region
{
public:
    bool Render(Frame& frame, int x, int y, int line, const PixelRow& row)
    {
        if (line == 0)
        {
            output = frame.Find(x, y);
            input.reset();
            collecting = output || frame.CanRemember();
        }
        if (line >= TileSide || row.width != TileSide || !row.source || !row.destination || !row.alpha ||
            !row.alphaStride || (row.sourceStride > -3 && row.sourceStride < 3) ||
            (row.destinationStride > -3 && row.destinationStride < 3))
        {
            collecting = false;
            return false;
        }

        const auto* previous = output ? &output->input->rows[line] : nullptr;
        const bool matches = previous && previous->Matches(row);
        if (collecting)
        {
            if (!matches && !input)
            {
                input = std::make_shared<InputTile>(x, y);
                if (previous)
                    std::copy_n(output->input->rows.begin(), line, input->rows.begin());
            }
            if (input)
            {
                if (matches)
                    input->rows[line] = *previous;
                else
                    input->rows[line].Save(row);
            }
            if (line == TileSide - 1)
                frame.Remember(input ? std::move(input) : output->input);
        }

        if (!matches || !output->ready.load(std::memory_order_acquire))
            return false;
        previous->CopyOutput(row, output->rows[line]);
        return true;
    }

private:
    const OutputTile* output = nullptr;
    std::shared_ptr<InputTile> input;
    bool collecting = false;
};

// Workers process private copies. Destruction cancels queued work and drains callbacks.
class Work
{
public:
    Work(std::shared_ptr<Frame> frame, std::shared_ptr<const RowRenderer> renderer)
        : frame(std::move(frame)), renderer(std::move(renderer))
    {
        work = CreateThreadpoolWork(Run, this, nullptr);
        if (work)
        {
            const size_t count = (std::min)(WorkerLimit, this->frame->outputs.size());
            for (size_t i = 0; i < count; ++i)
                SubmitThreadpoolWork(work);
        }
    }

    ~Work()
    {
        if (work)
        {
            canceled.store(true, std::memory_order_relaxed);
            WaitForThreadpoolWorkCallbacks(work, TRUE);
            CloseThreadpoolWork(work);
        }
    }

    Work(const Work&) = delete;
    Work& operator=(const Work&) = delete;

private:
    static void CALLBACK Run(PTP_CALLBACK_INSTANCE, void* context, PTP_WORK)
    {
        auto& batch = *static_cast<Work*>(context);
        for (;;)
        {
            const auto index = batch.next.fetch_add(1, std::memory_order_relaxed);
            if (index >= batch.frame->outputs.size())
                return;
            auto& tile = *batch.frame->outputs[index];
            for (int line = 0; line < TileSide; ++line)
            {
                if (batch.canceled.load(std::memory_order_relaxed))
                    return;
                const auto& input = tile.input->rows[line];
                batch.renderer->RenderRow(PixelRow{tile.rows[line].data(), input.bgr.data(), input.alpha.data(),
                    3, 3, 1, TileSide});
            }
            tile.ready.store(true, std::memory_order_release);
        }
    }

    const std::shared_ptr<Frame> frame;
    const std::shared_ptr<const RowRenderer> renderer;
    PTP_WORK work = nullptr;
    std::atomic<size_t> next{0};
    std::atomic<bool> canceled{false};
};
}
