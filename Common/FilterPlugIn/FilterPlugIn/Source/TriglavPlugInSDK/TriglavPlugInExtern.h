//! TriglavPlugIn SDK
//! Copyright (c) CELSYS Inc.
//! All Rights Reserved.

#ifndef	_TRIGLAVPLUGINEXTERN_H_
#define	_TRIGLAVPLUGINEXTERN_H_

#ifdef __cplusplus
#define TRIGLAV_PLUGIN_EXTERN_C			extern "C"
#define TRIGLAV_PLUGIN_EXTERN_C_START	extern "C" {
#define TRIGLAV_PLUGIN_EXTERN_C_END		}
#else
#define TRIGLAV_PLUGIN_EXTERN_C
#define TRIGLAV_PLUGIN_EXTERN_C_START
#define TRIGLAV_PLUGIN_EXTERN_C_END
#endif

#if defined(_WINDOWS)
	#define	TRIGLAV_PLUGIN_DLL_EXTERN	TRIGLAV_PLUGIN_EXTERN_C	__declspec(dllexport)
#else
	#define TRIGLAV_PLUGIN_DLL_EXTERN	TRIGLAV_PLUGIN_EXTERN_C	__attribute__((visibility("default")))
#endif

#if defined(_WINDOWS)
	#define	TRIGLAV_PLUGIN_API			__cdecl
	#define	TRIGLAV_PLUGIN_CALLBACK		__cdecl
#else
	#define	TRIGLAV_PLUGIN_API			//__attribute__((__cdecl__))
	#define	TRIGLAV_PLUGIN_CALLBACK		//__attribute__((__cdecl__))
#endif

#if PRAGMA_STRUCT_ALIGN
	#define TRIGLAV_PLUGIN_PACK			options align=mac68k
	#define TRIGLAV_PLUGIN_RESETPACK	options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#define TRIGLAV_PLUGIN_PACK			pack(push,1)
	#define TRIGLAV_PLUGIN_RESETPACK	pack(pop)
#elif PRAGMA_STRUCT_PACK
	#define TRIGLAV_PLUGIN_PACK			pack(1)
	#define TRIGLAV_PLUGIN_RESETPACK	pack()
#else
	#define TRIGLAV_PLUGIN_PACK			pack(push,1)
	#define TRIGLAV_PLUGIN_RESETPACK	pack(pop)
#endif

#endif
