#include "CSPHacker.h"


#include <iostream>
#include "DllInject.h"




void CSPHacker::SetUp(uint8_t* codeMem, size_t codeMemSize, uint32_t VA)
{
    { _codeMem = codeMem; _codeMemSize = codeMemSize; _VA=VA; }

    _FindCheckDataFunc();
    _FindCheckLic();


    if (0 == pos_CheckData)
    {
        printf("Func Not Founded: CheckData\n");
    }

    if (0 == pos_CheckLicense)
    {
        printf("Func Not Founded: CheckLic\n");
    }



}













//CSP的lic检测目前看来像是一段宏插进代码的，而不是写在函数内的。
//其有如下特征


//00000001403AE6B9 | E8 52941B03 | call <clipstudiopaint.checkData> |
// 
// 
// 
// CODE PART 2
//00000001403AE6BE | 84C0 | test al, al |
//00000001403AE6C0 | 74 1B | je clipstudiopaint.1403AE6DD |
// 
// 
// 
// 
//00000001403AE6C2 | E8 E9BE1203 | call <clipstudiopaint.valueof(1450209B8)> |
// 
// 
// 
//  CODE PART 3
//00000001403AE6C7 | 48 : 85C0 | test rax, rax |
//00000001403AE6CA | 74 11 | je clipstudiopaint.1403AE6DD |
// 
// 
// 
// 
//00000001403AE6CC | E8 DFBE1203 | call <clipstudiopaint.valueof(1450209B8)> |
// 
// 
//00000001403AE6D1 | 48 : 8BC8 | mov rcx, rax |
// 
//00000001403AE6D4 | E8 F7F61203 | call <clipstudiopaint.checklic> |
//00000001403AE6D9 | 85C0 | test eax, eax |
// 
// 
// 
// 检查eax=1之后的逻辑
// 
//////**********************00000001403AE6DB | 75 62 | jne clipstudiopaint.1403AE73F |

//这里尝试以匹配、修改的方法将这些代码移除


uint8_t* Posstart = NULL;
bool CSPHacker::_MatchCallValueOf1450209b8(size_t pos)
{

    uint8_t* curPtr = _codeMem + pos;

    //这个函数在不同的版本取的地址不同，不针对版本特调的话只能模糊匹配
    uint8_t CallValueOf1450209b8_Feature1[] = {
        0xE8,  0xCC, 0xCC, 0xCC, 0xCC,
        0x48, 0x85, 0xC0
    };
    uint8_t CallValueOf1450209b8_Feature2[] = {
        0xE8,  0xCC, 0xCC, 0xCC, 0xCC,
        0x48, 0x8B, 0xC8
    };
    return _MatchFeatureCode(curPtr, CallValueOf1450209b8_Feature1,sizeof(CallValueOf1450209b8_Feature1))
        || _MatchFeatureCode(curPtr, CallValueOf1450209b8_Feature2, sizeof(CallValueOf1450209b8_Feature2));
}


bool CSPHacker::_MatchCallChekData(size_t pos)
{
    uint8_t* curPtr = _codeMem + pos;
    if (*curPtr != 0xe8)return false;


    uint32_t jumpOff = static_cast<uint32_t>(pos_CheckData - (pos + 5));
    if (*(uint32_t*)(curPtr + 1) == jumpOff)
    {
        return true;
    }

    return false;
}

bool CSPHacker::_MatchCallCheckLicense(size_t pos)
{
    uint8_t* curPtr = _codeMem + pos;
    if (*curPtr != 0xe8)return false;


    uint32_t jumpOff = static_cast<uint32_t>(pos_CheckLicense - (pos + 5));
    if (*(uint32_t*)(curPtr + 1) == jumpOff)
    {
        return true;
    }
    return false;
}



//发现了不同版本CSP的跳转逻辑不同
//但这几个关键函数的调用顺序是相同的
//我可以忽视间隔的代码，在少量字节中查找到下一个函数调用即为成功
#if 0
bool MatchCodePart1(uint8_t* pos)
{
    // CODE PART 1
    //00000001403AE6AE | 48 : 8D15 D327A304 | lea rdx, qword ptr ds : [144DE0E88] |
    //00000001403AE6B5 | 48 : 8D4D F0 | lea rcx, qword ptr ss : [rbp - 10] |
    // 

   //第一行不进行匹配
    if (*pos != 0x48)return false;
    if (*(pos + 7) != 0x48)return false;



    return true;
}

