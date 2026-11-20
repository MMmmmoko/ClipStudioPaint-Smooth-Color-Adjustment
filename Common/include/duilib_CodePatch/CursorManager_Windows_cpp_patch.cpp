//由于一些原因，CatTuber必须改动duilib的部分代码
//为了方便更新维护，修改的代码不存放在源文件中。

//这个修改是修复从内存（zip压缩包/内嵌资源）加载 .cur 光标时热点坐标丢失的问题。
//
//.cur 文件的热点保存在 ICONDIRENTRY 的 wPlanes/wBitCount 字段里
//（第1个条目在文件偏移 0x0A/0x0C），图像数据本身不含热点；
//而 CreateIconFromResourceEx 在 fIcon=FALSE 时要求数据前 4 字节是
//LOCALHEADER(xHotspot, yHotspot)，然后才是 BITMAPINFOHEADER。
//原实现直接按 fIcon=TRUE 创建图标（因为传 FALSE 会失败），
//图标当光标使用时热点会变成图像中心，导致吸管光标热点异常。
//这里按目录项读出热点，拼出 LOCALHEADER + 图像数据，再按光标创建。

#include "duilib/Core/CursorManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Utils/ApiWrapper_Windows.h"

#include <cstdint>
#include <cstring>
#include <vector>

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

namespace ui
{
namespace
{
    //.cur 的 ICONDIRENTRY；在光标文件中 wPlanes/wBitCount 表示热点坐标
    struct CursorDirEntry
    {
        uint8_t  bWidth;
        uint8_t  bHeight;
        uint8_t  bColorCount;
        uint8_t  bReserved;
        uint16_t wHotspotX;
        uint16_t wHotspotY;
        uint32_t dwBytesInRes;
        uint32_t dwImageOffset;
    };
    static_assert(sizeof(CursorDirEntry) == 16, "CursorDirEntry size mismatch");

    uint16_t ReadUInt16(const uint8_t* pData)
    {
        return static_cast<uint16_t>(pData[0] | (static_cast<uint16_t>(pData[1]) << 8));
    }

    uint32_t ReadUInt32(const uint8_t* pData)
    {
        return static_cast<uint32_t>(pData[0]) |
               (static_cast<uint32_t>(pData[1]) << 8) |
               (static_cast<uint32_t>(pData[2]) << 16) |
               (static_cast<uint32_t>(pData[3]) << 24);
    }
}

HCURSOR LoadCursorFromData(const Window* pWindow, std::vector<uint8_t>& fileData)
{
    if (fileData.empty() || (pWindow == nullptr)) {
        return nullptr;
    }
    const size_t fileDataSize = fileData.size();
    if (fileDataSize < 6) {
        return nullptr;
    }

    //只处理 .cur 文件：ICONDIR 的 idType 必须为 2
    const uint16_t idReserved = ReadUInt16(fileData.data());
    const uint16_t idType = ReadUInt16(fileData.data() + 2);
    const uint16_t idCount = ReadUInt16(fileData.data() + 4);
    bool bValidCursorFile = (idReserved == 0) && (idType == 2) && (idCount > 0);
    if (bValidCursorFile &&
        (fileDataSize < (6 + static_cast<size_t>(idCount) * sizeof(CursorDirEntry)))) {
        bValidCursorFile = false;
    }
    if (bValidCursorFile) {
        //检查每个图像数据是否越界
        for (uint16_t i = 0; i < idCount; ++i) {
            const uint8_t* pEntryData = fileData.data() + 6 + static_cast<size_t>(i) * sizeof(CursorDirEntry);
            const uint32_t dwBytesInRes = ReadUInt32(pEntryData + 8);
            const uint32_t dwImageOffset = ReadUInt32(pEntryData + 12);
            if ((dwImageOffset >= fileDataSize) ||
                (dwBytesInRes > (fileDataSize - dwImageOffset))) {
                bValidCursorFile = false;
                break;
            }
        }
    }
    ASSERT(bValidCursorFile);
    if (!bValidCursorFile) {
        return nullptr;
    }

    uint32_t uDpiScaleFactor = pWindow->Dpi().GetDisplayScaleFactor();
    uint32_t uDpi = pWindow->Dpi().MulDiv(uDpiScaleFactor, 96u, 100u);
    int32_t cxCursor = GetSystemMetricsForDpiWrapper(SM_CXCURSOR, uDpi);
    int32_t cyCursor = GetSystemMetricsForDpiWrapper(SM_CYCURSOR, uDpi);

    //沿用原来的选图方式，只补充选中条目的热点
    int32_t nImageOffset = ::LookupIconIdFromDirectoryEx((PBYTE)fileData.data(), TRUE, cxCursor, cyCursor, LR_DEFAULTCOLOR);
    if (nImageOffset <= 0) {
        return nullptr;
    }

    //找到被选中的目录项，读出热点坐标
    uint16_t wHotspotX = 0;
    uint16_t wHotspotY = 0;
    bool bFound = false;
    for (uint16_t i = 0; i < idCount; ++i) {
        const uint8_t* pEntryData = fileData.data() + 6 + static_cast<size_t>(i) * sizeof(CursorDirEntry);
        if (ReadUInt32(pEntryData + 12) == static_cast<uint32_t>(nImageOffset)) {
            wHotspotX = ReadUInt16(pEntryData + 4);
            wHotspotY = ReadUInt16(pEntryData + 6);
            bFound = true;
            break;
        }
    }
    ASSERT(bFound);
    if (!bFound) {
        return nullptr;
    }

    //CreateIconFromResourceEx(fIcon=FALSE) 要求数据以 LOCALHEADER(热点坐标) 开头
    const size_t imageSize = fileDataSize - static_cast<size_t>(nImageOffset);
    std::vector<uint8_t> cursorData(4 + imageSize);
    cursorData[0] = static_cast<uint8_t>(wHotspotX & 0xFF);
    cursorData[1] = static_cast<uint8_t>((wHotspotX >> 8) & 0xFF);
    cursorData[2] = static_cast<uint8_t>(wHotspotY & 0xFF);
    cursorData[3] = static_cast<uint8_t>((wHotspotY >> 8) & 0xFF);
    memcpy(cursorData.data() + 4, fileData.data() + nImageOffset, imageSize);

    HCURSOR hCursor = (HCURSOR)::CreateIconFromResourceEx(cursorData.data(), (DWORD)cursorData.size(),
                                                          FALSE, 0x00030000, cxCursor, cyCursor, LR_DEFAULTCOLOR);
    ASSERT(hCursor != nullptr);
    return hCursor;
}

} // namespace ui

#endif //DUILIB_BUILD_FOR_WIN && !DUILIB_BUILD_FOR_SDL
