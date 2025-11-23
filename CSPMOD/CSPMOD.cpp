#include<Windows.h>


#include "CSPMOD.h"
#include"AddressTable.h"

#include"TA_HSV.h"
#include"TA_ColorBalance.h"
#include"TA_ToneCurve.h"
#include"TimeLapseExport.h"
#include"CspData.h"
#include"LayerObject.h"
#include"CSPHelper.h"


#include"duilib/third_party/libudis86/udis86.h"
#include"DUI/CatDuiThread.h"

#include"AppSettings.h"

void CSPMOD::OnAttachCSP( uintptr_t moduleHandle)
{

	//获取基址
	baseAttr = (uintptr_t)GetModuleHandle(NULL);
	_moduleHandle = moduleHandle;


	UIMainThread::Start();

	//设置好GetRsp函数
	static uint8_t GetRspCode[4] = { 0x48,0x89,0xe0,0xc3 };//mov rax,rsp; ret

	GetRsp= (uintptr_t(*)())(void*)GetRspCode;
	{
		DWORD oldProtect;
		VirtualProtect(GetRspCode, sizeof(GetRspCode), PAGE_EXECUTE_READWRITE, &oldProtect);
	}


	//检查版本
	if (!AddressTable::LoadFromFile())
	{
		return;
	}





	CSPHelper::Init();
	CspData::Init();
	TA_HSV::Init();
	TA_ColorBalance::Init();
	TA_ToneCurve::Init();
	TimeLapseExport_hook::Init();
	LayerObject::Init();


	//加载设置项
	AppSettings::GetIns().Load();









}

