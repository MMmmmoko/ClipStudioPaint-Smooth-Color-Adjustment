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
//CSP4.0.3�汾
//����Դ�����Ҫ����
//checkData:ƥ��ĳ����ֵ����128λ�����Ƿ����

//valueof***:�鿴һ���ض���ַ��ֵ������Ƶ�����֡�

//��ȡ��ǰʱ�䣺  һ����ȡ��ǰʱ��ĺ����������ڷ�������ʱ�йص��߼�

static bool rtrue = false;


//ԭ�����϶���12�ֽ�����С��ת����ֱ����д��������ԭ������
static bool(*orig_CheckData)(void* p1, void* p2)=NULL;
__declspec(noinline) bool __fastcall CSPHook::Hook_CheckData(void* p1, void* p2)
{
	//void* testValue = (void*)(CatHook::baseAddr + 0x144de5270);

	//if (rtrue)return true;

	//���桢�زġ���ӡ������
	if ((void*)(CatHook::baseAddr + 0x144ddfea0) == p2)return true;//�ز�1
	if ((void*)(CatHook::baseAddr + 0x144E1A860) == p2)return true;//�ز�2

	if ((void*)(CatHook::baseAddr + 0x144dE0E88) == p2)return true;
	if ((void*)(CatHook::baseAddr + 0x144de5270) == p2)return true;//����

	if ((void*)(CatHook::baseAddr + 0x144de6f80) == p2)return true;//��������

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
	//	//ִ��һЩ��Ҫ�ӳ�ִ�д���

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



#pragma region ����hook

	////�����ļ�//��ͨ��������ʽ�޸�
	////
	//// ֮ǰ��Mac�ϵ�Ѱ��˼·�Ƿֱ�����һ�������һ��������CSP���������Ի������ϵ㣬���е��Ի���ʱ�鿴���ö�ջ��������һ�������ĵ��ô��϶ϵ㣬
	//// Ȼ����������CSP����checkData����ǿ�Ʒ���1����������ĸ��ط�������淢���˷��룬������봦�����ж��Ƿ��ܱ���Ĵ��븽����
	//// windows�������CreateWindow��API�ϴ��˶ϵ㣬�ڵ��ö�ջ�ϻ��ݶԱ�Ѱ���߼���Mac�б�����븽�����߼����Ƶĵط���
	//// Ѱ�ҵ������ǳ����õĺ���checkData��valueof******����ܸ��öԱ��ˡ�
	////0x14049CC80����Ϊ jmp 14049CD08
	//// E9 83 00 00 00
	////����48 89 9D 38 02 00 00 0F 10 80 50 01 00 00 0F 11 85 40 02 00 00
	//// 
	////uint8_t op_JMP_14049CD08[5] = { 0xE9,0x83,0x00,0x00,0x00 };
	////CatHook::CodePatch((void*)(baseAddr+ 0x14049CC80), op_JMP_14049CD08,sizeof(op_JMP_14049CD08));



	//������ʼѡ��汾�Ĵ���
	// 
	//Ѱ��˼·�����ҵ������ͳ�ʼ��������ѡ��Ի�������ʱ��ֱ����ͣ�����ڵ��ö�ջ�д��ͳ�ʼ����������һ��������ĩβ��
	//0x1402B2C18����Ϊ call 0x1434DA340
	//���� 4C 8D 9C 24 B0 03 00 00 49 8B 5B 38 49 8B 73 40 49 8B E3 41 5F 41 5E 41 5D 41 5C 5F C3
	uint8_t op_Call_SkipSelectWindow[5] = { 0xE8,0x23,0x77,0x22,0x03 };
	CatHook::CodePatch((void*)(baseAddr + 0x1402B2C18), op_Call_SkipSelectWindow, sizeof(op_Call_SkipSelectWindow));
	//û�о�������ݣ���Ҫ�ǰ�������֤���߼����뵽����
	//CatHook::Hook((void*)(CatHook::baseAddr + 0x1434DA340), (void*)Hook_Skip_SelectWindow, (void**)&orig_Skip_SelectWindow, 13);




	//�������������
	//
	// 0x1403D00FC����Ϊjmp 1403D0155
	//Ѱ��˼·���������ʼ��ʱ�������������ǰ��ʹ��CE����������棺�޷�ʹ�ò��ֹ��ܡ��ַ������ҵ�һ�����ӡ�����棺�޷�ʹ�ò��ֹ��ܡ��͡� - CLIP STUDIO PAINT EX���ĺ���
	//�������и��Ƿ���ڡ�����棺****���ַ������жϣ��ó������ж�ʱ��ת����������������֡�
	//����49 8B 87 18 03 00 00 48 89 45 C0 49 8B 87 20 03 00 00 48 89 45 C8 48 85 C0 74 06 F0 44 0F C1 70 08 48 8D 95 48 03 00 00 48 8D 4D C0
	//���������¶ϵ㣬���������ǹ���������ַ�����˵���ҵ���Ŀ�꺯����  Ŀ���޸�λ���������·�
	uint8_t op_Skip_Trial_String[2] = { 0xEB,0x57 };
	CatHook::CodePatch((void*)(baseAddr + 0x1403D00FC), op_Skip_Trial_String, sizeof(op_Skip_Trial_String));




	//�����������
	//
	//0x1402D446B����Ϊjmp 1402D450B
	//Ѱ��˼·������"cpm"����".cpm"�ַ��������µ�checkData������Ϊ��֧���ڴ�����G��X64DBGͼ�߷�֧��ͼ�������Կ����߼�������������֧��ֱ����ת���Ҳ��֧���ɡ�
	//����90 0F 57 C0 F3 0F 7F 45 D8 4C 89 6D E8 F3 0F 7F 45 F0 4C 89 6D 00 F3 0F 7F 45 08 4C 89 6D 18 F3 0F 7F 45 20 4C 89 6D 30
	uint8_t op_Skip_Plugin_Check[5] = { 0xe9,0x9b,0x00,0x00,0x00 };
	CatHook::CodePatch((void*)(baseAddr + 0x1402D446B), op_Skip_Plugin_Check, sizeof(op_Skip_Plugin_Check));




	////0x1403A3BC0
	//Ѱ��˼·��CEѰ�ҡ�����������������д��ϵ㡣
	//����48 8B 39 48 B8 FF FF FF FF FF FF FF 07 48 39 41 08 74 76 48 89 4C 24 20 48 C7 44 24 28 00 00 00 00 48 8D 4C 24 30
	CatHook::Hook((void*)(CatHook::baseAddr + 0x1403A3BC0), (void*)CSPHook::Hook_LateHook, (void**)&orig_LateHook, 15);



	//�ӹ�CheckData����һЩ����
	//CatHook::AutoHook((void*)(CatHook::baseAddr + 0x143567B10), (void*)CSPHook::Hook_CheckData, (void**)&orig_CheckData);







	//������֤
	// ��֤��ʱ���������ÿ�ζ����֣��������ҵ��� mac�Ǳ�ûע�⵽�������
	//
	//��1434dddd0���ĺ�������1
	// 
	//uint8_t op_mov_eax_1_ret[10] = { 
	//	0xb8,0x01,0x00,0x00,0x00, //mov eax,1
	// 0xc3,0x90,0x90,0x90,0x90 };//ret  nop nop nop nop
	//CatHook::CodePatch((void*)(baseAddr + 0x1434dddd0), op_mov_eax_1_ret, sizeof(op_mov_eax_1_ret));
	////����������ĵط�����1�ᵼ���޷��������������
	//uint8_t op_jmp_14032A0D6[2] = { 0XEB,0X6D };
	//CatHook::CodePatch((void*)(baseAddr + 0x14032A067), op_jmp_14032A0D6, sizeof(op_jmp_14032A0D6));
	//uint8_t op_jmp_14032A1AB_nop[6] = { 0XE9,0X8B,0X00,0X00,0X00,0X90 };
	//CatHook::CodePatch((void*)(baseAddr + 0x14032A11B), op_jmp_14032A1AB_nop, sizeof(op_jmp_14032A1AB_nop));








#pragma endregion




#pragma region ��������hook


	//�����˾���ģʽ�Ի�������ʱ��ʵ�ֲ����˾�ʵʱԤ��
	//
	//�ǰ�Mac�����ҵ�..
	uint8_t cmd_nop_nop[2] = { 0x90 ,0x90 };
	CatHook::CodePatch((void*)(CatHook::baseAddr + 0x142DE2DD3), cmd_nop_nop, sizeof(cmd_nop_nop));





	//��ɫ���hook
	HSV_hook::SetUpHook();
	ColorBalance_hook::SetUpHook();
	ToneCurve_hook::SetUpHook();


	//����CSP����
	TimeLapseExport_hook::SetUpHook();
	CspData::SetUpHook();



#pragma endregion









}




