#include "pch.h"


#include<iostream>
#include<string>
#include "CSPHook.h"
#include"HookTool.h"


#include"ToneAdjustment/HSV_Hook.h"
#include"ToneAdjustment/ColorBalance_Hook.h"
#include"ToneAdjustment/ToneCurve_Hook.h"


#include"FunctionFix/TimeLapseExport.h"
#include"FunctionFix/CspLayerObject.h"
#include"FunctionFix/CspData.h"

#include"DUI/CatDuiThread.h"

uintptr_t CSPHook::baseAddr=NULL;
//CSP4.0.3版本
//分析源码的重要函数
//checkData:匹配某个数值其后的128位数字是否相等

//valueof***:查看一个特定地址的值，极其频繁出现。

//获取当前时间：  一个获取当前时间的函数，有利于分析与延时有关的逻辑

static bool rtrue = false;


//原函数较短且12字节内有小跳转，就直接重写，不调用原函数了
static bool(*orig_CheckData)(void* p1, void* p2)=NULL;
__declspec(noinline) bool __fastcall CSPHook::Hook_CheckData(void* p1, void* p2)
{
	//void* testValue = (void*)(CatHook::baseAddr + 0x144de5270);

	//if (rtrue)return true;

	//保存、素材、打印、漫画
	if ((void*)(CatHook::baseAddr + 0x144ddfea0) == p2)return true;//素材1
	if ((void*)(CatHook::baseAddr + 0x144E1A860) == p2)return true;//素材2

	if ((void*)(CatHook::baseAddr + 0x144dE0E88) == p2)return true;
	if ((void*)(CatHook::baseAddr + 0x144de5270) == p2)return true;//保存

	if ((void*)(CatHook::baseAddr + 0x144de6f80) == p2)return true;//拍摄姿势

	//return true;

	return *(uintptr_t*)((uintptr_t)p1 + 8) == *(uintptr_t*)((uintptr_t)p2 + 8)
		&& *(uintptr_t*)((uintptr_t)p1 + 16) == *(uintptr_t*)((uintptr_t)p2 + 16);
}





static UIMainThread* pMainThread = NULL;
static uintptr_t(*orig_LateHook)(void* p1, void* p2) = NULL;
uintptr_t __fastcall CSPHook::Hook_LateHook(void* p1, void* p2)
{
	auto result=orig_LateHook(p1, p2);
	LayerObject::SetUpHook();
	//static int hasDone = 0;
	//if (hasDone<20)
	//{
	//	hasDone++;
	//}
	//else if (hasDone < 21)
	//{
	//	//执行一些需要延迟执行代码

	//	LayerObject::SetUpHook();
	//	hasDone++;
	//}




	if (!pMainThread)
	{
		pMainThread = new UIMainThread();
		pMainThread->StartWithoutLoop();
	}


	return result;
}







void CSPHook::Init(uintptr_t _thisModule)
{
	CatHook::Init(_thisModule);
	baseAddr = CatHook::baseAddr;
}

