#pragma once
class CSPHelper
{
public:

	static void Init();



	static void OnClickEntrance();
	static inline void(*orig_OnClickEntrance)() = nullptr;

};