#include "AddressGenerator.h"
#include <SDL3/SDL.h>
#include <cstdarg>

void AddressGenerator::SetUpCode(const uint8_t* code, size_t size, uint32_t rva, std::ostream* log)
{
    _codeMem = code;
    _codeMemSize = size;
    _VA = rva;
    _log = log;
}

Json::Value AddressGenerator::Build(const std::string& version, bool isUDM)
{
    addrJson = Json::Value(Json::objectValue);
    addrJson["CspVersion"] = version;
    addrJson["IsUDM"] = isUDM;
    addrJson["CspAddressRVA"] = Json::Value(Json::objectValue);
    if (_codeMem && _codeMemSize)
        Scan();
    return std::move(addrJson);
}

void AddressGenerator::Scan()
{
    PushCSPHelperAddr();
    _PushHsvAddr();
    _PushColorBalanceAddr();
    _PushToneCurveAddr();
    PushCSPDataAddr();
    PushTimeLapseExportAddr();
    PushLayerObjectAddr();
    PushTimerPointAddr();
    PushUDMPluginUnlockAddr();
    PushCustomFilterAddr();
}

void AddressGenerator::Log(const char* format, ...)
{
    char message[1024]{};
    va_list args;
    va_start(args, format);
    SDL_vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    if (_log && *_log)
        *_log << message << '\n';
    SDL_Log("%s", message);
}

void AddressGenerator::LogError(const char* format, ...)
{
    char message[1024]{};
    va_list args;
    va_start(args, format);
    SDL_vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    if (_log && *_log)
        *_log << "ERROR: " << message << '\n';
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s", message);
}

bool AddressGenerator::_MatchFeatureCode(const uint8_t* code, const uint8_t* feature, size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        if (feature[i] != BYTEWILDCARD && feature[i] != code[i])
            return false;
    }
    return true;
}

void AddressGenerator::PushTimerPointAddr()
{
    uint8_t timerMsToNsFeature[] = {
0x48,0x69,0xC2,0x40,0x42,0x0F,0x00,
0x48,0x89,0x41,0x08,
0xC3
    };


    uint8_t timerFeature[] = {
0x8B ,0x15   ,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
,0x48 ,0x8D ,0x4C ,0x24 ,0x20

,0xE8 ,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
,0x48 ,0x8D ,0x05  ,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
,0x48 ,0x89 ,0x44 ,0x24 ,0x30
//,0x48 ,0x89 ,0x5C ,0x24 ,0x38
//,0x0F ,0x28 ,0x44 ,0x24 ,0x30
//,0x66 ,0x0F ,0x7F ,0x44 ,0x24 ,0x30
//,0x48 ,0x8D ,0x4C ,0x24 ,0x30
//削短？？

//8B ,15   , ??, ??, ??, ??,
//,48 ,8D ,4C ,24 ,20
//,E8 , ??, ??, ??, ??,
//,48 ,8D ,05  , ??, ??, ??, ??,
//,48 ,89 ,44 ,24 ,30
//,48 ,89 ,5C ,24 ,38
//,0F ,28 ,44 ,24 ,30
//,66 ,0F ,7F ,44 ,24 ,30
//,48 ,8D ,4C ,24 ,30
    };


    Log("Start Find TimerPoint Addresses...");
	bool anyCodeFinded = false;
    int j = 0;
    for (uint32_t i = 0; i + sizeof(timerMsToNsFeature) <= _codeMemSize; i++)
    {
        if (_MatchFeatureCode(_codeMem + i, timerMsToNsFeature, sizeof(timerMsToNsFeature)))
        {

            //先找毫秒转纳秒的函数地址以适当减少应用范围
            uint32_t timerMsToNsFuncAddr = i;
            for (i = 0; i + sizeof(timerFeature) <= _codeMemSize; i++)
            {


                if (_MatchFeatureCode(_codeMem + i, timerFeature, sizeof(timerFeature)))
                {
                    uint32_t callAddr = *(uint32_t*)(_codeMem + i + 12) + i + 12 + 4;
                    if (callAddr != timerMsToNsFuncAddr)continue;


                    addrJson["CspAddressRVA"]["TimerAddrs"][j] = (_VA + i);
                    j++;
                    anyCodeFinded = true;
                }


            }


            break;



            //uint8_t cmd_mov_edx_0_nop[6] = { 0xba,0x01,0x00 ,0x00 ,0x00 ,0x90 };
            //memcpy(_codeMem + i, cmd_mov_edx_0_nop, sizeof(cmd_mov_edx_0_nop));

        }
    }

    if (anyCodeFinded)
    {
        Log("TimerPoint Addresses Finded.");
    }
    else
    {
        LogError( "TimerPoint Addresses Not Found!");
    }


}

