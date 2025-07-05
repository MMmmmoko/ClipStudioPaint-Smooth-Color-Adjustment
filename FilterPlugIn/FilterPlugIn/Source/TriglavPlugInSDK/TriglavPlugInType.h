//! TriglavPlugIn SDK
//! Copyright (c) CELSYS Inc.
//! All Rights Reserved.

#ifndef	_TRIGLAVPLUGINTYPE_H_
#define	_TRIGLAVPLUGINTYPE_H_

#include "TriglavPlugInExtern.h"
#include "TriglavPlugInDefine.h"

TRIGLAV_PLUGIN_EXTERN_C_START
#pragma TRIGLAV_PLUGIN_PACK

typedef	unsigned char	TriglavPlugInBool;
typedef	char			TriglavPlugInChar;
typedef	unsigned short	TriglavPlugInUniChar;
typedef	unsigned char	TriglavPlugInUInt8;
typedef	long int		TriglavPlugInInt;
typedef	long long int	TriglavPlugInInt64;
typedef	float			TriglavPlugInFloat;
typedef	double			TriglavPlugInDouble;
typedef	void*			TriglavPlugInPtr;

//	座標
typedef	struct	_TriglavPlugInPoint
{
	TriglavPlugInInt	x;
	TriglavPlugInInt	y;

}	TriglavPlugInPoint;

//	サイズ
typedef	struct	_TriglavPlugInSize
{
	TriglavPlugInInt	width;
	TriglavPlugInInt	height;

}	TriglavPlugInSize;

//	矩形
typedef	struct	_TriglavPlugInRect
{
	TriglavPlugInInt	left;
	TriglavPlugInInt	top;
	TriglavPlugInInt	right;
	TriglavPlugInInt	bottom;

}	TriglavPlugInRect;

//	RGBカラー
typedef	struct	_TriglavPlugInRGBColor
{
	TriglavPlugInUInt8	red;
	TriglavPlugInUInt8	green;
	TriglavPlugInUInt8	blue;

}	TriglavPlugInRGBColor;

//	CMYKカラー
typedef	struct	_TriglavPlugInCMYKColor
{
	TriglavPlugInUInt8	cyan;
	TriglavPlugInUInt8	magenta;
	TriglavPlugInUInt8	yellow;
	TriglavPlugInUInt8	keyplate;

}	TriglavPlugInCMYKColor;

//	ホスト
struct	_TriglavPlugInHost;
typedef	struct _TriglavPlugInHost*				TriglavPlugInHostObject;

//	ホスト権限
struct	_TriglavPlugInHostPermission;
typedef	struct _TriglavPlugInHostPermission*	TriglavPlugInHostPermissionObject;

//	文字列
struct	_TriglavPlugInString;
typedef	struct _TriglavPlugInString*			TriglavPlugInStringObject;

//	ビットマップ
struct	_TriglavPlugInBitmap;
typedef	struct _TriglavPlugInBitmap*			TriglavPlugInBitmapObject;

//	オフスクリーン
struct	_TriglavPlugInOffscreen;
typedef	struct _TriglavPlugInOffscreen*			TriglavPlugInOffscreenObject;

//	プロパティ
struct	_TriglavPlugInProperty;
typedef	struct _TriglavPlugInProperty*			TriglavPlugInPropertyObject;

#pragma TRIGLAV_PLUGIN_RESETPACK
TRIGLAV_PLUGIN_EXTERN_C_END

#endif