bool MatchCodePart2(uint8_t* pos)
{
    // CODE PART 2
    //00000001403AE6BE | 84C0 | test al, al |
    //00000001403AE6C0 | 74 1B | je clipstudiopaint.1403AE6DD |




    uint8_t bytes[3] = { 0x84,0xC0,0x74/*,0x1B*/ };
    if (memcmp(pos, bytes, 3) != 0)
        return false;

    return true;
}
bool MatchCodePart3(uint8_t* pos)
{
    //  CODE PART 3
    //00000001403AE6C7 | 48 : 85C0 | test rax, rax |
    //00000001403AE6CA | 74 11 | je clipstudiopaint.1403AE6DD |
    uint8_t bytes[4] = { 0x48,0x85,0xC0,0x74/*,0x11*/ };
    if (memcmp(pos, bytes, 4) != 0)
        return false;

    return true;
}
#endif


bool CSPHacker::MatchLicenseCode(size_t pos)
{

    //if (!MatchCodePart1(pos))return false;
    //pos += (0xb9-0xAE);
    if (!_MatchCallChekData(pos))return false;
    //允许间隔20字节以内的其他代码允许
    pos += 5;
    bool bSuccess = false;
    for (int i = 0; i < 20; i++)
    {
        if (_MatchCallValueOf1450209b8(pos+i))
        {
            pos = pos + i;
            bSuccess = true;
            break;
        }
    }
    if (!bSuccess)return false;
    pos += 5;

    for (int i = 0; i < 20; i++)
    {
        if (_MatchCallValueOf1450209b8(pos + i))
        {
            pos = pos + i;
            bSuccess = true;
            break;
        }
    }
    if (!bSuccess)return false;
    pos += 5;

    for (int i = 0; i < 20; i++)
    {
        if (_MatchCallCheckLicense(pos + i))
        {
            pos = pos + i;
            bSuccess = true;
            break;
        }
    }
    if (!bSuccess)return false;
    pos += 5;

    //对返回值进行比较
    uint8_t bytes2[2] = { 0x85,0xc0 };
    if (memcmp(_codeMem+pos, bytes2, 2) != 0)
        return false;

    return true;
}

void CSPHacker::PatchLicenseCode(size_t pos)
{

    int fillCount=0;
    size_t cmdSetPos = pos;
    for (int i = 5; i < 100; i++)
    {
        if (_MatchCallCheckLicense(pos + i))
        {
            fillCount = i;
            cmdSetPos = pos + i;
            break;
        }
    }
    for (int i = 0; i < fillCount; i++)
    {
        uint8_t* curPos = _codeMem+pos + i;
        *curPos = 0x90;//nop
    }
    //在起始处填充jmp
    *(_codeMem + pos) = 0xeb;//jmp
    *(_codeMem + pos+1) = fillCount-2;
    //将CheckLicense替换为mov eax，1
    
    uint8_t op_mov_eax_1[5] = { 0xb8,0x01,0x00,0x00,0x00 };
    memcpy(_codeMem+ cmdSetPos,op_mov_eax_1,sizeof(op_mov_eax_1));









}



bool CSPHacker::DoBaseHack()
{
    bool anyMatched = false;
    for (size_t i = 0; i < _codeMemSize - 200; i++)
    {


        if (MatchLicenseCode(i))
        {
            printf("success find check point at .text+%llu\n",i);
            PatchLicenseCode(i);
            anyMatched = true;
        }

    }
    if (!anyMatched)
    {
        //如果没有任何匹配到的值
        printf("Can not hack this version of CSP!\n");
    }
    return true;
}

