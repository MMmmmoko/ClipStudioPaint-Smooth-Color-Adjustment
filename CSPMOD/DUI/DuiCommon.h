#pragma once



#ifdef _WINDOWS
//#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
#include<Windows.h>
#endif

// duilib
#include "duilib/duilib.h"





#define STRFROMID(x) ui::MutiLanSupport::GetInstance()->GetStringViaID(x) 