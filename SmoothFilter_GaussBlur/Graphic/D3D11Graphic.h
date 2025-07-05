#ifndef D3D11Graphic_h
#define D3D11Graphic_h


#include<d3d11.h>
#include<wrl/client.h>



class D3D11Graphic
{
public:
	static bool Init();
	static void ShutDown() {
		pdevice.Reset(); pcontext.Reset();
	};



	static ID3D11Device* GetDevice()
	{
		if (pdevice)
		{
			return pdevice.Get();
		}
		Init();
		return pdevice.Get();
	}
	
	static ID3D11DeviceContext* GetContext()
	{
		if (pcontext)
		{
			return pcontext.Get();
		}
		Init();
		return pcontext.Get();
	}

	static Microsoft::WRL::ComPtr<ID3D11Device> pdevice;
	static Microsoft::WRL::ComPtr<ID3D11DeviceContext> pcontext;
};














#endif