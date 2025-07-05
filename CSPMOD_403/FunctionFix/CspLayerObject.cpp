// pch.cpp: 与预编译标头对应的源文件

#include<Windows.h>

#include"../HookTool.h"

#include "CspData.h"
#include "CspLayerObject.h"




//LayerObject
//
// 
// 
// 
// 
// +0x16c 第一bit 是否被选中
// +0x170 第一bit 是否可见
// 
// +0x1E0 指向图层组第一个图层的指针的指针（begin）
// +0x1E8 指向图层组第最后一个的后一个图层的指针的指针（end）
//



//创建菜单进行测试
const int ID_MENU_TEST = 10000;
void LayerObject::SetUpHook()
{
	//好像没办法..创建菜单...
	//HWND hwnd=::GetForegroundWindow();
	//HMENU mainMenu=::GetMenu(hwnd);
	//HMENU editMenu=::GetSubMenu(mainMenu,1);
	//::AppendMenu(editMenu, MF_STRING, ID_MENU_TEST, L"tset");
	////HMENU editMenu=::GetSubMenu(mainMenu,1);


	//MessageBox(hwnd,L"MENUTEST", L"MENUTEST", MB_OK);
}

//构造了一个比较通用的3参数函数（只有一个位置有效），部分调用只有2个参数，但理论上是可以直接调用这个三参数函数的.
void MenuCall(uintptr_t funcAddr)
{
	//菜单入参
	//144f61560+290+20+28
	_FUNC_1F3 _func=(_FUNC_1F3)(CatHook::baseAddr+ funcAddr);

	uintptr_t param[2];
	uintptr_t param0 = *(uintptr_t*)(CatHook::baseAddr + 0x144f61560);
	param0 = *(uintptr_t*)(param0+0x290);
	param0 = *(uintptr_t*)(param0+0x20);
	param0 = *(uintptr_t*)(param0+0x28);
	param[0] = param0;
	param[1] = *(uintptr_t*)(param0+0x30);//和mac不一致，mac这里是0x38

	_func((uintptr_t)param,0,0);

}



LayerObject LayerObject::GetCurrentLayer()
{
	//144F61560 + 290 + 20 + 38 +	(指针地图在这里合拢)  240 + 378
	uintptr_t pLayer = *(uintptr_t*)(CatHook::baseAddr + 0x144f61560);
	pLayer = *(uintptr_t*)(pLayer + 0x290);
	pLayer = *(uintptr_t*)(pLayer + 0x20);
	pLayer = *(uintptr_t*)(pLayer + 0x38); //(指针地图在这里合拢)
	pLayer = *(uintptr_t*)(pLayer + 0x240);
	pLayer = pLayer + 0x378;
	return *(LayerObject*)pLayer;
}

LayerObject LayerObject::GetCurrentRoot()
{
	//windows这边采用的是指针扫描的方法寻找基址，和mac那边关系相差很大
	// 
	// 笔记  找出目标地址，生成目标地址的指针地图，然后重启程序，再找出目标地址，生成目标地址的指针地图。
	//		右键新找到的地址进行指针扫描，基于新的指针地图，并选择参考旧指针地图和旧地址。
	// 
	// 这里选择了和GetCurrentLayer在前几个指针相近的指针路径
	//144F61560 + 290 + 20 + 38 +	(指针地图在这里合拢)  270 + 0+20
	uintptr_t pLayer = *(uintptr_t*)(CatHook::baseAddr + 0x144f61560);
	pLayer = *(uintptr_t*)(pLayer + 0x290);
	pLayer = *(uintptr_t*)(pLayer + 0x20);
	pLayer = *(uintptr_t*)(pLayer + 0x38); //(指针地图在这里合拢)
	pLayer = *(uintptr_t*)(pLayer + 0x270);
	pLayer = *(uintptr_t*)(pLayer + 0x0);
	pLayer = pLayer + 0x20;
	return *(LayerObject*)pLayer;
}



void LayerObject::Duplicate()
{



}

void LayerObject::SetVisibility(bool b)
{
	//函数地址14277FD50  参数1... 1A60C130   参数2 14c1c8


}

void LayerObject::SetLayerMask(bool b)
{

	//函数地址



}





int LayerObject::GetChildLayerCount()
{
	uintptr_t begin = *(uintptr_t*)(ptr + 0x1E0);
	uintptr_t end = *(uintptr_t*)(ptr + 0x1E8);
	return (end-begin)>>4;
}

LayerObject LayerObject::GetChildAt(int index)
{
	uintptr_t begin = *(uintptr_t*)(ptr + 0x1E0);
	return *(LayerObject*)(begin+index*(2*sizeof(uintptr_t)));
}
LayerObject LayerObject::GetChildAt_TopToBottom(int index)
{
	int childCount= GetChildLayerCount();
	int realIndex = childCount - 1 - index;
	return GetChildAt(realIndex);
}



void LayerObject::_MenuOp_SwitchLayerMask()
{
	MenuCall(0x14053D560);
}
