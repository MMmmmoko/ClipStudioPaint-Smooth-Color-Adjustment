#pragma once
#include<windows.h>
#include <SDL3/SDL.h>


typedef uint64_t(*_FUNC_1F1)(void*);
typedef uint64_t(*_FUNC_1F2)(void*, void*);
typedef uint64_t(*_FUNC_1F3)(void*, void*, void*);
typedef uint64_t(*_FUNC_1F4)(void*, void*, void*, void*);

class CSPMOD
{
public:
	enum _HOOKREG
	{
		_HOOKREG_RAX = 0,
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
public:
	static void OnAttachCSP(uintptr_t moduleHandle);
	
	static uintptr_t GetBaseAddr() { return baseAttr; };
	static uintptr_t GetDllModule() { return _moduleHandle; };

	static void CodePatch(void* targetAddr,void* data,size_t dataSize);
	static bool Hook(void* targetFuncAddr,void* myFunc, void** out_origfunc, _HOOKREG reg = _HOOKREG::_HOOKREG_RAX);
	static inline uintptr_t(*GetRsp)();
	

	template<typename T, typename PTRTYPE>
	static bool TryGetValue(PTRTYPE ptr,T* valueOut);

	template<typename P>
	static bool IsPtrValid(P ptr,size_t size=8);
private:
	static inline uintptr_t baseAttr;
	static inline uintptr_t _moduleHandle;

};


//bool CSPMOD::IsPtrValid(void* _ptr, size_t size)
//{
//	MEMORY_BASIC_INFORMATION mbi;
//	void* ptr = (void*)_ptr;
//	//size_t size = sizeof(T);
//	if (VirtualQuery(ptr, &mbi, sizeof(mbi)) == 0) {
//		return false; // 查询失败，指针无效
//	}
//
//	// 检查内存状态和保护属性
//	if (mbi.State != MEM_COMMIT || !(mbi.Protect & PAGE_READONLY) && !(mbi.Protect & PAGE_READWRITE)) {
//		return false; // 内存未提交或无读权限
//	}
//
//	// 检查访问范围是否超出内存块边界
//	if (reinterpret_cast<uintptr_t>(ptr) + size > reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize) {
//		return false; // 超出边界
//	}
//	return true;
//}


template<typename P>
bool CSPMOD::IsPtrValid(P _ptr, size_t size)
{
	MEMORY_BASIC_INFORMATION mbi;
	void* ptr = (void*)_ptr;
	//size_t size = sizeof(T);
	if (VirtualQuery(ptr, &mbi, sizeof(mbi)) == 0) {
		return false; // 查询失败，指针无效
	}

	// 检查内存状态和保护属性
	if (mbi.State != MEM_COMMIT || !(mbi.Protect & PAGE_READONLY) && !(mbi.Protect & PAGE_READWRITE)) {
		return false; // 内存未提交或无读权限
	}

	// 检查访问范围是否超出内存块边界
	if (reinterpret_cast<uintptr_t>(ptr) + size > reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize) {
		return false; // 超出边界
	}
	return true;
}

template<typename T, typename PTRTYPE>
bool CSPMOD::TryGetValue(PTRTYPE _ptr, T* valueOut)
{
	MEMORY_BASIC_INFORMATION mbi;
	void* ptr = (void*)_ptr;
	size_t size = sizeof(T);
	if (VirtualQuery(ptr, &mbi, sizeof(mbi)) == 0) {
		return false; // 查询失败，指针无效
	}

	// 检查内存状态和保护属性
	if (mbi.State != MEM_COMMIT || !(mbi.Protect & PAGE_READONLY) && !(mbi.Protect & PAGE_READWRITE)) {
		return false; // 内存未提交或无读权限
	}

	// 检查访问范围是否超出内存块边界
	if (reinterpret_cast<uintptr_t>(ptr) + size > reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize) {
		return false; // 超出边界
	}

	*valueOut = *(T*)ptr;
	return true;

}