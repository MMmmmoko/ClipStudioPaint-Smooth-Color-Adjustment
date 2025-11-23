#include "../pch.h"
#include "D3D11Graphic.h"


Microsoft::WRL::ComPtr<ID3D11Device> D3D11Graphic::pdevice = NULL;
Microsoft::WRL::ComPtr<ID3D11DeviceContext> D3D11Graphic::pcontext = NULL;

bool D3D11Graphic::Init()
{
	//创建设备和context
	HRESULT hr = S_OK;
	if (SUCCEEDED(hr))
	{
		UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;	// Direct2D需要支持BGRA格式
#if defined(DEBUG) || defined(_DEBUG)  
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		//C2DLOG_INFO("Execute D3D11CreateDevice...");
		hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, NULL, 0, D3D11_SDK_VERSION, &pdevice, NULL, &pcontext);
		if (FAILED(hr))
		{
			
			//C2DLOG_ERROR("D3D11CreateDevice failed in Cat2D::Init.");
		}
	} 



	if (SUCCEEDED(hr))
	{
		pdevice->GetImmediateContext(&pcontext);


		if (FAILED(hr))
		{
			//C2DLOG_ERROR("Device.As(DXGIDevice) failed in Cat2D::Init.");
		}

	}

	if (SUCCEEDED(hr))
	{
		return true;
	}
	return false;
}
