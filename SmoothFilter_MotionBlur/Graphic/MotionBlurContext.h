#ifndef MotionBlurContext_h
#define MotionBlurContext_h


#include <d3d11.h>
#include <cstdint>
#include"../Graphic/Texture.h"

#define MAX_MotionBlur_RADIUS 256

class MotionBlurContext
{
public:
	static MotionBlurContext& GetIns() { static MotionBlurContext ref; return ref; }
	static void ShutDown() { GetIns().motionCS.Reset();  GetIns().pUniformBuffer.Reset();
	};
	


	bool CreateContext();

	void BindAndDraw(
		RenderTexture* renderTarget,
		Texture* orgiTexture,
		Texture* selectAreaTexture,
		float targetX,
		float targetY,
		kBlurDirection diretcion,
		kBlurAlgorithm algorithm
		);




private:
	//bool _Cleanup();//使用了智能指针，不需要clean


	//d3d11需要16字节对齐，我不想在着色器中将寻址复杂化所以构建了一个16字节的结构表示浮点数
	struct BUFFER_POINT
	{
		int offset;
		float value[3]; 
		//BUFFER_FOLOAT& operator=(const float& other)
		//{
		//	this->value = other;
		//	return *this;
		//}
		//operator float() const { // 必须是const成员函数
		//	return static_cast<float>(this->value); // 返回float类型值
		//}
	};

	struct MotionBlurUniform
	{
		float blurDirectionX_normalized;
		float blurDirectionY_normalized;
		//int32_t arrayRadius; //数组半径 始终为正 实际半径的x分量
		int32_t padding; 
		int32_t arrayLen; //数组长度

		float _width;// 1/纹理宽高
		float _height;
		int32_t blockStart;//线程组中一个可绘制的区块的起始索引，可能为arrayRadius，可能为0..
		int32_t xZeroPos;//线程组中，x定位0点的索引，双向为半径处 单向为0或者2倍半径


		BUFFER_POINT motionWeights[MAX_MotionBlur_RADIUS * 2 + 1];
	};
	//计算高斯核
	static void CalcMotionBlurKerner(float strength,float directionX_normal, float directionY_normal, 
		BUFFER_POINT* weightsMemory, kBlurDirection direction,kBlurAlgorithm algorithm,
		int* arrayLen,bool positiveDirection
		);
	static void DoSamplerPoint(BUFFER_POINT* weightsMemory, float x,float y,float weight,int zeroPointIndex );


	bool needInit = true;

	Microsoft::WRL::ComPtr <ID3D11ComputeShader> motionCS;
	Microsoft::WRL::ComPtr <ID3D11ComputeShader> motionCS_Ver;
	Microsoft::WRL::ComPtr <ID3D11Buffer> pUniformBuffer;//像素着色器参数

};


#endif // !1
