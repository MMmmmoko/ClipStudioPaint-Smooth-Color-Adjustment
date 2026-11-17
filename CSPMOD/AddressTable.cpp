#include "pch.h"
#include "AddressTable.h"
#include "CSPMOD.h"
#include "CustomAdjustmentLayer.h"
#include "FreeTransformLocator.h"
#include "LevelsPreview.h"
#include "../Common/AddressGenerator/AddressGenerator.h"

#include <winver.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <utility>

#pragma comment(lib, "version.lib")

namespace
{
// Increment when the address table format or locator results change.
#ifdef CSPMOD_PRIVATE
constexpr unsigned tableSchema = 33;
#else
constexpr unsigned tableSchema = 34;
#endif
constexpr auto tableName = L"CSPAddrTable.json";
constexpr auto logName = L"CSPAddrTable.build.log";

std::filesystem::path ExecutablePath()
{
    std::wstring path(32768, L'\0');
    const DWORD length = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
    if (!length || length >= path.size())
        return {};
    path.resize(length);
    return path;
}

std::string ProductVersion(const std::filesystem::path& path)
{
    DWORD ignored = 0;
    const DWORD length = GetFileVersionInfoSizeW(path.c_str(), &ignored);
    if (!length)
        return {};

    std::vector<uint8_t> data(length);
    void* value = nullptr;
    UINT valueLength = 0;
    if (!GetFileVersionInfoW(path.c_str(), 0, length, data.data()) ||
        !VerQueryValueW(data.data(), L"\\", &value, &valueLength) ||
        valueLength < sizeof(VS_FIXEDFILEINFO))
        return {};

    const auto& info = *static_cast<const VS_FIXEDFILEINFO*>(value);
    if (info.dwSignature != VS_FFI_SIGNATURE)
        return {};

    std::string version = std::to_string(HIWORD(info.dwProductVersionMS)) + "." +
        std::to_string(LOWORD(info.dwProductVersionMS)) + "." +
        std::to_string(HIWORD(info.dwProductVersionLS));
    if (LOWORD(info.dwProductVersionLS))
        version += "." + std::to_string(LOWORD(info.dwProductVersionLS));
    return version;
}

const IMAGE_SECTION_HEADER* FindSection(const IMAGE_NT_HEADERS64& nt,
    const char (&name)[IMAGE_SIZEOF_SHORT_NAME], DWORD requiredFlags)
{
    const auto* sections = IMAGE_FIRST_SECTION(&nt);
    for (unsigned i = 0; i < nt.FileHeader.NumberOfSections; ++i)
    {
        const auto& section = sections[i];
        if (memcmp(section.Name, name, IMAGE_SIZEOF_SHORT_NAME) == 0 &&
            (section.Characteristics & requiredFlags) == requiredFlags)
            return &section;
    }
    return nullptr;
}

Json::Value ReadTable(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);
    Json::Value table;
    if (file)
    {
        Json::CharReaderBuilder reader;
        if (!Json::parseFromStream(reader, file, &table, nullptr))
            table = Json::Value{};
    }
    return table;
}

bool WriteTable(const std::filesystem::path& path, const Json::Value& table)
{
    auto temporary = path;
    temporary += L"." + std::to_wstring(GetCurrentProcessId()) + L".tmp";
    std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
    if (!file)
        return false;
    Json::StreamWriterBuilder writer;
    file << Json::writeString(writer, table);
    file.close();
    if (file && MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING))
        return true;

    DeleteFileW(temporary.c_str());
    return false;
}

bool ValidEntry(const Json::Value& entry, size_t size)
{
    const auto validRva = [size](const Json::Value& value) {
        return value.isUInt64() && value.asUInt64() > 0 && value.asUInt64() < size;
    };
    if (entry.isArray())
        return std::all_of(entry.begin(), entry.end(), validRva);
    return validRva(entry);
}

bool MatchesTable(const Json::Value& table, const std::string& version)
{
    return table.isObject() && table["SchemaVersion"].isUInt() &&
        table["SchemaVersion"].asUInt() == tableSchema && table["CspVersion"] == Json::Value(version) &&
        table["CspAddressRVA"].isObject();
}
}

static AddressTable ins;

