#ifndef MotionBlur_H
#define MotionBlur_H

#include "TriglavPlugInSDK/TriglavPlugInSDK.h"

#include "Graphic/Texture.h"


enum kBlurDirection
{
	kBlurDirectionFB,
	kBlurDirectionF,
	kBlurDirectionB
};

enum kBlurAlgorithm
{
	kBlurAlgorithmSmooth,
	kBlurAlgorithmAverage
};





class MotionBlur
{
public:
	static MotionBlur& GetIns() {
		if (!pthis)pthis= new MotionBlur;
		return *pthis;
	}

	void Init(TriglavPlugInServer* runPluginServer);//运行时获取插件服务，和初始化获取的服务好像是不同的

	void OnParamChanged(TriglavPlugInPoint point1, TriglavPlugInPoint point2,
		TriglavPlugInBool bPreview,kBlurDirection direction,kBlurAlgorithm algorithm);

	void ShutDown();

	static void PluginModuleCleanUp();
	 

	void Render(float targetX,float targetY,bool preview,kBlurDirection direction, kBlurAlgorithm algorithm);
	void SetSkip(bool bSkip1Frame);//某些情况需要逻辑上跳过1帧


	int64_t GetLastCostTimeMS() { return lastCostTime; };
private:
	static MotionBlur* pthis;

	bool inited = false;
	bool needSkip = false;
	TriglavPlugInServer* runPluginServer;
	TriglavPlugInBitmapObject renderBitmap=NULL;
	TriglavPlugInRect selectAreaRect;


	Texture originTexture;
	Texture selectAreaTexture;

	RenderTexture renderTexture;

	int64_t lastCostTime = 0;
};




#endif
