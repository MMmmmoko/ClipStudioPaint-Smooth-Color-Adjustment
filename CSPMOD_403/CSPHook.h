#pragma once






class CSPHook
{
public:
	static void Init(uintptr_t _thisModule);
	static void SetUpHook();

	static uintptr_t GetBaseAddr() { return baseAddr; };

private:
	static bool __fastcall Hook_CheckData(void* p1, void* p2);


	//hookһ��������Ѿ���ȫ�򿪺��ִ�еĺ���
	//��֤һЩ�߼�������򿪺�ſ�ʼִ��
	//����ѡ��ĺ�����һ�� �������������ı�ʱ�ᾭ���ĺ���
	static uintptr_t __fastcall Hook_LateHook(void* p1, void* p2);

	static uintptr_t baseAddr;
};

