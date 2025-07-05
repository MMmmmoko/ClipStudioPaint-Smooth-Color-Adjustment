
#ifndef CspLayerObject_h
#define CspLayerObject_h

#include<iostream>

//本质地址
class LayerObject
{
public:
	static LayerObject GetCurrentLayer();
	static LayerObject GetCurrentRoot();//每一个项目都有一个根节点
	static void SetUpHook();

	void Duplicate();//复制
	void SetSelect();

	std::wstring GetName();

	bool IsLocked();
	bool IsVisible();
	bool HasLayerMask();
	void SetVisibility(bool b);
	void SetLocked(bool b);
	void SetLayerMask(bool b);

	//图层组
	bool IsGroup();
	bool IsGroupOpen();
	bool SetGroupOpen(bool bOpen=true);

	int GetChildLayerCount();//只返回一层，不进行深度遍历， 图层组也算图层
	LayerObject GetChildAt(int index);
	LayerObject GetChildAt_TopToBottom(int index);

	void MoveUp();
	void MoveDown();
	void MergeDown();


private:
	static void _MenuOp_SwitchLayerMask();


public:
	uintptr_t ptr;
	uintptr_t ptr_ref;
};

















#endif
