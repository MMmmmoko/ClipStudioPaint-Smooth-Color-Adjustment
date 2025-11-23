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

	void Init(TriglavPlugInServer* runPluginServer);//运行时获取插件服务，和初始化获取的服务好像是不同的

	void OnParamChanged(double blurRadius);

	void ShutDown();

	void Render(float blurRadius);
	void SetSkip(bool bSkip1Frame);//某些情况需要逻辑上跳过1帧
	int64_t GetLastCostTimeMS() { return lastCostTime; };

	static void PluginModuleCleanUp();
private:
	static GaussBlur* pthis;


	bool inited = false;
	bool needSkip = false;
	TriglavPlugInServer* runPluginServer;
	TriglavPlugInBitmapObject renderBitmap = NULL;

	Texture originTexture;
	Texture selectAreaTexture;

	RenderTexture renderTextureStepHor;
	RenderTexture renderTextureStepHorVer;

	int64_t lastCostTime = 0;
};

