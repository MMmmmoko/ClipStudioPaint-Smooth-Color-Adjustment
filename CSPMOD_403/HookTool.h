#pragma once

#include<iostream>






#define SWITCHADDR_PTR(addr) ((void*)(CatHook::baseAddr + (uintptr_t)addr))

typedef uintptr_t(*_FUNC_1F3)(uintptr_t, uintptr_t, uintptr_t);


namespace CatHook
{
	extern void Init(uintptr_t thisModule);

	extern void CodePatch(void* targetAddr, void* data, size_t datasize);


	//û�ҵ��򵥵����ڷ��ضϵķ����⣬�ɴ��ֶ���...
	//���һ�������鿴����࣬��ֹ12�ֽ�ָ��ض�.

	//��Ϊ�������ƻ�rax�Ĵ������µ�bug�����ֶ�ָ��Ŀ��Ĵ�����...
	enum _HOOKREG
	{
		_HOOKREG_RAX=0,
		_HOOKREG_RBX,
		_HOOKREG_RCX,
		_HOOKREG_RDX,
		_HOOKREG_RSI,
		_HOOKREG_RDI,
		_HOOKREG_R8,
		_HOOKREG_R9,
		_HOOKREG_R10,
		_HOOKREG_R11,
		_HOOKREG_R12,
		_HOOKREG_R13,
		_HOOKREG_R14,
		_HOOKREG_R15,
		_HOOKREG_COUNT
	};
	extern void Hook(void* targetFuncAddr, void* in_myfunc, void** out_origfunc,int saveCodeByteCount);
	extern void HookEx(void* targetFuncAddr, void* in_myfunc, void** out_origfunc,int saveCodeByteCount, _HOOKREG reg= _HOOKREG::_HOOKREG_RAX);
	
	//����nimduilib�е�libudis86
	extern void AutoHook(void* targetFuncAddr, void* in_myfunc, void** out_origfunc);
	extern uintptr_t baseAddr;
	extern uintptr_t thisModule;

	extern uintptr_t (*GetRsp)();

}