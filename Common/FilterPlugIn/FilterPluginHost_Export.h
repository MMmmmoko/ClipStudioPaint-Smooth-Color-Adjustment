#pragma once
#include "TriglavPlugInSDK/TriglavPlugInType.h"
#include "TriglavPlugInSDK/TriglavPlugInServer.h"
#include "TriglavPlugInSDK/TriglavPlugInService.h"




typedef bool (*SetUpPluginInfoCall)(const char* pluginName, struct CSPFilterPlugin_Interface* interFace);


struct CSPFilterPlugin_Interface
{
	//void (*TriglavPlugInFilterPropertyCallBack)(TriglavPlugInInt* result, TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, const TriglavPlugInInt notify, TriglavPlugInPtr data) = nullptr;
	void (*TriglavPluginCall)(TriglavPlugInInt* result, TriglavPlugInPtr* data, TriglavPlugInInt selector, TriglavPlugInServer* pluginServer, TriglavPlugInPtr reserved) = nullptr;

};
