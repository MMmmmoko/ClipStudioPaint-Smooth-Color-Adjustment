#include<Windows.h>


#include "CSPMOD.h"
#include"AddressTable.h"
#include"FreeTransform.h"
#include"LevelsPreview.h"

#include"TA_HSV.h"
#include"TA_ColorBalance.h"
#include"TA_ToneCurve.h"
#include"TimeLapseExport.h"
#include"CspData.h"
#include"LayerObject.h"
#include"CSPHelper.h"
#include"DUI/Dialog/ColWheelWnd.h"

#include"duilib/third_party/libudis86/udis86.h"
#include"DUI/CatDuiThread.h"

#include"AppSettings.h"

void CSPMOD::OnAttachCSP( uintptr_t moduleHandle)
{
	SDL_Log("OnAttachCSP");
	//abort();
	//SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "CSPMOD Error", "No Version Info in CSPAddressTable.json", nullptr);



	//获取基址
	baseAttr = (uintptr_t)GetModuleHandle(NULL);
	_moduleHandle = moduleHandle;


    if (!AddressTable::LoadOrBuild())
        return;

	bool initResult=SDL_Init(SDL_INIT_VIDEO);
	ASSERT(initResult &&"SDL Video Init Failed!");

	//38CAC74
	//测试UDM的代码
//#if 1
//	uint8_t skipUDMDEBUG[] = {0xB8,0x00,0x00 ,0x00 ,0x00 ,0x90};
//	void* targetPos= (void*)(baseAttr + 0x170A79);
//	CSPMOD::CodePatch(targetPos, skipUDMDEBUG,sizeof(skipUDMDEBUG));
//#endif

	UIMainThread::Start();

	//设置好GetRsp函数
	static uint8_t GetRspCode[4] = { 0x48,0x89,0xe0,0xc3 };//mov rax,rsp; ret

	GetRsp= (uintptr_t(*)())(void*)GetRspCode;
	{
		DWORD oldProtect;
		VirtualProtect(GetRspCode, sizeof(GetRspCode), PAGE_EXECUTE_READWRITE, &oldProtect);
	}


	CSPHelper::Init();
	CspData::Init();
	TA_HSV::Init();
	TA_ColorBalance::Init();
	TA_ToneCurve::Init();
	LevelsPreview::Install();
	TimeLapseExport_hook::Init();
	LayerObject::Init();
	FreeTransform::Install();


	//加载设置项
	AppSettings::GetIns().Load();







}
static bool processStarted = false;
void CSPMOD::OnMainWindowStart()
{
	//一些主窗口运行后的启动项
	if (AppSettings::GetColorWheelOpenWhenStart())
	{
		ColWheelWnd::ShowForm();
	}
	ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, []() {processStarted = true; });

}

void CSPMOD::OnDetachCSP()
{
	if (processStarted&& ui::GlobalManager::Instance().Thread().GetCurrentThreadIdentifier() == ui::kThreadUI)
	{
		processStarted = false;
		//关闭所有UI资源
		bool needSave = false;
		if (ColWheelWnd::GetCurrentForm())
		{
			AppSettings::SetColorWheelOpenWhenStart(true);
			ColWheelWnd::GetCurrentForm()->SavePos();
			needSave = true;
		}
		if(needSave)
			AppSettings::GetIns()._DoSave();
		SDL_Log("OnDetachCSP");
	}
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
static size_t GetInstructionLength(const void* address, size_t minLength)
{
    if (!address)
        return 0;

    ud_t decoder;
    ud_init(&decoder);
    ud_set_mode(&decoder, 64);
    ud_set_syntax(&decoder, UD_SYN_INTEL);
    ud_set_input_buffer(&decoder, static_cast<const uint8_t*>(address), 32);

    size_t length = 0;
    while (length < minLength)
    {
        const size_t instruction = ud_disassemble(&decoder);
        if (!instruction || ud_insn_mnemonic(&decoder) == UD_Iinvalid)
            return 0;
        length += instruction;
    }
    return length;
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
    if (!targetFuncAddr || !myFunc || !out_origfunc || reg < 0 || reg >= _HOOKREG_COUNT)
        return false;

    auto* target = static_cast<uint8_t*>(targetFuncAddr);
    if (is_jmp_rel32(target))
        target = static_cast<uint8_t*>(resolve_rel32(target));
    else if (is_jmp_rip(target))
        target = static_cast<uint8_t*>(resolve_indirect(target));

    const size_t jumpSize = reg >= _HOOKREG_R8 ? 13 : 12;
    const size_t savedSize = GetInstructionLength(target, jumpSize);
    uint8_t replacement[48];
    if (savedSize < jumpSize || savedSize > sizeof(replacement))
        return false;

    const auto writeJump = [reg, jumpSize](uint8_t* code, const void* destination) {
        memcpy(code, MovCode[reg], 2);
        memcpy(code + 2, &destination, sizeof(destination));
        memcpy(code + 10, JmpCode[reg], jumpSize - 10);
    };

    auto* trampoline = static_cast<uint8_t*>(VirtualAlloc(nullptr, savedSize + jumpSize,
        MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
    if (!trampoline)
        return false;
    memcpy(trampoline, target, savedSize);
    writeJump(trampoline + savedSize, target + savedSize);

    DWORD oldProtection = 0;
    if (!VirtualProtect(trampoline, savedSize + jumpSize, PAGE_EXECUTE_READ, &oldProtection) ||
        !FlushInstructionCache(GetCurrentProcess(), trampoline, savedSize + jumpSize) ||
        !VirtualProtect(target, savedSize, PAGE_EXECUTE_READWRITE, &oldProtection))
    {
        VirtualFree(trampoline, 0, MEM_RELEASE);
        return false;
    }

    writeJump(replacement, myFunc);
    memset(replacement + jumpSize, 0x90, savedSize - jumpSize);
    *out_origfunc = trampoline;
    memcpy(target, replacement, savedSize);
    FlushInstructionCache(GetCurrentProcess(), target, savedSize);
    DWORD ignored = 0;
    VirtualProtect(target, savedSize, oldProtection, &ignored);
    return true;
}