bool AddressTable::LoadOrBuild()
{
    const auto path = ExecutablePath();
    const auto* image = reinterpret_cast<const uint8_t*>(CSPMOD::GetBaseAddr());
    const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(image);
    if (!image || dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew <= 0)
        return false;

    const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS64*>(image + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE || nt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
        return false;

    ins.imageSize = nt->OptionalHeader.SizeOfImage;

    ins.cspVersion = ProductVersion(path);
    if (ins.cspVersion.empty())
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Address table: executable product version unavailable.");
        return false;
    }

    const auto tablePath = path.parent_path() / tableName;
    auto table = ReadTable(tablePath);
    bool matches = MatchesTable(table, ins.cspVersion);
    const auto filename = path.filename().wstring();
    std::filesystem::path fallback;
    std::filesystem::path fallbackTable;
    if (char* preferencePath = matches ? nullptr : SDL_GetPrefPath("CSPMOD", "AddressTable"))
    {
        fallback = std::filesystem::u8path(preferencePath);
        SDL_free(preferencePath);
        const auto version = std::filesystem::u8path(ins.cspVersion).wstring();
        fallbackTable = fallback / (L"CSPAddrTable-" + filename + L"-" + version + L".json");
        table = ReadTable(fallbackTable);
        matches = MatchesTable(table, ins.cspVersion);
    }
    if (matches)
    {
        ins.addressMap = std::move(table);
        SDL_Log("Address table: loaded for version %s.", ins.cspVersion.c_str());
        DoStaticPatch();
        return true;
    }

    const auto* text = FindSection(*nt, ".text\0\0", IMAGE_SCN_MEM_EXECUTE);
    const auto* data = FindSection(*nt, ".data\0\0", IMAGE_SCN_MEM_WRITE);
    if (!text || text->VirtualAddress >= ins.imageSize)
        return false;
    const size_t textSize = (std::min)({static_cast<size_t>(text->Misc.VirtualSize),
        static_cast<size_t>(text->SizeOfRawData), ins.imageSize - text->VirtualAddress});

    std::ostringstream featureLog;
    Json::Value verifiedFeatures(Json::objectValue);
    const auto& unwind = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION];
    const bool freeTransformReady = FreeTransform::Locate(image, static_cast<uint32_t>(ins.imageSize),
        text->VirtualAddress, static_cast<uint32_t>(textSize), unwind.VirtualAddress, unwind.Size,
        verifiedFeatures, featureLog);
    const bool levelsReady = LevelsPreview::Locate(image, static_cast<uint32_t>(ins.imageSize),
        text->VirtualAddress, static_cast<uint32_t>(textSize), unwind.VirtualAddress, unwind.Size,
        data ? data->VirtualAddress : 0, data ? data->Misc.VirtualSize : 0, verifiedFeatures, featureLog);
    const bool customAdjustmentReady = CustomAdjustmentLayer::Locate(image, static_cast<uint32_t>(ins.imageSize),
        text->VirtualAddress, static_cast<uint32_t>(textSize), unwind.VirtualAddress, unwind.Size,
        verifiedFeatures, featureLog);
    const bool customAdjustmentNavigationReady = customAdjustmentReady &&
        CustomAdjustmentLayer::LocateDialogNavigation(image, static_cast<uint32_t>(ins.imageSize),
            text->VirtualAddress, static_cast<uint32_t>(textSize), unwind.VirtualAddress, unwind.Size,
            verifiedFeatures, featureLog);
    const bool customAdjustmentMenuReady = customAdjustmentReady &&
        CustomAdjustmentLayer::LocateNativeMenu(image, static_cast<uint32_t>(ins.imageSize),
            text->VirtualAddress, static_cast<uint32_t>(textSize), unwind.VirtualAddress, unwind.Size,
            verifiedFeatures, featureLog);
    const bool customAdjustmentCatalogReady = customAdjustmentMenuReady &&
        CustomAdjustmentLayer::LocateCommandCatalog(image, static_cast<uint32_t>(ins.imageSize),
            text->VirtualAddress, static_cast<uint32_t>(textSize), unwind.VirtualAddress, unwind.Size,
            verifiedFeatures, featureLog);
    const bool customAdjustmentBindingsReady = customAdjustmentCatalogReady &&
        CustomAdjustmentLayer::LocateCommandBindings(image, static_cast<uint32_t>(ins.imageSize),
            text->VirtualAddress, static_cast<uint32_t>(textSize), unwind.VirtualAddress, unwind.Size,
            verifiedFeatures, featureLog);
    if (!customAdjustmentReady)
        featureLog << "SKIP: CustomAdjustment dialog navigation and native menu require the layer factory and render callbacks.\n";
    if (!customAdjustmentMenuReady)
        featureLog << "SKIP: CustomAdjustment command catalog requires the native menu.\n";
    if (!customAdjustmentCatalogReady)
        featureLog << "SKIP: CustomAdjustment shortcut and toolbar bindings require the command catalog.\n";

    std::ofstream log(path.parent_path() / logName, std::ios::out | std::ios::trunc);
    if (!log && !fallback.empty())
    {
        const auto fallbackLog = fallback / logName;
        log.clear();
        log.open(fallbackLog, std::ios::out | std::ios::trunc);
        if (log)
            SDL_Log("Address table log: %s", fallbackLog.u8string().c_str());
    }

    if (!log)
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Address table: could not open a build log.");
    log << "Address table rebuild for " << path.filename().u8string()
        << " version " << ins.cspVersion << '\n';
    SDL_Log("Address table: rebuilding for version %s.", ins.cspVersion.c_str());
    AddressGenerator generator;
    generator.SetUpCode(image + text->VirtualAddress, textSize, text->VirtualAddress, &log);
    const bool isUDM = _wcsnicmp(filename.c_str(), L"UDMPaint", 8) == 0;
    table = generator.Build(ins.cspVersion, isUDM);
    auto& entries = table["CspAddressRVA"];
    for (const auto& name : verifiedFeatures.getMemberNames())
        entries[name] = verifiedFeatures[name];
    log << featureLog.str();
    if (!freeTransformReady || !levelsReady || !customAdjustmentReady || !customAdjustmentNavigationReady ||
        !customAdjustmentMenuReady ||
        !customAdjustmentCatalogReady || !customAdjustmentBindingsReady)
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s", featureLog.str().c_str());

    for (const auto& name : entries.getMemberNames())
    {
        if (!ValidEntry(entries[name], ins.imageSize))
        {
            log << "ERROR: " << name << " points outside the executable image.\n";
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Address table: invalid feature %s.", name.c_str());
            entries.removeMember(name);
        }
    }

    table["SchemaVersion"] = tableSchema;
    if (!WriteTable(tablePath, table) && (fallbackTable.empty() || !WriteTable(fallbackTable, table)))
    {
        log << "Cache could not be saved. The rebuilt table is available for this process.\n";
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Address table: cache unavailable, using the rebuilt table in memory.");
    }
    log << "Located " << entries.size() << " address entries.\n";
    ins.addressMap = std::move(table);

    DoStaticPatch();
    return true;
}

