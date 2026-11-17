#pragma once

class CSPHelper
{
public:

	static void Init();


	static void OnClickEntrance();
	static inline void(*orig_OnClickEntrance)() = nullptr;




#ifdef CSPMOD_PRIVATE
	static void LoadLicenseAtStartup();
	static void ShowLicenseDialog();
	static bool isProUser();
	static bool isPaid();
#endif



};