void CSPHook::SetUpHook()
{



#pragma region 基础hook

	////保存文件//已通过其他方式修改
	////
	//// 之前在Mac上的寻找思路是分别运行一份正版和一份体验版的CSP，体验版给对话框打个断点，运行到对话框时查看调用堆栈，对其上一定数量的调用打上断点，
	//// 然后运行正版CSP（或将checkData函数强制返回1），检查在哪个地方和体验版发生了分离，这个分离处就是判断是否能保存的代码附近。
	//// windows这边是在CreateWindow的API上打了断点，在调用堆栈上回溯对比寻找逻辑和Mac中保存代码附近的逻辑相似的地方。
	//// 寻找到两个非常常用的函数checkData和valueof******后就能更好对比了。
	////0x14049CC80处改为 jmp 14049CD08
	//// E9 83 00 00 00
	////特征48 89 9D 38 02 00 00 0F 10 80 50 01 00 00 0F 11 85 40 02 00 00
	//// 
	////uint8_t op_JMP_14049CD08[5] = { 0xE9,0x83,0x00,0x00,0x00 };
	////CatHook::CodePatch((void*)(baseAddr+ 0x14049CC80), op_JMP_14049CD08,sizeof(op_JMP_14049CD08));



	//跳过初始选择版本的窗口
	// 
	//寻找思路：先找到超大型初始化函数，选择对话框工作的时候直接暂停程序，在调用堆栈中大型初始化函数的下一级函数的末尾。
	//0x1402B2C18处改为 call 0x1434DA340
	//特征 4C 8D 9C 24 B0 03 00 00 49 8B 5B 38 49 8B 73 40 49 8B E3 41 5F 41 5E 41 5D 41 5C 5F C3
	uint8_t op_Call_SkipSelectWindow[5] = { 0xE8,0x23,0x77,0x22,0x03 };
	CatHook::CodePatch((void*)(baseAddr + 0x1402B2C18), op_Call_SkipSelectWindow, sizeof(op_Call_SkipSelectWindow));
	//没有具体改内容，主要是把正版验证的逻辑插入到这理
	//CatHook::Hook((void*)(CatHook::baseAddr + 0x1434DA340), (void*)Hook_Skip_SelectWindow, (void**)&orig_Skip_SelectWindow, 13);




	//隐藏体验版文字
	//
	// 0x1403D00FC处改为jmp 1403D0155
	//寻找思路：在软件初始化时，进入软件界面前，使用CE搜索“体验版：无法使用部分功能”字符串，找到一个链接“体验版：无法使用部分功能”和“ - CLIP STUDIO PAINT EX”的函数
	//函数中有个是否存在“体验版：****”字符串的判断，让程序在判断时跳转就能消除体验版文字。
	//特征49 8B 87 18 03 00 00 48 89 45 C0 49 8B 87 20 03 00 00 48 89 45 C8 48 85 C0 74 06 F0 44 0F C1 70 08 48 8D 95 48 03 00 00 48 8D 4D C0
	//特征函数下断点，如果处理的是关于体验版字符串，说明找到了目标函数。  目标修改位置在特征下方
	uint8_t op_Skip_Trial_String[2] = { 0xEB,0x57 };
	CatHook::CodePatch((void*)(baseAddr + 0x1403D00FC), op_Skip_Trial_String, sizeof(op_Skip_Trial_String));




	//解锁插件功能
	//
	//0x1402D446B处改为jmp 1402D450B
	//寻找思路：搜索"cpm"或者".cpm"字符串，其下的checkData函数即为分支所在处。按G打开X64DBG图线分支视图可以明显看到逻辑有左右两个分支，直接跳转到右侧分支即可。
	//特征90 0F 57 C0 F3 0F 7F 45 D8 4C 89 6D E8 F3 0F 7F 45 F0 4C 89 6D 00 F3 0F 7F 45 08 4C 89 6D 18 F3 0F 7F 45 20 4C 89 6D 30
	uint8_t op_Skip_Plugin_Check[5] = { 0xe9,0x9b,0x00,0x00,0x00 };
	CatHook::CodePatch((void*)(baseAddr + 0x1402D446B), op_Skip_Plugin_Check, sizeof(op_Skip_Plugin_Check));




	////0x1403A3BC0
	//寻找思路，CE寻找“画布窗口数”，打写入断点。
	//特征48 8B 39 48 B8 FF FF FF FF FF FF FF 07 48 39 41 08 74 76 48 89 4C 24 20 48 C7 44 24 28 00 00 00 00 48 8D 4C 24 30
	CatHook::Hook((void*)(CatHook::baseAddr + 0x1403A3BC0), (void*)CSPHook::Hook_LateHook, (void**)&orig_LateHook, 15);



	//接管CheckData用于一些测试
	//CatHook::AutoHook((void*)(CatHook::baseAddr + 0x143567B10), (void*)CSPHook::Hook_CheckData, (void**)&orig_CheckData);







	//正版验证
	// 验证的时候这个函数每次都出现，很容易找到， mac那边没注意到这个函数
	//
	//让1434dddd0处的函数返回1
	// 
	//uint8_t op_mov_eax_1_ret[10] = { 
	//	0xb8,0x01,0x00,0x00,0x00, //mov eax,1
	// 0xc3,0x90,0x90,0x90,0x90 };//ret  nop nop nop nop
	//CatHook::CodePatch((void*)(baseAddr + 0x1434dddd0), op_mov_eax_1_ret, sizeof(op_mov_eax_1_ret));
	////有两处特殊的地方返回1会导致无法启动软件，跳过
	//uint8_t op_jmp_14032A0D6[2] = { 0XEB,0X6D };
	//CatHook::CodePatch((void*)(baseAddr + 0x14032A067), op_jmp_14032A0D6, sizeof(op_jmp_14032A0D6));
	//uint8_t op_jmp_14032A1AB_nop[6] = { 0XE9,0X8B,0X00,0X00,0X00,0X90 };
	//CatHook::CodePatch((void*)(baseAddr + 0x14032A11B), op_jmp_14032A1AB_nop, sizeof(op_jmp_14032A1AB_nop));








#pragma endregion




#pragma region 其他功能hook


	//加速滤镜的模式对话框跳出时间实现部分滤镜实时预览
	//
	//是按Mac特征找的..
	uint8_t cmd_nop_nop[2] = { 0x90 ,0x90 };
	CatHook::CodePatch((void*)(CatHook::baseAddr + 0x142DE2DD3), cmd_nop_nop, sizeof(cmd_nop_nop));





	//调色相关hook
	HSV_hook::SetUpHook();
	ColorBalance_hook::SetUpHook();
	ToneCurve_hook::SetUpHook();


	//其他CSP功能
	TimeLapseExport_hook::SetUpHook();
	CspData::SetUpHook();



#pragma endregion









}




