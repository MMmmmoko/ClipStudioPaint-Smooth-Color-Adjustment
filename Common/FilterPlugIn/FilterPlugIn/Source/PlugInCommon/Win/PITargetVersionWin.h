//!
//! @file  targetver.h
//! @brief Winのプラットフォームバージョン
//!
#pragma once

// 以下のマクロは、最低限必要なプラットフォームを定義します。最低限必要なプラットフォームとは、
// アプリケーションを実行するために必要な機能を備えた最も古いバージョンの Windows や Internet Explorer など
// をいいます。これらのマクロは、指定したバージョンと、それ以前のバージョンのプラットフォーム上で利用できるすべての機能を有効にすることによって 
// 動作します。

// 下で指定された定義の前に対象プラットフォームを指定しなければならない場合、以下の定義を変更してください。
// 異なるプラットフォームに対応する値に関する最新情報については、MSDN を参照してください。
//------------------------------------------------------------------------------
//	Windows 95 and Windows NT 4.0		WINVER=0x0400
//	Windows 98 and Windows NT 4.0		_WIN32_WINDOWS=0x0410 and WINVER=0x0400 
//	Windows NT 4.0						_WIN32_WINNT=0x0400 and WINVER=0x0400
//	Windows 98							_WIN32_WINDOWS=0x0410 
//	Windows 2000						_WIN32_WINNT=0x0500 and WINVER=0x0500  
//	Windows Me							_WIN32_WINDOWS=0x0490 
//	Windows XP and Windows .NET Server	_WIN32_WINNT=0x0501 and WINVER=0x0501
//	Internet Explorer 3.0, 3.01, 3.02	_WIN32_IE=0x0300 
//	Internet Explorer 4.0				_WIN32_IE=0x0400 
//	Internet Explorer 4.01				_WIN32_IE=0x0401 
//	Internet Explorer 5.0, 5.0a, 5.0b	_WIN32_IE=0x0500
//	Internet Explorer 5.01, 5.5			_WIN32_IE=0x0501
//	Internet Explorer 6.0				_WIN32_IE=0x0560 or _WIN32_IE=0x0600 
//------------------------------------------------------------------------------
#ifndef WINVER                          // 最低限必要なプラットフォームが Windows Vista であることを指定します。
//#define WINVER 0x0600           // これを Windows の他のバージョン向けに適切な値に変更してください。
#define WINVER 0x0501           // これを Windows の他のバージョン向けに適切な値に変更してください。
#endif

#ifndef _WIN32_WINNT            // 最低限必要なプラットフォームが Windows Vista であることを指定します。
//#define _WIN32_WINNT 0x0600     // これを Windows の他のバージョン向けに適切な値に変更してください。
#define _WIN32_WINNT 0x0501     // これを Windows の他のバージョン向けに適切な値に変更してください。
#endif

#ifndef _WIN32_WINDOWS          // 最低限必要なプラットフォームが Windows 98 であることを指定します。
#define _WIN32_WINDOWS 0x0410 // これを Windows Me またはそれ以降のバージョン向けに適切な値に変更してください。
#endif

#ifndef _WIN32_IE                       // 最低限必要なプラットフォームが Internet Explorer 7.0 であることを指定します。
#define _WIN32_IE 0x0700        // これを IE の他のバージョン向けに適切な値に変更してください。
#endif
