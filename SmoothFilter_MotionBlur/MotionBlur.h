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

	void Init(TriglavPlugInServer* runPluginServer);//����ʱ��ȡ������񣬺ͳ�ʼ����ȡ�ķ�������ǲ�ͬ��

	void OnParamChanged(TriglavPlugInPoint point1, TriglavPlugInPoint point2,
		TriglavPlugInBool bPreview,kBlurDirection direction,kBlurAlgorithm algorithm);

	void ShutDown();

	static void PluginModuleCleanUp();


	void Render(float targetX,float targetY,bool preview,kBlurDirection direction, kBlurAlgorithm algorithm);
	void SetSkip(bool bSkip1Frame);//ĳЩ�����Ҫ�߼�������1֡

private:
	static MotionBlur* pthis;


	bool inited = false;
	bool needSkip = false;
	TriglavPlugInServer* runPluginServer;
	TriglavPlugInBitmapObject renderBitmap=NULL;

	Texture originTexture;
	Texture selectAreaTexture;

	RenderTexture renderTexture;

};




#endif
