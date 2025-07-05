#pragma once
#include"DuiCommon.h"


/** @class �����߳�
* @brief �����߳��࣬�̳� nbase::FrameworkThread
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
	* �麯������ʼ���߳�
	* @return void	�޷���ֵ
	*/
	virtual void OnInit() override;

	/**
	* �麯�����߳��˳�ʱ����һЩ������
	* @return void	�޷���ֵ
	*/
	virtual void OnCleanup() override;

private:
	ui::ThreadIdentifier thread_id_;
};

/** @class MainThread
* @brief ���̣߳�UI�̣߳��࣬�̳� nbase::FrameworkThread
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
	* �麯������ʼ�����߳�
	* @return void	�޷���ֵ
	*/
	virtual void OnInit() override;

	/**
	* �麯�������߳��˳�ʱ����һЩ������
	* @return void	�޷���ֵ
	*/
	virtual void OnCleanup() override;

private:
	std::unique_ptr<UIMiscThread>	misc_thread_;	// ר�Ŵ������µ��߳�
};
