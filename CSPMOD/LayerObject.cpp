#include"CSPMOD.h"
#include"AddressTable.h"
#include "LayerObject.h"



//LayerObject
//
// 
// +0x10 第一bit 是否是调整图层
// 
// 
// +0x16c 第一bit 是否被选中 1正常选中 第3bit表示蒙版选中 
// +0x16d 第一bit 蒙版和图层是否有连接 
// CSP貌似有bug，内存中显示，选中蒙版的时候选则其他图层，只将第1bit置0没将第2bit置0
// +0x170 第一bit 是否可见 第2bit 蒙版是否可见
// +0x174 第一bit 是否上锁
// +0x178 第一bit 是否创建了剪辑蒙版
// 
// 
// +0x1A0 第1字节表示图层不透明度，第二字节为1，第1字节为0表示100%
// 
// +0x1BC 第1bit 是否是图层组
// +0x1BC 第5bit 是否打开 0是打开1是关闭
// 
// 
// 
// +0x1E0 指向图层组第一个图层的指针的指针（begin）
// +0x1E8 指向图层组第最后一个的后一个图层的指针的指针（end）
//+0X1EC 第1字节:图层选中flag? 1为正常，3为选中蒙版0为未选中
//




//重要！！！！！！！！！
//LayerObject在原CSP实现中是有引用计数的共享指针.
//在调用原CSP函数的时候，需要所有有引用计数的入参增加引用计数！！

void LayerObject::Init()
{

    layerBaseAddr = AddressTable::GetAddress("LayerObject_Base");
	layer_selectThisLayerFuncAddr = AddressTable::GetAddress("LayerObject_SelectThisLayer_Func");
	layer_duplicateFuncAddr = AddressTable::GetAddress("LayerObject_Duplicate_Func");
	layer_setVisibleFuncAddr = AddressTable::GetAddress("LayerObject_SetVisible_Func");
	layer_moveUpFuncAddr = AddressTable::GetAddress("LayerObject_MoveUp_Func");
	layer_moveDownFuncAddr = AddressTable::GetAddress("LayerObject_MoveDown_Func");
	layer_mergeDownFuncAddr = AddressTable::GetAddress("LayerObject_MergeDown_Func");
	layer_mergeDownPatchAddr = AddressTable::GetAddress("LayerObject_MergeDownPatchAddr");
	layer_mergeSelectFuncAddr = AddressTable::GetAddress("LayerObject_MergeSelect_Func");
	layer_createLayerFuncAddr = AddressTable::GetAddress("LayerObject_CreateLayer_Func");


	layer_getMenuFuncParam2FuncAddr = AddressTable::GetAddress("LayerObject_GetMenuFuncParam2_Func");
	layer_releaseMenuFuncParam2FuncAddr = AddressTable::GetAddress("LayerObject_ReleaseMenuFuncParam2_Func");
	layer_getMenuFuncParam3FuncAddr = AddressTable::GetAddress("LayerObject_GetMenuFuncParam3_Func");
	layer_releaseMenuFuncParam3FuncAddr = AddressTable::GetAddress("LayerObject_ReleaseMenuFuncParam3_Func");

}

bool LayerObject::ApplyEffectAvailable()
{
	return bool(layerBaseAddr && layer_selectThisLayerFuncAddr && layer_duplicateFuncAddr
		&& layer_moveUpFuncAddr && layer_moveDownFuncAddr
		&& layer_mergeDownFuncAddr && layer_mergeDownPatchAddr && layer_mergeSelectFuncAddr
		&& layer_createLayerFuncAddr
		&& layer_getMenuFuncParam2FuncAddr && layer_releaseMenuFuncParam2FuncAddr
		//&& layer_getMenuFuncParam3FuncAddr && layer_releaseMenuFuncParam3FuncAddr
		);

}




