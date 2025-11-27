#pragma once
#include <cstdint>

class CSPHacker
{

public:
	//.text
	void SetUp(uint8_t* codeMem, size_t codeMemSize, uint32_t VA);
	bool DoBaseHack();
	bool DoJumpStartWindowHack();
	bool DoJumpHideTrialText();
	bool DoPluginUnlock();

	bool DoPatchTimerPoint();//按理来说这个应该交给CSPMOD.dll解决的.发现特征码匹配数量太多，说明有非常多神秘的延时机制，这里一起解决



	//bool DoJumpUDMKeyCheck();//解决优动漫启动问题


private:
	bool _FindCheckDataFunc();
	bool _FindCheckLicense();
	bool _MatchCallChekData(size_t pos);
	bool _MatchCallValueOf1450209b8(size_t pos);
	bool _MatchCallCheckLicense(size_t pos);
	bool MatchLicenseCode(size_t pos);
	void PatchLicenseCode(size_t pos);


	//0xcc作为通配符
	static bool _MatchFeatureCode(uint8_t* dest,uint8_t* fetureCode,size_t codeSize);


	uint32_t _VA;
	uint8_t* _codeMem=nullptr;
	size_t _codeMemSize=0;



	//一些特征函数位置
	size_t pos_CheckData = 0;
	size_t pos_CheckLicense = 0;





};

