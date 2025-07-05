#pragma once


#include<d3d11.h>
#include<wrl/client.h>
#include <cstdint>

class Texture
{
public:
	enum _format
	{
		R8G8B8A8,
		B8G8R8A8,
		Alpha8
	};



	Texture(_format format = B8G8R8A8, unsigned int sizeW = 0, unsigned int sizeH = 0,void* pixData=NULL);
	virtual ~Texture() {
		pThis.Reset(); 
		pSRW.Reset();
	};

	Texture(const Texture&)=delete;
	Texture& operator=(const Texture&)=delete;


	virtual void Resize(unsigned int newWidth,unsigned int newHeight,void* pixdata,unsigned int rowBytes=0);
	//void UpdateData(unsigned int posX, unsigned int posY, unsigned int width, unsigned int height, void* data, unsigned int rowStride = 0);

	unsigned int GetSizeW() { return sizeW; };
	unsigned int GetSizeH() { return sizeH; };


	ID3D11ShaderResourceView* GetTex() { return pSRW.Get(); };


	void Bind();
	void UnBind() {};

protected:

	Microsoft::WRL::ComPtr<ID3D11Texture2D> pThis;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pSRW;
	_format format = _format::B8G8R8A8;
	unsigned int sizeW = 0;
	unsigned int sizeH = 0;
};

class RenderTexture :public Texture
{
public:
	RenderTexture() = default;
	virtual ~RenderTexture() {
		pRt.Reset();
		pCopyBuff.Reset();
	};//使用了智能指针，不需要释放

	void Resize(unsigned int newWidth,unsigned int newHeight);

	void Bind();
	void UnBind();
	ID3D11UnorderedAccessView* GetRT() { return pRt.Get(); };


	void CopyToMem(uint8_t* membuf,bool long_term, unsigned int rowBytes=0);



protected:
	
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> pRt;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> pCopyBuff;//用于拷贝的缓存

};

class AlphaTexture :public Texture
{
public:
	AlphaTexture() :Texture(_format::Alpha8) {};
	~AlphaTexture() = default;

};