LayerObject LayerObject::GetCurrentLayer()
{
	if(!layerBaseAddr)return LayerObject();
	uintptr_t pLayer = *(uintptr_t*)(layerBaseAddr);
	if (!CSPMOD::IsPtrValid(pLayer + 0x290))return LayerObject();
		pLayer = *(uintptr_t*)(pLayer + 0x290);
	if (!CSPMOD::IsPtrValid(pLayer + 0x20))return LayerObject();
		pLayer = *(uintptr_t*)(pLayer + 0x20);
	if (!CSPMOD::IsPtrValid(pLayer + 0x38))return LayerObject();
		pLayer = *(uintptr_t*)(pLayer + 0x38);
	if (!CSPMOD::IsPtrValid(pLayer + 0x240))return LayerObject();
		pLayer = *(uintptr_t*)(pLayer + 0x240);
	if (!CSPMOD::IsPtrValid(pLayer + 0x378))return LayerObject();
	pLayer = pLayer + 0x378;
	
	LayerObject obj= *(LayerObject*)pLayer;
	if (!obj.CheckPtr())return LayerObject();
	return obj;
}

LayerObject LayerObject::GetCurrentRoot()
{
	if (!layerBaseAddr)return LayerObject();
	//windows这边采用的是指针扫描的方法寻找基址，和mac那边关系相差很大
	// 
	// 笔记  找出目标地址，生成目标地址的指针地图，然后重启程序，再找出目标地址，生成目标地址的指针地图。
	//		右键新找到的地址进行指针扫描，基于新的指针地图，并选择参考旧指针地图和旧地址。
	// 
	// 这里选择了和GetCurrentLayer在前几个指针相近的指针路径
	//144F61560 + 290 + 20 + 38 +	(指针地图在这里合拢)  270 + 0+20
	uintptr_t pLayer = *(uintptr_t*)(layerBaseAddr);
	if (!CSPMOD::IsPtrValid(pLayer + 0x290))return LayerObject();
	pLayer = *(uintptr_t*)(pLayer + 0x290);
	
	if (!CSPMOD::IsPtrValid(pLayer + 0x20))return LayerObject();
	pLayer = *(uintptr_t*)(pLayer + 0x20);
	if (!CSPMOD::IsPtrValid(pLayer + 0x38))return LayerObject();
	pLayer = *(uintptr_t*)(pLayer + 0x38);
	if (!CSPMOD::IsPtrValid(pLayer + 0x270))return LayerObject();
	pLayer = *(uintptr_t*)(pLayer + 0x270);

	if (!CSPMOD::IsPtrValid(pLayer+0x0))return LayerObject();
	pLayer = *(uintptr_t*)(pLayer + 0x0);

	if (!CSPMOD::IsPtrValid(pLayer + 0x20))return LayerObject();
	pLayer = pLayer + 0x20;


	LayerObject obj = *(LayerObject*)pLayer;
	if (!obj.CheckPtr())return LayerObject();
	return obj;
}

void* LayerObject::GetCurrentHost()
{
	if (!layerBaseAddr)return nullptr;
	uintptr_t pLayer = *(uintptr_t*)(layerBaseAddr);
	if (!CSPMOD::IsPtrValid(pLayer + 0x1D0))return nullptr;
	pLayer = *(uintptr_t*)(pLayer + 0x1D0);

	if (!CSPMOD::IsPtrValid(pLayer + 0x100))return nullptr;
	pLayer = *(uintptr_t*)(pLayer + 0x100);
	if (!CSPMOD::IsPtrValid(pLayer + 0xA0))return nullptr;
	pLayer = *(uintptr_t*)(pLayer + 0xA0);
	if (!CSPMOD::IsPtrValid(pLayer + 0x88))return nullptr;
	pLayer = *(uintptr_t*)(pLayer + 0x88);

	if (!CSPMOD::IsPtrValid(pLayer + 0x0))return nullptr;
	pLayer = *(uintptr_t*)(pLayer + 0x0);

	return (void*)pLayer;
}


int LayerObject::GetChildLayerCount()
{

	uintptr_t begin = *(uintptr_t*)(ptr + 0x1E0);
	uintptr_t end = *(uintptr_t*)(ptr + 0x1E8);
	return static_cast<int>((end - begin) >> 4);
}

LayerObject LayerObject::GetChildAt(int index)
{
	uintptr_t begin = *(uintptr_t*)(ptr + 0x1E0);
	return *(LayerObject*)(begin + index * (2 * sizeof(uintptr_t)));
}
LayerObject LayerObject::GetChildAt_TopToBottom(int index)
{
	int childCount = GetChildLayerCount();
	int realIndex = childCount - 1 - index;
	return GetChildAt(realIndex);
}

