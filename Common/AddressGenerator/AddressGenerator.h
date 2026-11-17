#pragma once

#include "json/json.h"
#include <cstdint>
#include <ostream>

#define BYTEWILDCARD 0xCC

class AddressGenerator
{
public:
    void SetUpCode(const uint8_t* code, size_t size, uint32_t rva, std::ostream* log = nullptr);
    Json::Value Build(const std::string& version, bool isUDM);

private:
    void Scan();
    void PushCSPHelperAddr();
    void PushCSPDataAddr();
#ifdef CSPMOD_PRIVATE
	void PushPaintColorAddr();
#endif
    void PushTimeLapseExportAddr();
    void PushLayerObjectAddr();
    void PushCustomFilterAddr();
    void PushTimerPointAddr();
    void PushUDMPluginUnlockAddr();
    void _PushHsvAddr();
    void _PushColorBalanceAddr();
    void _PushToneCurveAddr();

    void Log(const char* format, ...);
    void LogError(const char* format, ...);
    static bool _MatchFeatureCode(const uint8_t* code, const uint8_t* feature, size_t size);

    uint32_t _VA = 0;
    const uint8_t* _codeMem = nullptr;
    size_t _codeMemSize = 0;
    std::ostream* _log = nullptr;
    Json::Value addrJson;
};
