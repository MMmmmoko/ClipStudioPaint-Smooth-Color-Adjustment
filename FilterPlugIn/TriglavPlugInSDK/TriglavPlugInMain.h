//! TriglavPlugIn SDK
//! Copyright (c) CELSYS Inc.
//! All Rights Reserved.

#ifndef	_TRIGLAVPLUGINMAIN_H_
#define	_TRIGLAVPLUGINMAIN_H_

#include "TriglavPlugInExtern.h"
#include "TriglavPlugInServer.h"

TRIGLAV_PLUGIN_DLL_EXTERN	void TRIGLAV_PLUGIN_CALLBACK TriglavPluginCall( TriglavPlugInInt* result, TriglavPlugInPtr* data, const TriglavPlugInInt selector, TriglavPlugInServer* pluginServer, TriglavPlugInPtr reserved );

#endif
