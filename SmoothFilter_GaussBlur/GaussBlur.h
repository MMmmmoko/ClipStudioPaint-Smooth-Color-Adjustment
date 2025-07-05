#pragma once




#include "TriglavPlugInSDK/TriglavPlugInSDK.h"

#include "Graphic/Texture.h"
class GaussBlur
{
public:
	static GaussBlur& GetIns() {
		if (!pthis)pthis = new GaussBlur;
		return *pthis;
	}

	void Init(TriglavPlugInServer* runPluginServer);//����ʱ��ȡ������񣬺ͳ�ʼ����ȡ�ķ�������ǲ�ͬ��

	void OnParamChanged(double blurRadius);

	void ShutDown();

	void Render(float blurRadius);
	void SetSkip(bool bSkip1Frame);//ĳЩ�����Ҫ�߼�������1֡


	static void PluginModuleCleanUp();
private:
	static GaussBlur* pthis;


	bool inited = false;
	bool needSkip = false;
	TriglavPlugInServer* runPluginServer;
	TriglavPlugInBitmapObject renderBitmap=NULL;

	Texture originTexture;
	Texture selectAreaTexture;

	RenderTexture renderTextureStepHor;
	RenderTexture renderTextureStepHorVer;

};