void LayerObject::Duplicate()
{
	if (!layer_duplicateFuncAddr)return;
	_FUNC_1F3 _func = (_FUNC_1F3)(layer_duplicateFuncAddr);
	uintptr_t param[2];

	uintptr_t ptr = *(uintptr_t*)(LayerObject::layerBaseAddr);
	//需要增加引用计数
	uintptr_t refPtr = *(uintptr_t*)(ptr + 0x1D8);
	(*(uint32_t*)(refPtr + 8))++;


	ptr = *(uintptr_t*)(ptr + 0x1D0);
	param[0] = ptr;
	param[1] = *(uintptr_t*)(param[0] + 0x30);
	_func(param, 0, 0);



}

void LayerObject::AddSelect()
{
	(*(uint8_t*)(ptr + 0x16c))|=1;
}

void LayerObject::SetSelect(bool b)
{
	if (layer_selectThisLayerFuncAddr)
	{

			_FUNC_1F4 func = (_FUNC_1F4)layer_selectThisLayerFuncAddr;
			uintptr_t param[2];
			uintptr_t ptr = *(uintptr_t*)(LayerObject::layerBaseAddr);
			//需要增加引用计数
			uintptr_t refPtr = *(uintptr_t*)(ptr + 0x1D8);
			(*(uint32_t*)(refPtr + 8))++;
			this->_AddRef();

			ptr = *(uintptr_t*)(ptr + 0x1D0);
			param[0] = ptr;
			param[1] = *(uintptr_t*)(param[0] + 0x30);



			func(param, this, (void*)(uintptr_t)(b?1:0), 0);




	}
}

std::wstring_view LayerObject::GetName()
{
	return *(wchar_t**)(ptr + 0x138);
}

bool LayerObject::CheckPtr()
{
	//对图层的指针进行一次简单的验证
	//验证逻辑是最远处的内容是否可读

	return CSPMOD::IsPtrValid(ptr,0x1E8);
}

void LayerObject::_AddRef()
{
	if (!ptr_ref)return;
	//uintptr_t refAddr = *(uintptr_t*)ptr_ref;
	(*(uint32_t*)(ptr_ref + 8))++;
}

bool LayerObject::IsLocked()
{
	return (*(uint8_t*)(ptr + 0x174)) & 0x1;
}

bool LayerObject::IsVisible()
{
	return (*(uint8_t*)(ptr + 0x170)) & 0x1;
}
bool LayerObject::IsLayerMaskVisible()
{
	return (*(uint8_t*)(ptr + 0x170)) & 0x2;
}

void LayerObject::SetLayerMaskVisibility(bool b)
{
	if (b)
	{
		(*(uint8_t*)(ptr + 0x170)) |= 0x2;
	}
	else
	{
		(*(uint8_t*)(ptr + 0x170)) &= 0xFD;
	}
}

bool LayerObject::HasLayerClippingMask()
{
	return (*(uint8_t*)(ptr + 0x178)) & 0x1;
}

bool LayerObject::HasLayerMask()
{
	//FIXME/TODO:应该用是否有指针进行判断
	return (*(uint8_t*)(ptr + 0x170)) & 0x2;
}

void LayerObject::SetVisibility(bool b)
{
	if (b)
	{
		(*(uint8_t*)(ptr + 0x170)) |= 0x1;
	}
	else
	{
		(*(uint8_t*)(ptr + 0x170)) &= 0xFE;
	}
	return;

	//使用函数而不是直接在数据中设置值是为了能有画面刷新
	if (!layer_setVisibleFuncAddr|| !layerBaseAddr)return;

	_FUNC_1F2 _func = (_FUNC_1F2)(layer_setVisibleFuncAddr);
	uintptr_t param[2];

	uintptr_t ptr = *(uintptr_t*)(LayerObject::layerBaseAddr);
	//需要增加引用计数
	uintptr_t refPtr = *(uintptr_t*)(ptr + 0x1D8);
	(*(uint32_t*)(refPtr + 8))++;


	ptr = *(uintptr_t*)(ptr + 0x1D0);
	param[0] = ptr;
	param[1] = *(uintptr_t*)(param[0] + 0x30);




	_func(param, (void*)b);



}

