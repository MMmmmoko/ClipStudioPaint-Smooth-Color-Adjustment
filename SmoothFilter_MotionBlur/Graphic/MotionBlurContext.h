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
	//bool _Cleanup();//ʹ��������ָ�룬����Ҫclean


	//d3d11��Ҫ16�ֽڶ��룬�Ҳ�������ɫ���н�Ѱַ���ӻ����Թ�����һ��16�ֽڵĽṹ��ʾ������
	struct BUFFER_FOLOAT
	{
		float value;
		float padding[3];
		BUFFER_FOLOAT& operator=(const float& other)
		{
			this->value = other;
			return *this;
		}
		operator float() const { // ������const��Ա����
			return static_cast<float>(this->value); // ����float����ֵ
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


		BUFFER_FOLOAT motionWeights[MAX_MotionBlur_RADIUS * 2 + 1 + 1];//D3d11������Ҫ16�ֽڶ���.....
	};
	//�����˹��
	static void CalcMotionBlurKerner(float strength, BUFFER_FOLOAT* weightsMemory, kBlurDirection direction,kBlurAlgorithm algorithm);



	bool needInit = true;

	Microsoft::WRL::ComPtr <ID3D11ComputeShader> motionCS;
	Microsoft::WRL::ComPtr <ID3D11Buffer> pUniformBuffer;//������ɫ������

};


#endif // !1
