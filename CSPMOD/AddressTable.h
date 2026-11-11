#pragma once

#include "json/json.h"
#include <string_view>
#include <vector>

class AddressTable
{
public:
    static bool LoadOrBuild();
    static void* GetAddress(const char* addrName);
    std::vector<int> GetOffsetList(const char* offsetName);
    static std::string_view GetCSPVersion();

private:
    static void DoStaticPatch();

    Json::Value addressMap;
    std::string cspVersion;
    size_t imageSize = 0;
};
