#include"AddressGenerator.h"
#include"DllInject.h"
#include<SDL3/SDL.h>


void AddressGenerator::PushCSPHelperAddr()
{
	SDL_Log("Start Find CSP Helper Entrance Addresses...");
	{
		uint8_t entranceFeature[] = {
		0x48,0x89,0x5C,0x24,0x10,0x48,0x89,0x74,0x24,0x18,0x55,0x57,0x41,
		0x56,0x48,0x8D,0xAC,0x24,0x40,0xFF,0xFF,0xFF,0x48,0x81,
		0xEC,0xC0,0x01,0x00,0x00,
		0x48,0x8B,0x05,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD ,
		0x48,0x33,0xC4,0x48,0x89,0x85,0xB0,0x00,0x00,0x00,0x48,0x8B,0xF1,
		0xE8,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,
		0x8B,0xC8,0x83,0xF8,0x03,0x0F,0x85,0x15,0x01,0x00,0x00,0x33,0xFF



		//48,89,5C,24,10,48,89,74,24,18,55,57,41,
		//56,48,8D,AC,24,40,FF,FF,FF,48,81,
		//EC,C0,01,00,00,
		//48,8B,05,??, ??,??,?? ,
		//48,33,C4,48,89,85,B0,00,00,00,48,8B,F1,
		//E8,??, ??,??,?? ,
		//8B,C8,83,F8,03,0F,85,15,01,00,00,33,FF
		};


		bool success = false;
		for (size_t i = 0; i < _codeMemSize - sizeof(entranceFeature); i++)
		{
			if (_MatchFeatureCode(_codeMem + i, entranceFeature, sizeof(entranceFeature)))
			{
				SDL_Log("CSPHelper Entrance Finded.");
				addrJson["CspAddressRVA"]["CSPHelper_Entrance"] = _VA + i;
				success = true;
				break;
			}
		}
		if (!success)
			SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_ERROR, "CSPHelper Entrance Not Found!");

	}


}





