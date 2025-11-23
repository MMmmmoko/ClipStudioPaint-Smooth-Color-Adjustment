#include"AddressGenerator.h"
#include"DllInject.h"
#include<SDL3/SDL.h>



void AddressGenerator::_PushHsvAddr()
{
	SDL_Log("Start Find HSV Addresses...");

	//?? ?? ?? ?? ?? ?? 83 BB E8 00 00 00 00 74 1B C7 83 E8 00 00 00 00 00 00 00 48 8D 8B F8 00 00 00 BA E8 03 00 00 E8 ?? ?? ?? ?? 48 8D 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 8B D0
	uint8_t hsvSkipFeature[] =
	{ 0xCC, 0xCC,0xCC,0xCC,0xCC,0xCC,
	0x83 ,0xBB ,0xE8 ,0x00 ,0x00 ,0x00 ,0x00 ,0x74 ,0x1B
	,0xC7 ,0x83 ,0xE8 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x48
	,0x8D ,0x8B ,0xF8 ,0x00 ,0x00 ,0x00 ,0xBA ,0xE8 ,0x03 ,0x00 ,0x00
	,0xE8 ,0xCC,0xCC,0xCC,0xCC ,0x48 ,0x8D ,0x0D ,0xCC,0xCC,0xCC,0xCC ,0xE8 ,0xCC,0xCC,0xCC,0xCC ,0x8B ,0xD0 };

	


	bool success = false;
    for (size_t i = 0; i < _codeMemSize - sizeof(hsvSkipFeature); i++)
    {
        if (_MatchFeatureCode(_codeMem + i, hsvSkipFeature, sizeof(hsvSkipFeature)))
        {
			SDL_Log("HSV Skip Addr Finded.");
			addrJson["CspAddressRVA"]["HSV_SkipTimer"]=_VA+i;
			success = true;
            break;
        }
    }
	if(!success)
		SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_ERROR,"HSV Skip Addr Not Found!");









}