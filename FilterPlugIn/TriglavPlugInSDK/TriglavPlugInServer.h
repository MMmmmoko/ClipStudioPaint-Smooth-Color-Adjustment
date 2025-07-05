//! TriglavPlugIn SDK
//! Copyright (c) CELSYS Inc.
//! All Rights Reserved.

#ifndef	_TRIGLAVPLUGINSERVER_H_
#define	_TRIGLAVPLUGINSERVER_H_

#include "TriglavPlugInRecordFunction.h"
#include "TriglavPlugInService.h"

TRIGLAV_PLUGIN_EXTERN_C_START
#pragma TRIGLAV_PLUGIN_PACK

//	プラグインサーバー
typedef	struct	_TriglavPlugInServer
{
	TriglavPlugInRecordSuite		recordSuite;
	TriglavPlugInServiceSuite		serviceSuite;
	TriglavPlugInHostObject			hostObject;

}	TriglavPlugInServer;

#pragma TRIGLAV_PLUGIN_RESETPACK
TRIGLAV_PLUGIN_EXTERN_C_END

#endif
