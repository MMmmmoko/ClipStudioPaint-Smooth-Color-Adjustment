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
	 * һ�������ӿ��Ǳ���Ҫ��д�Ľӿڣ����������������ӿ�����������
	 * GetSkinFolder		�ӿ�������Ҫ���ƵĴ���Ƥ����Դ·��
	 * GetSkinFile			�ӿ�������Ҫ���ƵĴ��ڵ� xml �����ļ�
	 * GetWindowClassName	�ӿ����ô���Ψһ��������
	 */
	virtual std::wstring GetSkinFolder() override;
	virtual std::wstring GetSkinFile() override;

	//virtual ui::Control* CreateControl(const std::wstring& controlClass)override;


	/**
	 * �յ� WM_CREATE ��Ϣʱ�ú����ᱻ���ã�ͨ����һЩ�ؼ���ʼ���Ĳ���
	 */
	virtual void OnInitWindow() override;


	static const std::wstring kClassName;

public:
	bool OnLanguageBtnClicked(ui::EventArgs* msg);
	bool OnLanguageMenuItemClicked(ui::EventArgs* msg);



};