void AddressGenerator::PushUDMPluginUnlockAddr()
{

    uint8_t pluginFeature[] = {
    0x0F,0x84,BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD,0x48,0x8D,0x8C,0x24,BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD,
        0xE8, BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD,
        0x90,0x48,0x8B,0x54,0x24,BYTEWILDCARD,0x48,0x81,0xC2,
        0x88,0x00,0x00,0x00,0x48,0x8D,0x8C,0x24,BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD,
        0xE8, BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD,
        0x48,0x8D,0x8C,0x24,BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD,
        0xE8, BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD,
        0x85,0xC0,0x0F,0x8E
		//4.2.0失效了，这里减少特征码长度并将rsp+X也改为通配符



    //0F,84,??, ??, ??, ??,48,8D,8C,24,??, ??, ??, ??,
    //    E8, ??, ??, ??, ??,
    //    90,48,8B,54,24,??,48,81,C2,
    //    88,00,00,00,48,8D,8C,24,??, ??, ??, ??,
    //    E8, ??, ??, ??, ??,
    //    48,8D,8C,24,??, ??, ??, ??,
    //    E8, ??, ??, ??, ??,
    //    85,C0,0F,8E



    //0F,84,68,01,00,00,48,8D,8C,24,E8,00,00,00,
    //    E8, ??, ??, ??, ??,
    //    90,48,8B,54,24,30,48,81,C2,
    //    88,00,00,00,48,8D,8C,24,E8,00,00,00,
    //    E8, ? ? , ? ? , ? ? , ? ? ,
    //    48,8D,8C,24,E8,00,00,00,
    //    E8, ? ? , ? ? , ? ? , ? ? ,
    //    85,C0,0F,8E,1E,01,00,00,
    //    48,8D,8C,24,E8,00,00,00,
    //    E8, ? ? , ? ? , ? ? , ? ? ,
    };


    Log("Start Find Plugin Unlock Address...");
    bool anyCodeFinded = false;
    for (size_t i = 0; i + sizeof(pluginFeature) <= _codeMemSize; i++)
    {
        if (_MatchFeatureCode(_codeMem + i, pluginFeature, sizeof(pluginFeature)))
        {

            //第三方插件和官方插件在同一个地方加载，
            //我们寄宿到的插件是高斯模糊，首字母G会比流畅调色插件首字母S早
            //所以高斯模糊加载后，后面的第三方插件是能有效加载的
            //这个会找到2个地址，在跨越版本时难以找到具体单个的特征，全部改了
            addrJson["CspAddressRVA"]["UDMPluginUnlockAddrVec"].append(_VA + i);
            anyCodeFinded = true;
            //je指令改为nop
            //uint8_t cmd_nop[6] = { 0x90,0x90,0x90 ,0x90 ,0x90 ,0x90 };
            //memcpy(_codeMem + i, cmd_nop, sizeof(cmd_nop));

        }
    }

    if (anyCodeFinded)
    {
        Log("Plugin Unlock Address Finded.");
    }
    else
    {
		LogError( "Plugin Unlock Address Not Found!");
    }


}
