#pragma once



#ifdef _WINDOWS
//#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // �� Windows ͷ�ļ����ų�����ʹ�õ�����
#include<Windows.h>
#endif

// duilib
#include "duilib/duilib.h"





#define STRFROMID(x) ui::MutiLanSupport::GetInstance()->GetStringViaID(x)