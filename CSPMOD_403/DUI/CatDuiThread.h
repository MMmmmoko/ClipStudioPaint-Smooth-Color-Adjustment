#pragma once
#include"DuiCommon.h"


/** @class 杂事线程
* @brief 杂事线程类，继承 nbase::FrameworkThread
* @copyright (c) 2015, NetEase Inc. All rights reserved
* @date 2015/1/1
*/
class UIMiscThread : public ui::FrameworkThread
{
public:
	UIMiscThread(ui::ThreadIdentifier thread_id,const wchar_t* name)
		: FrameworkThread(name, thread_id)
		{
	}

	~UIMiscThread(void) {}

private:
	/**
	* 虚函数，初始化线程
	* @return void	无返回值
	*/
	virtual void OnInit() override;

	/**
	* 虚函数，线程退出时，做一些清理工作
	* @return void	无返回值
	*/
	virtual void OnCleanup() override;

private:
	ui::ThreadIdentifier thread_id_;
};

/** @class MainThread
* @brief 主线程（UI线程）类，继承 nbase::FrameworkThread
* @copyright (c) 2015, NetEase Inc. All rights reserved
* @date 2015/1/1
*/
class UIMainThread : public ui::FrameworkThread
{
public:
	UIMainThread() : ui::FrameworkThread(L"MainThread",ui::kThreadUI) {}
	virtual ~UIMainThread() {}



	void StartWithoutLoop() { OnInit(); };
private:
	/**
	* 虚函数，初始化主线程
	* @return void	无返回值
	*/
	virtual void OnInit() override;

	/**
	* 虚函数，主线程退出时，做一些清理工作
	* @return void	无返回值
	*/
	virtual void OnCleanup() override;

private:
	std::unique_ptr<UIMiscThread>	misc_thread_;	// 专门处理杂事的线程
};
