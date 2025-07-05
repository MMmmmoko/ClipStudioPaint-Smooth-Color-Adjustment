#include "HookTool.h"
#include <windows.h>
#include <assert.h>

#include"duilib/third_party/libudis86/udis86.h"


namespace CatHook
{
	//ȫ�ֱ���
	uintptr_t baseAddr=0;
	uintptr_t thisModule =0;

	typedef uintptr_t(*GetRspFuncType)();
	uint8_t GetRspCode[4] = {0x48,0x89,0xe0,0xc3};//mov rax,rsp; ret
	uintptr_t(*GetRsp)()=(GetRspFuncType)(void*)GetRspCode;


	void Init(uintptr_t _thisModule)
	{
		//�Ȼ�ȡ��ַƫ��
//Ϊ�˷��������޸�PE�ļ�����ַ�̶�Ϊ140000000���رն�̬��ַ�������·�baseAddr=0x140000000�������޸�IDA pro�еĻ�ַΪ0x140000000������ida pro��x64dbg�еĵ�ַ����ȫ��ͬ��

		baseAddr = (uintptr_t)GetModuleHandle(NULL) - 0x140000000;
		thisModule = _thisModule;




		DWORD oldProtect;
		VirtualProtect(GetRspCode, sizeof(GetRspCode), PAGE_EXECUTE_READWRITE, &oldProtect);

	}


	void CodePatch(void* targetAddr, void* data, size_t datasize)
	{
		DWORD oldProtect;
		VirtualProtect(targetAddr, datasize, PAGE_EXECUTE_READWRITE, &oldProtect);


		memcpy(targetAddr, data, datasize);

		VirtualProtect(targetAddr, datasize, oldProtect, &oldProtect);
	}





	//ע��Ŀǰ��64λ�������//Ϊʲôdbg64�п������ٿ�����һ�Σ���vs�����п�����������
	void Hook(void* targetFuncAddr, void* in_myfunc, void** out_origfunc, int saveCodeByteCount)
	{
		assert(saveCodeByteCount>=12&& saveCodeByteCount <= 48);
		//����һ���ֽ�������Ŀ�꺯����ʼ�����ֽ�
		DWORD oldProtect;
		//VirtualProtect((void*)((uintptr_t)targetFuncAddr-5), saveCodeByteCount+10, PAGE_EXECUTE_READWRITE, &oldProtect);
		VirtualProtect((void*)((uintptr_t)targetFuncAddr-0), saveCodeByteCount, PAGE_EXECUTE_READWRITE, &oldProtect);


		//�޸�Ŀ�꺯����ʼָ��


		//mov rax, address
		//jmp rax
		uint8_t NewCode[48];
		NewCode[0] = 0x48; NewCode[1] = 0xB8;
		*(uintptr_t*)(NewCode + 2) = (uintptr_t)in_myfunc;


		NewCode[10] = 0xFF; NewCode[11] = 0xE0;


		//�ݲ����㸴ԭ�����Բ�����ָ��
		uint8_t* NewFunc = new uint8_t[saveCodeByteCount+12];
		memcpy(NewFunc, targetFuncAddr, saveCodeByteCount);


		NewFunc[saveCodeByteCount]= 0x48; NewFunc[saveCodeByteCount+1] = 0xB8;
		*(uintptr_t*)(NewFunc + saveCodeByteCount + 2) = (uintptr_t)targetFuncAddr + saveCodeByteCount;
		NewFunc[saveCodeByteCount+10]= 0xFF; NewFunc[saveCodeByteCount + 11] = 0xE0;


		*out_origfunc = (void*)NewFunc;
		

		for (int i = 12; i < saveCodeByteCount; i++)
			NewCode[i] = 0x90;//nop
		//memcpy(targetFuncAddr, NewCode, saveCodeByteCount);
		memcpy((void*)((uintptr_t)targetFuncAddr - 0), NewCode, saveCodeByteCount);




		//VirtualProtect((void*)((uintptr_t)targetFuncAddr - 5), saveCodeByteCount + 10, oldProtect, &oldProtect);
		VirtualProtect(targetFuncAddr, saveCodeByteCount, oldProtect, &oldProtect);

		//��out_origfuncִ��Ȩ��
		VirtualProtect(*out_origfunc, saveCodeByteCount + 12,PAGE_EXECUTE_READWRITE,&oldProtect);


	}




	typedef uint8_t _2byteCode[2];
	typedef uint8_t _3byteCode[3];

