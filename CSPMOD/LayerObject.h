#pragma once

#include<iostream>
class LayerObject
{
	friend class CSPHelper;
public:
	static void Init();
	static bool ApplyEffectAvailable();



	static LayerObject GetCurrentLayer();
	static LayerObject GetCurrentRoot();//每一个项目都有一个根节点
	static void* GetCurrentHost();
	
	int GetChildLayerCount();//只返回一层，不进行深度遍历， 图层组也算图层
	LayerObject GetChildAt(int index);
	LayerObject GetChildAt_TopToBottom(int index);
	LayerObject GetIndexInRoot(int index);
	
	
	static void Duplicate();//复制
	void SetSelect(bool b=true);
	void AddSelect();

	std::wstring_view GetName();

	bool IsLocked();
	bool IsVisible();
	bool IsLayerMaskVisible();

	bool HasLayerClippingMask();
	bool HasLayerMask();
	
	void SetVisibility(bool b);
	void SetLayerMaskVisibility(bool b);
	void SetLocked(bool b);
	void SetLayerClippingMask(bool b);
	uint16_t GetOpacity();//返回0~256 256表示100%
		void SetOpacity(uint16_t);//输入0~256 256表示100%



	//图层组
	bool IsGroup();
	bool IsGroupOpen();
	void SetGroupOpen(bool bOpen = true);
	bool IsAdjustLayer();


	static void MoveUp();
	static void MoveDown();
	static void MergeDown();
	static void MergeSelect();
	static void CreateLayer();
	//这个功能用于保存原始蒙版信息，但是现在有难以逆向、难以调用的情况
	//更改蒙版策略，尝试复制图层->透明度置0->新建图层->向下合并
	//void LoadSelectAreaFromLayer();



	bool CheckPtr();
	void _AddRef();

	
private:
	//static void _MenuOp_SwitchLayerMask();



	static inline void* layerBaseAddr = 0;
	static inline void* layer_selectThisLayerFuncAddr = 0;
	static inline void* layer_duplicateFuncAddr = 0;
	static inline void* layer_setVisibleFuncAddr = 0;
	static inline void* layer_moveUpFuncAddr = 0;
	static inline void* layer_moveDownFuncAddr = 0;
	static inline void* layer_mergeDownFuncAddr = 0;
	static inline void* layer_mergeDownPatchAddr = 0;
	static inline void* layer_mergeSelectFuncAddr = 0;
	static inline void* layer_createLayerFuncAddr = 0;


	static inline void* layer_getMenuFuncParam2FuncAddr = 0;
	static inline void* layer_releaseMenuFuncParam2FuncAddr = 0;
	static inline void* layer_getMenuFuncParam3FuncAddr = 0;
	static inline void* layer_releaseMenuFuncParam3FuncAddr = 0;



public:
	uintptr_t ptr=0;
	uintptr_t ptr_ref=0;
};