bool CSPHacker::DoJumpStartWindowHack()
{
    //CSP4.2.0之后改成了A才跳过启动窗口
    //有个条件跳跃函数，把入参从0改成9
//    uint8_t jumpFeature[] = {
//    0xC7 ,0x44 ,0x24 ,0x34 ,0x00 ,0x00 ,0x00 ,0x00 ,0x48 ,0x8D ,0x05 ,0xCC ,0xCC,0xCC,0xCC, 0x48 ,0x89 
//    ,0x44 ,0x24 ,0x28 ,0x48 ,0x8D ,0x54 ,0x24 ,0x28 ,0x48 ,0x8D ,0x4C ,0x24 ,0x40,0xE8,0xCC ,0xCC,0xCC,0xCC ,0x8B ,0xD3 ,0x48 ,0x8D ,0x4C ,0x24 ,0x40
//// C7 ,44 ,24 ,34 ,00 ,00 ,00 ,00 ,48 ,8D ,05 ,?? ,??,??,??, 48 ,89 
//    //,44 ,24 ,28 ,48 ,8D ,54 ,24 ,28 ,48 ,8D ,4C ,24 ,40,E8,?? ,??,??,?? ,8B ,D3 ,48 ,8D ,4C ,24 ,40
//    };

    
    //先获取跳转函数
    uint8_t jumpFuncFeature[] = {
        0x48,0x89,0x5C,0x24,0x20,0x57,0x48,0x81,0xEC,0x00,0x01,0x00,0x00,
        0x48,0x8B,0x05,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,0x48,0x33,0xC4,
        0x48,0x89,0x84,0x24,0xF0,0x00,0x00,0x00,0x8B,0xDA,0x48,0x8B,
        0xF9,0x44,0x89,0x44,0x24,0x20,0x48,0x8D,0x4C,0x24,0x40,
        0xE8,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,0x90,0xC7,0x44,0x24,0x34,0x00,0x00,0x00,0x00
    };

    //然后匹配如下特征的代码段以获取目标函数地址
    //00007FF7DDB073B3 | E8 B8813D03 | call clipstudiopaint420.7FF7E0EDF570 |
    //    00007FF7DDB073B8 | EB 13 | jmp clipstudiopaint420.7FF7DDB073CD |
    //    00007FF7DDB073BA | E8 21843D03 | call clipstudiopaint420.7FF7E0EDF7E0 |
    //    00007FF7DDB073BF | 45:8BC4 | mov r8d, r12d |
    //    00007FF7DDB073C2 | 41 : 8BD7 | mov edx, r15d |
    //    00007FF7DDB073C5 | 48 : 8BC8 | mov rcx, rax |
    //    00007FF7DDB073C8 | E8 E3823D03 | call <clipstudiopaint420.默认启动入口> |


    uint8_t startWindowCodeFeature[] = {
0xE8,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,
0xEB,0x13,
0xE8,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,
0x45,0x8B,0xC4,
0x41,0x8B,0xD7,
0x48,0x8B,0xC8,
0xE8,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
    };

    for (uint32_t i = 0; i < _codeMemSize - sizeof(jumpFuncFeature); i++)
    {
        if (_MatchFeatureCode(_codeMem + i, jumpFuncFeature, sizeof(jumpFuncFeature)))
        {
            
            //然后匹配调用特征
            for (uint32_t j = 0; j < _codeMemSize - sizeof(startWindowCodeFeature); j++)
            {
                if (_MatchFeatureCode(_codeMem + j, startWindowCodeFeature, sizeof(startWindowCodeFeature)))
                {
                    uint32_t targetFuncAddr = *(uint32_t*)(_codeMem + j + 1) + j + 5;
                    //将最后一个CALL目标地址改为targetFuncAddr
                    *(uint32_t*)(_codeMem + j + 5 + 2 + 5 + 3 + 3 + 3 + 1) = 
                        targetFuncAddr - (j+5+2+5+3+3+3+5);


                    return true;
                }
            }
        }
    }

    return false;
}

