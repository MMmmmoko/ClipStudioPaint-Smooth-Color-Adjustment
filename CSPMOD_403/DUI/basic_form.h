#pragma once

namespace cat
{
	class UiInternalInterface;
}
class PresetProvider;
class BasicForm : public ui::WindowImplBase
{
	friend class cat::UiInternalInterface;
public:
	BasicForm();
	virtual ~BasicForm();

	/**
	 * 一下三个接口是必须要覆写的接口，父类会调用这三个接口来构建窗口
	 * GetSkinFolder		接口设置你要绘制的窗口皮肤资源路径
	 * GetSkinFile			接口设置你要绘制的窗口的 xml 描述文件
	 * GetWindowClassName	接口设置窗口唯一的类名称
	 */
	virtual std::wstring GetSkinFolder() override;
	virtual std::wstring GetSkinFile() override;

	//virtual ui::Control* CreateControl(const std::wstring& controlClass)override;


	/**
	 * 收到 WM_CREATE 消息时该函数会被调用，通常做一些控件初始化的操作
	 */
	virtual void OnInitWindow() override;


	static const std::wstring kClassName;

public:
	bool OnLanguageBtnClicked(ui::EventArgs* msg);
	bool OnLanguageMenuItemClicked(ui::EventArgs* msg);



};