void LayerObject::SetLocked(bool b)
{
	//return (*(uint8_t*)(ptr + 0x174)) & 0x1;

	uint8_t state = (*(uint8_t*)(ptr + 0x174));
	if (b)
	{
		state = state | 0x1;
	}
	else
	{
		state = state & 0xFE;

	}
	(*(uint8_t*)(ptr + 0x174)) = state;
}

void LayerObject::SetLayerClippingMask(bool b)
{
	//蒙版函数难以用特征码进行搜索这里用直接设置数值的方式
	uint8_t state = (*(uint8_t*)(ptr + 0x178));
	if (b)
	{
		state = state | 0x1;
	}
	else
	{
		state = state & 0xFE;

	}
	(*(uint8_t*)(ptr + 0x178)) = state;
	//坏了这个不刷新画面的，用代码调用这个后调用向下合并会导致蒙版无效
	//在单帧里尝试修改两次visibility是无效的
	//SetVisibility(!IsVisible());
	//SetVisibility(!IsVisible());

	//尝试将visibleflag修改后再设置可见度失败
	//{
	//	bool isVisible = (*(uint8_t*)(ptr + 0x170))&0x01;
	//	//在第一bit所以直接加减
	//	if (isVisible)
	//	{
	//		(*(uint8_t*)(ptr + 0x170)) -= 1;
	//	}
	//	else
	//	{
	//		(*(uint8_t*)(ptr + 0x170)) += 1;
	//	}
	//	SetVisibility(isVisible);
	//}
	

	//错误的根本不能updown
	//先up后down吧因为处理的图层一定在图层组内，那么必定能up
	//MoveUp();
	//MoveDown();

}

uint16_t LayerObject::GetOpacity()
{
	//0~256
	uint16_t opacity= (*(uint16_t*)(ptr + 0x1A0));
	opacity &= 0x1FF;
	return opacity;
}

void LayerObject::SetOpacity(uint16_t opacity)
{
	opacity &= 0x1FF;
	uint16_t oldOpacityBytes = (*(uint16_t*)(ptr + 0x1A0));
	oldOpacityBytes &= 0xFE00;
	(*(uint16_t*)(ptr + 0x1A0)) = oldOpacityBytes+ opacity;
}

bool LayerObject::IsGroup()
{
	return  (*(uint8_t*)(ptr + 0x1BC))&0x1;
}

bool LayerObject::IsGroupOpen()
{
	return  IsGroup()&&(
		!((*(uint8_t*)(ptr + 0x1BC))&0x10)
		);
}

void LayerObject::SetGroupOpen(bool bOpen)
{
	if (ptr == GetCurrentRoot().ptr)return;
	if (bOpen)
	{
		uint8_t oldValue = *(uint8_t*)(ptr + 0x1BC);
		oldValue &= 0xEF;//1110'1111
		*(uint8_t*)(ptr + 0x1BC)= oldValue;
	}
	else
	{
		uint8_t oldValue = *(uint8_t*)(ptr + 0x1BC);
		oldValue |= 0x10;
		*(uint8_t*)(ptr + 0x1BC) = oldValue;
	}
}

bool LayerObject::IsAdjustLayer()
{
	return *(uint8_t*)(ptr + 0x10) & 0x01;
}

void LayerObject::MoveUp()
{
	//SetSelect();

	uintptr_t param2[400];
	_FUNC_1F1 _funcGetParam2 = (_FUNC_1F1)(layer_getMenuFuncParam2FuncAddr);
	_FUNC_1F1 _funcReleaseParam2 = (_FUNC_1F1)(layer_releaseMenuFuncParam2FuncAddr);
	_funcGetParam2(param2);

	_FUNC_1F2 _func = (_FUNC_1F2)(layer_moveUpFuncAddr);


	uintptr_t param[2];

	uintptr_t ptr = *(uintptr_t*)(LayerObject::layerBaseAddr);
	//需要增加引用计数
	uintptr_t refPtr = *(uintptr_t*)(ptr + 0x1D8);
	(*(uint32_t*)(refPtr + 8))++;


	ptr = *(uintptr_t*)(ptr + 0x1D0);

	param[0] = ptr;
	param[1] = *(uintptr_t*)(param[0] + 0x30);
	//size_t result = 0;
	_func(param, param2);

	_funcReleaseParam2(param2);
}