bool CSPHacker::DoJumpHideWindowText()
{
    uint8_t jumpFeature[] = {
        0x7E ,0x57//2
        ,0x48 ,0x8D ,0x8D ,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD//7
        ,0xE8 ,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD//5
        ,0x85 ,0xC0//2
        ,0x7E ,0x34//2
        ,0x48 ,0x8D ,0x15 ,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD//7
        ,0x48 ,0x8D ,0x8D ,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD//7
        ,0xE8 ,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD//5
        ,0x90//1
        ,0x48 ,0x8D ,0x95 ,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD//7
        ,0x48 ,0x8D ,0x8D ,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD//7
        ,0xE8 ,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD//5




        //特征码取太长了 削点
        //,0x90
        //,0x48 ,0x8D ,0x8D ,0x50 ,0x05 ,0x00 ,0x00
        //,0xE8 ,0xCC ,0xCC ,0xCC ,0xCC
        //,0x48 ,0x8D ,0x95 ,0xA8 ,0x03 ,0x00 ,0x00
        //,0x48 ,0x8D ,0x8D ,0x58 ,0x02 ,0x00 ,0x00
        //,0xE8 ,0xCC ,0xCC ,0xCC ,0xCC
        //,0x48 ,0x8D ,0x95 ,0x58 ,0x02 ,0x00 ,0x00


        //4.1.4之前的特征
       //7E ,57
       // ,48 ,8D ,8D ,58 ,02 ,00 ,00
       // ,E8 ,?? ,?? ,?? ,??
       // ,85 ,C0
       // ,7E ,34
       // ,48 ,8D ,15 ,?? ,?? ,?? ,??
       // ,48 ,8D ,8D ,50 ,05 ,00 ,00
       // ,E8 ,?? ,?? ,?? ,??
       // ,90
       // ,48 ,8D ,95 ,50 ,05 ,00 ,00
       // ,48 ,8D ,8D ,58 ,02 ,00 ,00
       // ,E8 ,?? ,?? ,?? ,??
    };

    for (uint32_t i = 0; i < _codeMemSize - sizeof(jumpFeature); i++)
    {
        if (_MatchFeatureCode(_codeMem + i, jumpFeature, sizeof(jumpFeature)))
        {
            //检查rbp+X数值是否相等以辅助判断
            uint32_t value1 = *(uint32_t*)(_codeMem+i+2+3);
            uint32_t value2 = *(uint32_t*)(_codeMem+i+2+7+5+2+2+7+7+5+1+7+3);
            if (value1 != value2)continue;
            value1 = *(uint32_t*)(_codeMem + i + 2 + 7+5+2+2+7+3);
            value2 = *(uint32_t*)(_codeMem + i + 2 + 7+5+2+2+7+7+5+1+3);
            if (value1 != value2)continue;



            *(_codeMem + i) = 0xEB;

            return true;
        }
    }

    return false;
}

bool CSPHacker::DoPluginUnlock()
{
    //TestHack();

	uint8_t pluginFeature[] = {
	0x0F ,0x84 ,0xC3 ,0x03 ,0x00 ,0x00
	,0xE8 ,0xCC ,0xCC ,0xCC ,0xCC
		,0x48 ,0x85 ,0xC0 ,0x0F ,0x84 ,0xB5 ,0x03 ,0x00 ,0x00
		,0xE8  ,0xCC ,0xCC ,0xCC ,0xCC
		,0x48 ,0x8B ,0xC8
	//0F ,0x84 ,0xC3 ,0x03 ,0x00 ,0x00
	//,0xE8 ,0xCC ,0xCC ,0xCC ,0xCC
	//	,0x48 ,0x85 ,0xC0 ,0x0F ,0x84 ,0xB5 ,0x03 ,0x00 ,0x00
	//	,0xE8  ,0xCC ,0xCC ,0xCC ,0xCC
	//	,0x48 ,0x8B ,0xC8
	};


	for (size_t i = 0; i < _codeMemSize - sizeof(pluginFeature); i++)
	{
		if (_MatchFeatureCode(_codeMem + i, pluginFeature, sizeof(pluginFeature)))
		{
            bool anyMatched = false;
            for (size_t j = i - 200; j < i+100; j++)
            {
                if (MatchLicenseCode(j))
                {
                    PatchLicenseCode(j);
                    anyMatched = true;
                }
            }
#if 0
			uint8_t op_jmp[6] = { 0xb8,0x01,0x00,0x00,0x00 ,0x90 };
			memcpy(_codeMem + i, op_jmp, sizeof(op_jmp));
#endif

			return anyMatched;

		}
	}

    return false;
}