void AddressTable::DoStaticPatch()
{
    //TimerPoints
    


    if (ins.addressMap["CspAddressRVA"].isMember("TimerAddrs") && ins.addressMap["CspAddressRVA"]["TimerAddrs"].isArray())
    {
        for (auto& x : ins.addressMap["CspAddressRVA"]["TimerAddrs"])
        {
            if (x.isUInt64() && x.asUInt64() > 0 && x.asUInt64() <= ins.imageSize - 6)
            {
                void* addr = reinterpret_cast<void*>(CSPMOD::GetBaseAddr() + x.asUInt64());

                uint8_t cmd_mov_edx_0_nop[6] = { 0xba,0x01,0x00 ,0x00 ,0x00 ,0x90 };
				CSPMOD::CodePatch(addr, cmd_mov_edx_0_nop, sizeof(cmd_mov_edx_0_nop));
                //memcpy(_codeMem + i, cmd_mov_edx_0_nop, sizeof(cmd_mov_edx_0_nop));
            }
        }
    }


    //UDMPlugin
    {
		void* pUDMPluginCheckFunc = GetAddress("UDMPluginUnlockAddr");
        if (pUDMPluginCheckFunc)
        {
            uint8_t cmd_nop[6] = { 0x90,0x90,0x90 ,0x90 ,0x90 ,0x90 };
			CSPMOD::CodePatch(pUDMPluginCheckFunc, cmd_nop, sizeof(cmd_nop));
        }
    }
    if (ins.addressMap["CspAddressRVA"].isMember("UDMPluginUnlockAddrVec") && ins.addressMap["CspAddressRVA"]["UDMPluginUnlockAddrVec"].isArray())
    {
        for (auto& x : ins.addressMap["CspAddressRVA"]["UDMPluginUnlockAddrVec"])
        {
            if (x.isUInt64())
            {
                void* addr = (void*)(x.asUInt64() + CSPMOD::GetBaseAddr());

                uint8_t cmd_nop[6] = { 0x90,0x90,0x90 ,0x90 ,0x90 ,0x90 };
                CSPMOD::CodePatch(addr, cmd_nop, sizeof(cmd_nop));
            }
        }
    }



    //return false;
}










void* AddressTable::GetAddress(const char* addrName)
{

    const auto& entries = std::as_const(ins.addressMap)["CspAddressRVA"];
    const auto& value = entries[addrName];
    if (value.isUInt64() && value.asUInt64() > 0 && value.asUInt64() < ins.imageSize)
        return reinterpret_cast<void*>(CSPMOD::GetBaseAddr() + value.asUInt64());

    return nullptr;
}

std::vector<int> AddressTable::GetOffsetList(const char* offsetName)
{
    std::vector<int> result;
    if (ins.addressMap.isMember(offsetName) && ins.addressMap[offsetName].isArray())
    {
        for (auto& x : addressMap[offsetName])
        {
            if (x.isInt())
            {
                result.push_back(x.asInt());
            }
        }
    }
    return result;
}

std::string_view AddressTable::GetCSPVersion()
{
    return ins.cspVersion;
}