void LayerObject::MoveDown()
{
	//SetSelect();

	uintptr_t param2[400];
	_FUNC_1F1 _funcGetParam2 = (_FUNC_1F1)(layer_getMenuFuncParam2FuncAddr);
	_FUNC_1F1 _funcReleaseParam2 = (_FUNC_1F1)(layer_releaseMenuFuncParam2FuncAddr);
	_funcGetParam2(param2);

	_FUNC_1F2 _func = (_FUNC_1F2)(layer_moveDownFuncAddr);


	uintptr_t param[2];

	uintptr_t ptr = *(uintptr_t*)(LayerObject::layerBaseAddr);
	//需要增加引用计数
	uintptr_t refPtr = *(uintptr_t*)(ptr + 0x1D8);
	(*(uint32_t*)(refPtr + 8))++;


	ptr = *(uintptr_t*)(ptr + 0x1D0);

	param[0] = ptr;
	param[1] = *(uintptr_t*)(param[0] + 0x30);
	//size_t result = 0;
	_func(param, param2);

	_funcReleaseParam2(param2);
}

void LayerObject::MergeDown()
{
	if(!layer_mergeDownFuncAddr ||!layer_mergeDownPatchAddr)return;


	//调用mergedown之前修改代码以跳过确认图层蒙版的对话框
	uint8_t oldOp[5];
	memcpy(oldOp, layer_mergeDownPatchAddr,sizeof(oldOp));
	uint8_t op_mov_rax_4[5] = {0xB8,0x04,0x00,0x00,0x00};



	CSPMOD::CodePatch(layer_mergeDownPatchAddr, op_mov_rax_4,sizeof(op_mov_rax_4));



	//SetSelect();
	//4.2.0后使用第三个参数会出问题（也有可能是地址没有找对
	// 传入0时不会出问题，干脆不适用param3
	//uintptr_t param3[200];
	//_FUNC_1F1 _funcGetParam3 = (_FUNC_1F1)(layer_getMenuFuncParam3FuncAddr);
	//_FUNC_1F1 _funcReleaseParam3 = (_FUNC_1F1)(layer_releaseMenuFuncParam3FuncAddr);
	
	//_funcGetParam3(param3);

	_FUNC_1F3 _func = (_FUNC_1F3)(layer_mergeDownFuncAddr);
	uintptr_t param[2];
	uintptr_t ptr = *(uintptr_t*)(LayerObject::layerBaseAddr);
	//需要增加引用计数
	uintptr_t refPtr = *(uintptr_t*)(ptr + 0x1D8);
	(*(uint32_t*)(refPtr + 8))++;
	ptr = *(uintptr_t*)(ptr + 0x1D0);
	param[0] = ptr;
	param[1] = *(uintptr_t*)(param[0] + 0x30);

	_func(param,(void*)1, 0);//会在下方有蒙版的时候报错？

	//_funcReleaseParam3(param3);


	//复原代码
	CSPMOD::CodePatch(layer_mergeDownPatchAddr, oldOp, sizeof(oldOp));

}

void LayerObject::MergeSelect()
{
	uintptr_t param[2];

	uintptr_t ptr = *(uintptr_t*)(LayerObject::layerBaseAddr);
	//需要增加引用计数
	uintptr_t refPtr = *(uintptr_t*)(ptr + 0x1D8);
	(*(uint32_t*)(refPtr + 8))++;
	ptr = *(uintptr_t*)(ptr + 0x1D0);
	param[0] = ptr;
	param[1] = *(uintptr_t*)(param[0] + 0x30);
	_FUNC_1F1 _func = (_FUNC_1F1)(layer_mergeSelectFuncAddr);
	_func(param);
}

void LayerObject::CreateLayer()
{
	if (!layer_createLayerFuncAddr)return;
	_FUNC_1F1 _func = (_FUNC_1F1)(layer_createLayerFuncAddr);


	uintptr_t param[2];

	uintptr_t ptr = *(uintptr_t*)(LayerObject::layerBaseAddr);
	//需要增加引用计数
	uintptr_t refPtr = *(uintptr_t*)(ptr + 0x1D8);
	(*(uint32_t*)(refPtr + 8))++;


	ptr = *(uintptr_t*)(ptr + 0x1D0);

	param[0] = ptr;
	param[1] = *(uintptr_t*)(param[0] + 0x30);
	//size_t result = 0;
	_func(param);
}


