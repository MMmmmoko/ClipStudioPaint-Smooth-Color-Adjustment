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
	struct BUFFER_FOLOAT
	{
		float value;
		float padding[3];
		BUFFER_FOLOAT& operator=(const float& other)
		{
			this->value = other;
			return *this;
		}
		operator float() const { // 必须是const成员函数
			return static_cast<float>(this->value); // 返回float类型值
		}
	};

	struct MotionBlurUniform
	{
		float blurDirectionX_normalized;
		float blurDirectionY_normalized;
		int32_t radius;
		int32_t bDirectionFB;

		float _width;
		float _height;
		float padding1;
		float padding2;


		BUFFER_FOLOAT motionWeights[MAX_MotionBlur_RADIUS * 2 + 1 + 1];//D3d11数组需要16字节对齐.....
	};
	//计算高斯核
	static void CalcMotionBlurKerner(float strength, BUFFER_FOLOAT* weightsMemory, kBlurDirection direction,kBlurAlgorithm algorithm);



	bool needInit = true;

	Microsoft::WRL::ComPtr <ID3D11ComputeShader> motionCS;
	Microsoft::WRL::ComPtr <ID3D11Buffer> pUniformBuffer;//像素着色器参数

};


#endif // !1