	_2byteCode MovCode[_HOOKREG_COUNT] = {
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
	_3byteCode JmpCode[_HOOKREG_COUNT] = { 
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

	void HookEx(void* targetFuncAddr, void* in_myfunc, void** out_origfunc, int saveCodeByteCount, _HOOKREG reg)
	{

		int minCode = (reg >= _HOOKREG::_HOOKREG_R8 ? 13 : 12);

			assert(saveCodeByteCount >= minCode && saveCodeByteCount <= 48);

		//����һ���ֽ�������Ŀ�꺯����ʼ�����ֽ�
		DWORD oldProtect;
		//VirtualProtect((void*)((uintptr_t)targetFuncAddr-5), saveCodeByteCount+10, PAGE_EXECUTE_READWRITE, &oldProtect);
		VirtualProtect((void*)((uintptr_t)targetFuncAddr - 0), saveCodeByteCount, PAGE_EXECUTE_READWRITE, &oldProtect);


		//�޸�Ŀ�꺯����ʼָ��


		//mov rax, address
		//jmp rax
		uint8_t NewCode[48];

		//NewCode[0] = 0x48; NewCode[1] = 0xB8;

		*(uint16_t*)NewCode = *(uint16_t*)(MovCode[reg]);


		*(uintptr_t*)(NewCode + 2) = (uintptr_t)in_myfunc;


		
		NewCode[10] = JmpCode[reg][0];
		NewCode[11] = JmpCode[reg][1];
		if(reg>= _HOOKREG::_HOOKREG_R8)
			NewCode[12] = JmpCode[reg][2];


		//�ݲ����㸴ԭ�����Բ�����ָ��
		uint8_t* NewFunc = new uint8_t[saveCodeByteCount + 13];
		memcpy(NewFunc, targetFuncAddr, saveCodeByteCount);



		*(uint16_t*)(NewFunc+ saveCodeByteCount) = *(uint16_t*)(MovCode[reg]);
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

		//��out_origfuncִ��Ȩ��
		VirtualProtect(*out_origfunc, saveCodeByteCount + 13, PAGE_EXECUTE_READWRITE, &oldProtect);
	}


	// ʹ��libudis86����ָ���
	size_t get_instruction_length(void* address, size_t minLength) {

		DWORD oldProtect;
		//VirtualProtect((void*)((uintptr_t)targetFuncAddr-5), saveCodeByteCount+10, PAGE_EXECUTE_READWRITE, &oldProtect);
		VirtualProtect((void*)((uintptr_t)address - 0), 32, PAGE_EXECUTE_READWRITE, &oldProtect);

		ud_t ud_obj;
		ud_init(&ud_obj);
		ud_set_mode(&ud_obj, 64);
		ud_set_syntax(&ud_obj, UD_SYN_INTEL);
		ud_set_input_buffer(&ud_obj, (uint8_t*)address, 32); // ��ȡ�㹻����



		size_t offset = 0;

		char buf[256];
		while (offset < minLength)
		{
			ud_disassemble(&ud_obj);
			size_t len = ud_insn_len(&ud_obj);
			offset += len;
		}


		VirtualProtect((void*)((uintptr_t)address - 0), 32, oldProtect, &oldProtect);

		return offset;
	}


	// ����Ƿ��� jmp rel32 ָ�E9 + ƫ�ƣ�
	bool is_jmp_rel32(uint8_t* addr) {
		return addr[0] == 0xE9;
	}

	// ����Ƿ��� jmp [rip+imm32]��FF 25 + imm32��
	bool is_jmp_rip(uint8_t* addr) {
		return addr[0] == 0xFF && addr[1] == 0x25;
	}

	// ��ȡ jmp rel32 ��Ŀ���ַ
	void* resolve_rel32(uint8_t* addr) {
		int32_t rel = *(int32_t*)(addr + 1);
		return addr + 5 + rel;
	}

	// ��ȡ jmp [rip+imm32] ��Ŀ���ַ
	void* resolve_indirect(uint8_t* addr) {
		int32_t rel = *(int32_t*)(addr + 2);
		uint64_t* target_ptr = (uint64_t*)(addr + 6 + rel);  // 6 �ֽ�ָ��� + ���ƫ��
		return (void*)(*target_ptr);
	}


	void _GenHookParam(void* targetFuncAddrIn, void** realAddr, size_t* cmdLen)
	{
		//���ڱ������Ż���memcpy���ܻ����jmp������VS����תĿ�꺯����ʾ�����������
		//��ʵ��û������ģ���Ϊvs��תĿ�꺯���Ļ����߻������ʱ��Ҳ�������
		//ֻ����������ת�ˣ��۲쵽��ת���ڴ�ͷ���ര����ʾ�ĵ�ַ�Ǻ�targetFuncAddrIn��һ�µġ�
		//�е�����

		DWORD oldProtect;
		VirtualProtect(targetFuncAddrIn, 32, PAGE_EXECUTE_READWRITE, &oldProtect);
		void* _realAddr= targetFuncAddrIn;
		if (is_jmp_rel32((uint8_t*)targetFuncAddrIn))
		{
			_realAddr=resolve_rel32((uint8_t*)targetFuncAddrIn);
		}
		if (is_jmp_rip((uint8_t*)targetFuncAddrIn))
		{
			_realAddr= resolve_indirect((uint8_t*)targetFuncAddrIn);
		}
		*realAddr = _realAddr;
		//Ȼ����_realAddr�ϼ��������
		*cmdLen=get_instruction_length(_realAddr,12);


		VirtualProtect(targetFuncAddrIn, 32, oldProtect, &oldProtect);

	}

	void AutoHook(void* targetFuncAddr, void* in_myfunc, void** out_origfunc)
	{
		//todo ��Ȼ��ʱ�����rax��ͻ������
		//����ָ���
		void* trueAddr;
		size_t cmdLen;
		_GenHookParam(targetFuncAddr,&trueAddr,&cmdLen);
		Hook(trueAddr, in_myfunc, out_origfunc, cmdLen);

	}


}