//TestHack
bool CSPHacker::DoPatchTimerPoint()
{
	uint8_t pluginFeature[] = {
0x8B ,0x15   ,0xCC ,0xCC ,0xCC ,0xCC
,0x48 ,0x8D ,0x4C ,0x24 ,0x20

,0xE8,0xCC ,0xCC ,0xCC ,0xCC
,0x48 ,0x8D ,0x05 ,0xCC ,0xCC ,0xCC ,0xCC
,0x48 ,0x89 ,0x44 ,0x24 ,0x30
,0x48 ,0x89 ,0x5C ,0x24 ,0x38
,0x0F ,0x28 ,0x44 ,0x24 ,0x30
,0x66 ,0x0F ,0x7F ,0x44 ,0x24 ,0x30
,0x48 ,0x8D ,0x4C ,0x24 ,0x30
	};


	for (size_t i = 0; i < _codeMemSize - sizeof(pluginFeature); i++)
	{
		if (_MatchFeatureCode(_codeMem + i, pluginFeature, sizeof(pluginFeature)))
		{
			//uint8_t op_jmp[5] = { 0x90,0x90,0x90,0x90,0x90 };
            uint8_t cmd_mov_edx_0_nop[6] = { 0xba,0x01,0x00 ,0x00 ,0x00 ,0x90 };
			memcpy(_codeMem + i, cmd_mov_edx_0_nop, sizeof(cmd_mov_edx_0_nop));

			//return true;
		}
	}

    return true;
}


/*
bool CSPHacker::DoJumpUDMKeyCheck()
{
    uint8_t keyCheckFuncFeature[] = {
0x48,0x89,0x5C,0x24,0x20,0x55,0x56,0x57,0x41,0x56,0x41,0x57,0x48,0x8D,0x6C,0x24,0xC9,0x48,0x81,0xEC,0x90,0x00,0x00,0x00,0x48,0x8B,0x05,

0xCC ,0xCC ,0xCC ,0xCC,

0x48,0x33,0xC4,0x48,0x89,0x45,0x27,0x4D,0x8B,0xF0,0x48,0x8B,0xF2,0x48,0x8B,0xD9,0x48,0x89,0x55,0xDF,0x4C,0x89,0x45,0xE7,0xE8,

0xCC ,0xCC ,0xCC ,0xCC,

0x48,0x8B,0xF8,0xFF,0x15
,
0xCC ,0xCC ,0xCC ,0xCC,

0x85,0xC0,0x0F,0x95,0xC1,0x85,0xC0
    };
    
    for (size_t i = 0; i < _codeMemSize - sizeof(keyCheckFuncFeature); i++)
    {
        if (_MatchFeatureCode(_codeMem + i, keyCheckFuncFeature, sizeof(keyCheckFuncFeature)))
        {
            uint8_t cmd_mov_rax_1_ret[8] = { 0x48,0xC7,0xC0 ,0x01 ,0x00 ,0x00,0X00, 0XC3 };
            memcpy(_codeMem + i, cmd_mov_rax_1_ret, sizeof(cmd_mov_rax_1_ret));

            return true;
        }
    }

    return false;
}
*/



bool CSPHacker::_FindCheckDataFunc()
{
    //寻找CheckData函数 
    uint8_t checkDataBody[] = {
    0x48 ,0x8B,0x42 ,0x08
        ,0x48 ,0x2B ,0x41 ,0x08
        ,0x75 ,0x08
        ,0x48 ,0x8B ,0x42 ,0x10
        ,0x48 ,0x2B ,0x41 ,0x10
        ,0x48 ,0x85 ,0xC0
        ,0x0F ,0x94 ,0xC0
        ,0xC3 };



    //48, 8B, 42, 08
    //    , 48, 2B, 41, 08
    //    , 75, 08
    //    , 48, 8B, 42, 10
    //    , 48, 2B, 41, 10
    //    , 48, 85, C0
    //    , 0F, 94, C0
    //    , C3

    for (size_t i = 0; i < _codeMemSize - sizeof(checkDataBody); i++)
    {
        if (_MatchFeatureCode(_codeMem+i, checkDataBody, sizeof(checkDataBody)))
        {
            pos_CheckData =i;
            return true;
        }
    }



    return false;
}

