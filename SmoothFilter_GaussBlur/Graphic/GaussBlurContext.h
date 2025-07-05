#ifndef GaussBlurContext_h
#define GaussBlurContext_h


#include <d3d11.h>
#include <cstdint>
#include"../Graphic/Texture.h"

#define MAX_GAUSSBLUR_RADIUS 256

class GaussBlurContext
{
public:
	static GaussBlurContext& GetIns() { static GaussBlurContext ref; return ref; }
	static void ShutDown() {
		GetIns().gaussCSHor.Reset(); GetIns().gaussCSVer.Reset(); GetIns().pUniformBuffer.Reset()
			;
	}



	bool CreateContext();

	void BindAndDraw(
		RenderTexture* renderTargetStep1,
		RenderTexture* renderTargetStep2,
		Texture* orgiTexture,
		Texture* selectAreaTexture,
		float blurRadius
		);




private:
	//bool _Cleanup();//ʹ��������ָ�룬����Ҫclean

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

	struct GaussBlurUniform
	{
		int32_t blurRadius;//��˹ģ�����ĵİ뾶
		float _width;
		float _height;
		float _padding;

		BUFFER_FOLOAT gaussWeights[MAX_GAUSSBLUR_RADIUS * 2 + 1 + 1];//D3d11������Ҫ16�ֽڶ���.....
	};
	//�����˹��
	static void CalcGaussBlurKerner(float radius, BUFFER_FOLOAT* weightsMemory, int* radius_out);



	bool needInit = true;

	Microsoft::WRL::ComPtr <ID3D11ComputeShader> gaussCSHor;
	Microsoft::WRL::ComPtr <ID3D11ComputeShader> gaussCSVer;


	Microsoft::WRL::ComPtr <ID3D11Buffer> pUniformBuffer;//������ɫ������

};


#endif // !1
