#pragma once

#include<SDL3/SDL.h>


class TA_HSV
{
public:
	static void Init();
	static bool Available() { return isAvailable; };
	static void Enable();
	static void Disable();
	static bool IsEnabled() { return isEnabled; };
	

private:
	static inline bool isAvailable= false;
	static inline bool isEnabled= false;


	static inline uint8_t* pSkipAddr = nullptr;


};