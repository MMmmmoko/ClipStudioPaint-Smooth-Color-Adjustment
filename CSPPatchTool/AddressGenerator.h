#pragma once



#include"json/json.h"

class AddressGenerator
{

public:
	void SetUp(class DllInject* parent);
	bool PushAddressData();





	bool PushCspVersion();

	void PushCSPHelperAddr();
	void PushToneAdjustmentAddr();
	void PushCSPDataAddr();
	void PushTimeLapseExportAddr();
	void PushLayerObjectAddr();









	void GenerateOutputFile();


private:
	static bool _MatchFeatureCode(uint8_t* dest, uint8_t* fetureCode, size_t codeSize);

	void _PushHsvAddr();
	void _PushColorBalanceAddr();
	void _PushToneCurveAddr();


	DllInject* _parent;
	uint32_t _VA;
	uint8_t* _codeMem = nullptr;
	size_t _codeMemSize = 0;



	Json::Value addrJson;



};