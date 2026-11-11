#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <vector>

namespace NativeCode
{
struct FunctionRange
{
    uint32_t begin;
    uint32_t end;
    uint32_t unwindInfo;
};

using Pattern = std::vector<int>;

inline Pattern Parse(std::string_view bytes)
{
    Pattern pattern;
    for (size_t i = 0; i + 1 < bytes.size(); i += 3)
    {
        const auto digit = [](char c) { return c >= 'A' ? c - 'A' + 10 : c - '0'; };
        pattern.push_back(bytes[i] == '?' ? -1 : (digit(bytes[i]) << 4) + digit(bytes[i + 1]));
    }
    return pattern;
}

class Code
{
public:
    Code(const uint8_t* image, uint32_t imageSize, uint32_t textRva, uint32_t textSize,
        const FunctionRange* functions, size_t count)
        : image(image), imageSize(imageSize), start(textRva), stop(textRva + textSize),
          functions(functions), count(count)
    {
    }

    bool Match(uint32_t rva, const Pattern& pattern) const
    {
        if (rva < start || rva >= stop || pattern.size() > stop - rva)
            return false;
        for (size_t i = 0; i < pattern.size(); ++i)
            if (pattern[i] >= 0 && pattern[i] != image[rva + i])
                return false;
        return true;
    }

    bool Match(uint32_t rva, std::string_view bytes) const { return Match(rva, Parse(bytes)); }

    std::vector<uint32_t> Find(std::string_view bytes, uint32_t from = 0, uint32_t to = 0) const
    {
        const auto pattern = Parse(bytes);
        from = from ? from : start;
        to = to ? to : stop;
        std::vector<uint32_t> matches;
        if (from < start || to > stop || from >= to || pattern.empty() ||
            pattern.size() > to - from || pattern[0] < 0)
            return matches;

        const auto* cursor = image + from;
        const auto* limit = image + to - pattern.size() + 1;
        while (cursor < limit)
        {
            const auto* hit = static_cast<const uint8_t*>(memchr(cursor, pattern[0], limit - cursor));
            if (!hit)
                break;
            const auto rva = static_cast<uint32_t>(hit - image);
            if (Match(rva, pattern))
                matches.push_back(rva);
            cursor = hit + 1;
        }
        return matches;
    }

    FunctionRange Containing(uint32_t rva) const
    {
        const auto* next = std::upper_bound(functions, functions + count, rva,
            [](uint32_t address, const FunctionRange& function) { return address < function.begin; });
        if (next == functions || rva >= (next - 1)->end)
            return {};
        return *(next - 1);
    }

    uint32_t Call(uint32_t rva) const
    {
        if (rva < start || rva >= stop || stop - rva < 5 || image[rva] != 0xe8)
            return 0;
        const auto target = Relative(rva, 1, 5);
        return target >= start && target < stop ? target : 0;
    }

    uint32_t Relative(uint32_t rva, unsigned displacementAt, unsigned length) const
    {
        int32_t displacement = 0;
        memcpy(&displacement, image + rva + displacementAt, sizeof(displacement));
        const int64_t target = static_cast<int64_t>(rva) + length + displacement;
        return target > 0 && target < imageSize ? static_cast<uint32_t>(target) : 0;
    }

private:
    const uint8_t* image;
    uint32_t imageSize;
    uint32_t start;
    uint32_t stop;
    const FunctionRange* functions;
    size_t count;
};
}