void CSPMOD::CodePatch(void* targetAddr, void* data, size_t datasize)
{
	DWORD oldProtect;
	VirtualProtect(targetAddr, datasize, PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(targetAddr, data, datasize);
	VirtualProtect(targetAddr, datasize, oldProtect, &oldProtect);
}







// 检查是否是 jmp rel32 指令（E9 + 偏移）
static bool is_jmp_rel32(uint8_t* addr) {
	return addr[0] == 0xE9;
}

// 检查是否是 jmp [rip+imm32]（FF 25 + imm32）
static bool is_jmp_rip(uint8_t* addr) {
	return addr[0] == 0xFF && addr[1] == 0x25;
}

// 获取 jmp rel32 的目标地址
static void* resolve_rel32(uint8_t* addr) {
	int32_t rel = *(int32_t*)(addr + 1);
	return addr + 5 + rel;
}

// 获取 jmp [rip+imm32] 的目标地址
static void* resolve_indirect(uint8_t* addr) {
	int32_t rel = *(int32_t*)(addr + 2);
	uint64_t* target_ptr = (uint64_t*)(addr + 6 + rel);  // 6 字节指令长度 + 相对偏移
	return (void*)(*target_ptr);
}


// 使用libudis86计算指令长度
size_t get_instruction_length(void* address, size_t minLength) {

	DWORD oldProtect;
	//VirtualProtect((void*)((uintptr_t)targetFuncAddr-5), saveCodeByteCount+10, PAGE_EXECUTE_READWRITE, &oldProtect);
	VirtualProtect((void*)((uintptr_t)address - 0), 32, PAGE_EXECUTE_READWRITE, &oldProtect);

	ud_t ud_obj;
	ud_init(&ud_obj);
	ud_set_mode(&ud_obj, 64);
	ud_set_syntax(&ud_obj, UD_SYN_INTEL);
	ud_set_input_buffer(&ud_obj, (uint8_t*)address, 32); // 读取足够长度



	size_t offset = 0;

	//char buf[256];
	while (offset < minLength)
	{
		ud_disassemble(&ud_obj);
		size_t len = ud_insn_len(&ud_obj);
		offset += len;
	}


	VirtualProtect((void*)((uintptr_t)address - 0), 32, oldProtect, &oldProtect);

	return offset;
}

static void _GenHookParam(void* targetFuncAddrIn, void** realAddr, size_t* cmdLen)
{
	//由于编译器优化，memcpy可能会读出jmp表，但是VS中跳转目标函数显示的正常反汇编
	//其实是没有问题的，因为vs跳转目标函数的汇编或者机器码的时候，也读的这里，
	//只是它帮我跳转了，观察到跳转后内存和反汇编窗口显示的地址是和targetFuncAddrIn不一致的。
	//有点神秘

	DWORD oldProtect;
	VirtualProtect(targetFuncAddrIn, 32, PAGE_EXECUTE_READWRITE, &oldProtect);
	void* _realAddr = targetFuncAddrIn;
	if (is_jmp_rel32((uint8_t*)targetFuncAddrIn))
	{
		_realAddr = resolve_rel32((uint8_t*)targetFuncAddrIn);
	}
	if (is_jmp_rip((uint8_t*)targetFuncAddrIn))
	{
		_realAddr = resolve_indirect((uint8_t*)targetFuncAddrIn);
	}
	*realAddr = _realAddr;
	//然后在_realAddr上计算命令长度
	*cmdLen = get_instruction_length(_realAddr, 12);
	VirtualProtect(targetFuncAddrIn, 32, oldProtect, &oldProtect);
}


typedef uint8_t _2byteCode[2];
typedef uint8_t _3byteCode[3];
static _2byteCode MovCode[CSPMOD::_HOOKREG_COUNT] = {
	{0x48 ,0xB8},
	{0x48 ,0xBB},
	{0x48 ,0xB9},
	{0x48 ,0xBA},
	{0x48 ,0xBE},
	{0x48 ,0xBF},

	{0x49 ,0xB8},
	{0x49 ,0xB9},
	{0x49 ,0xBA},
	{0x49 ,0xBB},
	{0x49 ,0xBC},
	{0x49 ,0xBD},
	{0x49 ,0xBE},
	{0x49 ,0xBF}
};
static _3byteCode JmpCode[CSPMOD::_HOOKREG_COUNT] = {
	{0xFF ,0xE0,0x90},
	{0xFF ,0xE3,0x90},
	{0xFF ,0xE1,0x90},
	{0xFF ,0xE2,0x90},
	{0xFF ,0xE6,0x90},
	{0xFF ,0xE7,0x90},

	{0x41 ,0xFF,0xE0},
	{0x41 ,0xFF,0xE1},
	{0x41 ,0xFF,0xE2},
	{0x41 ,0xFF,0xE3},
	{0x41 ,0xFF,0xE4},
	{0x41 ,0xFF,0xE5},
	{0x41 ,0xFF,0xE6},
	{0x41 ,0xFF,0xE7}
};


bool CSPMOD::Hook(void* targetFuncAddr, void* myFunc, void** out_origfunc, _HOOKREG reg)
{
	//计算指令长度
	void* trueAddr;
	size_t saveCodeByteCount;
	_GenHookParam(targetFuncAddr, &trueAddr, &saveCodeByteCount);
	targetFuncAddr = trueAddr;
	{

		int minCode = (reg >= _HOOKREG::_HOOKREG_R8 ? 13 : 12);

		if(saveCodeByteCount < minCode || saveCodeByteCount > 48)return false;

		//根据一定字节数备份目标函数起始部分字节
		DWORD oldProtect;
		//VirtualProtect((void*)((uintptr_t)targetFuncAddr-5), saveCodeByteCount+10, PAGE_EXECUTE_READWRITE, &oldProtect);
		VirtualProtect((void*)((uintptr_t)targetFuncAddr - 0), saveCodeByteCount, PAGE_EXECUTE_READWRITE, &oldProtect);


		//修改目标函数起始指令


		//mov rax, address
		//jmp rax
		uint8_t NewCode[48];

		//NewCode[0] = 0x48; NewCode[1] = 0xB8;

		*(uint16_t*)NewCode = *(uint16_t*)(MovCode[reg]);


		*(uintptr_t*)(NewCode + 2) = (uintptr_t)myFunc;



		NewCode[10] = JmpCode[reg][0];
		NewCode[11] = JmpCode[reg][1];
		if (reg >= _HOOKREG::_HOOKREG_R8)
			NewCode[12] = JmpCode[reg][2];


		//暂不打算复原，所以不保存指针
		uint8_t* NewFunc = new uint8_t[saveCodeByteCount + 13];
		memcpy(NewFunc, targetFuncAddr, saveCodeByteCount);



		*(uint16_t*)(NewFunc + saveCodeByteCount) = *(uint16_t*)(MovCode[reg]);
		//NewFunc[saveCodeByteCount] = 0x48; NewFunc[saveCodeByteCount + 1] = 0xB8;
		*(uintptr_t*)(NewFunc + saveCodeByteCount + 2) = (uintptr_t)targetFuncAddr + saveCodeByteCount;
		NewFunc[saveCodeByteCount + 10] = JmpCode[reg][0];
		NewFunc[saveCodeByteCount + 11] = JmpCode[reg][1];
		if (reg >= _HOOKREG::_HOOKREG_R8)
			NewFunc[saveCodeByteCount + 12] = JmpCode[reg][2];


		*out_origfunc = (void*)NewFunc;


		for (int i = minCode; i < saveCodeByteCount; i++)
			NewCode[i] = 0x90;//nop
		//memcpy(targetFuncAddr, NewCode, saveCodeByteCount);
		memcpy((void*)((uintptr_t)targetFuncAddr - 0), NewCode, saveCodeByteCount);




		//VirtualProtect((void*)((uintptr_t)targetFuncAddr - 5), saveCodeByteCount + 10, oldProtect, &oldProtect);
		VirtualProtect(targetFuncAddr, saveCodeByteCount, oldProtect, &oldProtect);

		//给out_origfunc执行权限
		VirtualProtect(*out_origfunc, saveCodeByteCount + 13, PAGE_EXECUTE_READWRITE, &oldProtect);
	}
	return true;
}