bool CSPHacker::_FindCheckLic()
{
    //特征在4.2.0后发生了变化
    //现在转为调用特征匹配
    // 
    //00007FF68BF19833 | E8 780D1B03 | call <clipstudiopaint.valueof(1450209B8)> |
    //    00007FF68BF19838 | 48:898424 E84E0000 | mov qword ptr ss : [rsp + 4EE8] , rax | rax : EntryPoint
    //    00007FF68BF19840 | 48 : 83BC24 E84E0000 00 | cmp qword ptr ss : [rsp + 4EE8] , 0 |
    //    00007FF68BF19849 | 0F84 A2000000 | je clipstudiopaint.7FF68BF198F1 |
    //    00007FF68BF1984F | E8 5C0D1B03 | call <clipstudiopaint.valueof(1450209B8)> |
    //    00007FF68BF19854 | 48 : 898424 F04E0000 | mov qword ptr ss : [rsp + 4EF0] , rax | rax : EntryPoint
    //    00007FF68BF1985C | 48 : 8B8C24 F04E0000 | mov rcx, qword ptr ss : [rsp + 4EF0] |
    //    00007FF68BF19864 | E8 67451B03 | call <clipstudiopaint.CheckLic> |

    //取这样一个特征并检查调用的函数地址、rsp+X是否相等



    uint8_t checkLicenseFeture[] = {
0xE8,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD,//5
0x48,0x89,0x84,0x24,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD,//8
0x48,0x83,0xBC,0x24,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD,0x00,//9
0x0F,0x84,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD,//6
0xE8,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD,//5
0x48,0x89,0x84,0x24,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD,//8
0x48,0x8B,0x8C,0x24,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD,//8
0xE8,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD ,BYTEWILDCARD //5





//4.1.4及以前的函数头部特征
    //48 ,89 ,5C ,24 ,10
    //,48 ,89 ,6C ,24 ,18
    //    ,56 ,57 ,41 ,56 ,48
    //    ,83 ,EC ,30 ,48 ,8B ,E9 ,48 ,8B ,49 ,20
    //    ,48 ,8B ,01 ,FF ,50 ,38 ,85 ,C0 ,0F ,84 ,C2 ,00 ,00 ,00
    };

    for (uint32_t i = 0; i < _codeMemSize - sizeof(checkLicenseFeture); i++)
    {
        if (_MatchFeatureCode(_codeMem + i, checkLicenseFeture, sizeof(checkLicenseFeture)))
        {

            //检查两次CALL的地址是否相等
            uint32_t value1=*(uint32_t*)(_codeMem + i + 1)+i+5;
            uint32_t value2 = *(uint32_t*)(_codeMem + i + 5 + 8 + 9 + 6 + 1)+i+5+8+9+6+5;
            if (value1 != value2)continue;
            //检查第一组rsp+X是否相等
            value1 = *(uint32_t*)(_codeMem + i + 5+4);
            value2 = *(uint32_t*)(_codeMem + i + 5+8+4);
            if (value1 != value2)continue;
            //检查第二组rsp+X是否相等
            value1 = *(uint32_t*)(_codeMem + i + 5 + 8+9+6+5+4);
            value2 = *(uint32_t*)(_codeMem + i + 5 + 8 + 9+6+5+8+4);
            if (value1 != value2)continue;




            pos_CheckLicense = *(uint32_t*)(_codeMem + i + 5 + 8 + 9 + 6 + 5 + 8+8+1)+i+5+8+9+6+5+8+8+5;
            return true;
        }
    }


    return false;
}

bool CSPHacker::_MatchFeatureCode(uint8_t* dest, uint8_t* fetureCode, size_t codeSize)
{
    for (size_t i = 0; i < codeSize; i++)
    {
        if (dest[i] == fetureCode[i] || fetureCode[i] == 0xcc)
        {
            continue;
        }
        else
        {
            return false;
        }
    }
    return true;
}

