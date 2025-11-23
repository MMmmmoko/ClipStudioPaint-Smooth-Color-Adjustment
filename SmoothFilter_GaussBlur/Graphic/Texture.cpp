#include "../pch.h"
#include "Texture.h"
#include"D3D11Graphic.h"


Texture::Texture(Texture::_format format, unsigned int sizeW, unsigned int sizeH, void* pixData)
	:format(format), sizeW(sizeW), sizeH(sizeH)
{
	Resize(sizeW,sizeH, pixData);
}


//Texture::~Texture()
//{
//	//使用了智能指针，不需要释放
//}


void Texture::Resize(unsigned int newWidth,unsigned int newHeight, void* pixdata, unsigned int rowBytes)
{
	//if (newWidth == sizeW && newHeight == sizeH)
	//{
	//	return;
	//}

	if (newWidth == 0 || newHeight == 0)
	{ 
		return;
	}

	DXGI_FORMAT dataformat;
	if (format == _format::R8G8B8A8)
		dataformat = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	else if (format == _format::B8G8R8A8)
		dataformat = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
	else if (format == _format::Alpha8)
		dataformat = DXGI_FORMAT::DXGI_FORMAT_A8_UNORM;
	




	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = newWidth;
	textureDesc.Height = newHeight;
	//textureDesc.MipLevels = hasMipmap ? 0 : 1;
	textureDesc.MipLevels = 1;

	textureDesc.ArraySize = 1;
	textureDesc.Format = dataformat;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	//textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;



	D3D11_SUBRESOURCE_DATA subData;
	subData.pSysMem = pixdata;
	if(rowBytes>0)
		subData.SysMemPitch = rowBytes;
	else
		subData.SysMemPitch =(dataformat== Alpha8)? newWidth:(4* newWidth);
	subData.SysMemSlicePitch = 0;


	D3D11Graphic::GetDevice()->CreateTexture2D(
		&textureDesc, &subData, &pThis
	);




	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	//srvDesc.ViewDimension = b_ms ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;


	D3D11Graphic::GetDevice()->CreateShaderResourceView(
		pThis.Get(), &srvDesc, &pSRW
	);

	sizeW = newWidth;
	sizeH = newHeight;
}

//void Texture::UpdateData(unsigned int posX, unsigned int posY, unsigned int width, unsigned int height, void* data, unsigned int rowStride)
//{
//	return;
//}









void RenderTexture::Resize(unsigned int newWidth, unsigned int newHeight)
{
	if (newWidth == sizeW && newHeight == sizeH)
	{
		return;
	}

	DXGI_FORMAT dataformat;
	if (format == _format::R8G8B8A8)
		dataformat = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	else if (format == _format::B8G8R8A8)
		dataformat = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
	else if (format == _format::Alpha8)
		dataformat = DXGI_FORMAT::DXGI_FORMAT_A8_UNORM;





	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = newWidth;
	textureDesc.Height = newHeight;
	//textureDesc.MipLevels = hasMipmap ? 0 : 1;
	textureDesc.MipLevels = 1;

	textureDesc.ArraySize = 1;
	textureDesc.Format = dataformat;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage =D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_UNORDERED_ACCESS;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	//textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;




	D3D11Graphic::GetDevice()->CreateTexture2D(
		&textureDesc, NULL, &pThis
	);




	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	//srvDesc.ViewDimension = b_ms ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;


	D3D11Graphic::GetDevice()->CreateShaderResourceView(
		pThis.Get(), &srvDesc, &pSRW
	);


	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = textureDesc.Format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	D3D11Graphic::GetDevice()->CreateUnorderedAccessView(
		pThis.Get(), &uavDesc, &pRt
	);

	sizeW = newWidth;
	sizeH = newHeight;

}

void RenderTexture::CopyToMem(uint8_t* membuf, bool long_term, unsigned int rowBytes)
{
	//if(ms)
	//cat::Vector2i size = GetSize();


	bool needReset = false;
	if (pCopyBuff)
	{
		D3D11_TEXTURE2D_DESC copyDesc;
		pCopyBuff->GetDesc(&copyDesc);
		if (sizeW != copyDesc.Width || sizeH != copyDesc.Height)
			needReset = true;
	}
	else
	{
		needReset = true;
	}



	if (needReset)
	{


		//创建一个能由CPU访问的纹理
		D3D11_TEXTURE2D_DESC copyDesc;


		pThis->GetDesc(&copyDesc);
		copyDesc.Usage = D3D11_USAGE_STAGING;
		copyDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		copyDesc.SampleDesc.Count = 1;
		copyDesc.BindFlags = 0;
		copyDesc.MiscFlags = 0;

		D3D11Graphic::GetDevice()->CreateTexture2D(&copyDesc, nullptr, &pCopyBuff);
	}



	Microsoft::WRL::ComPtr<ID3D11Resource> copyBuffer;
	Microsoft::WRL::ComPtr<ID3D11Resource> thisBuffer;


	pCopyBuff->QueryInterface(__uuidof(ID3D11Resource), (void**)(&copyBuffer));

	pThis->QueryInterface(__uuidof(ID3D11Resource), (void**)(&thisBuffer));




	//if(ms)
	//GetContext(cat2d)->ResolveSubresource(copyBuffer.Get(), 0, cpThisBuffer.Get(), 0, DXGI_FORMAT_B8G8R8A8_UNORM);
	//else


	D3D11Graphic::GetContext()->CopyResource(copyBuffer.Get(), thisBuffer.Get());

	D3D11_MAPPED_SUBRESOURCE resultResources;

		D3D11Graphic::GetContext()->Map(copyBuffer.Get(), 0, D3D11_MAP_READ, 0, &resultResources);





		for (unsigned int i = 0; i < sizeH; i++)
		{
			auto pCurPos = membuf + i * rowBytes;
			auto pMapPos=(uint8_t*)resultResources.pData + resultResources.RowPitch * i;
			memcpy(pCurPos, pMapPos, sizeW*4);
		}



	//memcpy(membuf, resultResources.pData, sizeW * sizeH * 4);
	D3D11Graphic::GetContext()->Unmap(copyBuffer.Get(), 0);


	//如果不是长期复制  比如将资源转移到虚拟摄像头内存，那么就重置释放资源
	if (!long_term)
		pCopyBuff.Reset();

}