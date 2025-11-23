//#include "../pch.h"
#include"D3D11Graphic.h"
#include "GaussBlurContext.h"
//#include"../Shader/VertexShader.h"
#include <corecrt_math.h>

#ifndef BYTE
#define BYTE uint8_t
#endif // !BYTE

#include"../GaussBlurCSHor.h"
#include"../GaussBlurCSVer.h"

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"dxgi.lib")



bool GaussBlurContext::CreateContext()
{
    auto pdevice=D3D11Graphic::GetDevice();
    auto pcontext=D3D11Graphic::GetContext();
    


    //顶点缓冲
    //const float vertexData[] = {
    //    -1.0f,-1.0f,0.f,1.f,
    //    1.0f,-1.0f,1.f,1.f,
    //    -1.0f,1.0f,0.f,0.f,
    //    1.0f,1.0f,1.f,0.f,
    //};

    //


    //D3D11_BUFFER_DESC bd = {};
    //bd.BindFlags = D3D11_BIND_VERTEX_BUFFER; 
    //bd.Usage = D3D11_USAGE_DEFAULT;
    //bd.CPUAccessFlags = 0u;
    //bd.MiscFlags = 0u;
    //bd.ByteWidth =sizeof(vertexData);
    //bd.StructureByteStride = sizeof(float)*4;
    //D3D11_SUBRESOURCE_DATA sd = {};
    //sd.pSysMem = vertexData;

    //Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;
    //if (FAILED(pdevice->CreateBuffer(&bd, &sd, &pVertexBuffer)))
    //    return false;


    //// 创建输入布局
    //D3D11_INPUT_ELEMENT_DESC layout[] = {
    //    { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    //    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    //};

    //Microsoft::WRL::ComPtr <ID3D11InputLayout> pInputLayout;
    //if (FAILED(pdevice->CreateInputLayout(layout, 2, VertexShader, sizeof(VertexShader), &pInputLayout))) {
    //    return false;
    //}
    ////
    ////D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP



    ////着色器
    ////顶点着色器
    //Microsoft::WRL::ComPtr<ID3D11VertexShader> pVS;
    //if (FAILED( pdevice->CreateVertexShader(VertexShader, sizeof(VertexShader), NULL, &pVS)))
    //{
    //    return false;
    //}


    //计算着色器
    if (FAILED(pdevice->CreateComputeShader(GaussBlurCSHor, sizeof(GaussBlurCSHor), NULL, &gaussCSHor)))
    {
        return false;
    }
    if (FAILED(pdevice->CreateComputeShader(GaussBlurCSVer, sizeof(GaussBlurCSVer), NULL, &gaussCSVer)))
    {
        return false;
    }


    //计算着色器常数
    {
        D3D11_BUFFER_DESC cbDesc;
        ZeroMemory(&cbDesc, sizeof(cbDesc));
        //对齐16字节
        auto padding = 16 - sizeof(GaussBlurUniform) % 16;
        if (padding == 16)padding = 0;
        cbDesc.ByteWidth = static_cast<uint32_t>(sizeof(GaussBlurUniform)+ padding);
        cbDesc.Usage = D3D11_USAGE_DYNAMIC;      // 允许CPU动态更新
        cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // CPU可写入
        if (FAILED(pdevice->CreateBuffer(&cbDesc, nullptr, &pUniformBuffer)))
        {
            return false;
        }
    }
    //采样器

    D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC{ CD3D11_DEFAULT{} };
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
    Microsoft::WRL::ComPtr <ID3D11SamplerState> pSamplerStates;
    if (FAILED(pdevice->CreateSamplerState(&samplerDesc, &pSamplerStates)))
    {
        return false;
    }



    //向管线绑定固定属性
    //顶点
    //pcontext->IASetVertexBuffers(0u, 1u, pVertexBuffer.GetAddressOf(), &bd.StructureByteStride, 0);
    //pcontext->IASetInputLayout(pInputLayout.Get());
    //pcontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    //pcontext->VSSetShader(pVS.Get(), nullptr, 0u);
    
    //光栅
    pcontext->CSSetSamplers(0,1, pSamplerStates.GetAddressOf());


	return true;
}

void GaussBlurContext::BindAndDraw(RenderTexture* renderTargetStep1, RenderTexture* renderTargetStep2, Texture* orgiTexture, Texture* selectAreaTexture, float blurRadius)
{

    //计算高斯核更新参数
    if (needInit)
    {
        needInit = false;
        CreateContext();
    }

    //防止参数越界
    if (blurRadius > MAX_GAUSSBLUR_RADIUS)blurRadius = MAX_GAUSSBLUR_RADIUS;


    auto pdevice = D3D11Graphic::GetDevice();
    auto pcontext = D3D11Graphic::GetContext();


    //设置参数
    GaussBlurUniform gaussBlurUniform;
    //int __blurRadius;
    CalcGaussBlurKerner(blurRadius,gaussBlurUniform.gaussWeights,&gaussBlurUniform.blurRadius);
    gaussBlurUniform._width =1.f/orgiTexture->GetSizeW();
    gaussBlurUniform._height =1.f/orgiTexture->GetSizeH();


    //更新constantBuffer
    D3D11_MAPPED_SUBRESOURCE msr;
    pcontext->Map(pUniformBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &msr);
    memcpy(msr.pData, &gaussBlurUniform, sizeof(gaussBlurUniform));
    pcontext->Unmap(pUniformBuffer.Get(), 0u);





    //绑定并绘制水平方向
    {
        pcontext->CSSetShader(gaussCSHor.Get(),NULL,0);
        ID3D11ShaderResourceView* pTex = orgiTexture->GetTex();
        pcontext->CSSetShaderResources(0,1, &pTex);
        ID3D11UnorderedAccessView* pRT=renderTargetStep1->GetRT();
        pcontext->CSSetUnorderedAccessViews(0,1, &pRT,NULL);
        
        pcontext->CSSetConstantBuffers(0,1, pUniformBuffer.GetAddressOf());
    
        //pcontext->Dispatch(static_cast<uint32_t>(ceil(orgiTexture->GetSizeW() / 16.f)), static_cast<uint32_t>((ceil(orgiTexture->GetSizeH() / 16.f))), 1);


        //int blockWidth= 512 - gaussBlurUniform.blurRadius * 2;
        //if (blockWidth == 0)blockWidth = 1;
        //uint32_t wThreadGroupNum= (orgiTexture->GetSizeW()+ blockWidth-1)/ blockWidth;
        //pcontext->Dispatch(wThreadGroupNum, (orgiTexture->GetSizeH()+1)/2,1);

        int blockWidth= 1024 - gaussBlurUniform.blurRadius * 2;
        if (blockWidth == 0)blockWidth = 1;
        uint32_t wThreadGroupNum= (orgiTexture->GetSizeW()+ blockWidth-1)/ blockWidth;
        pcontext->Dispatch(wThreadGroupNum, orgiTexture->GetSizeH(),1);
    }
    


    //绑定并绘制垂直方向
    {
        pcontext->CSSetShader(gaussCSVer.Get(), NULL, 0);

        ID3D11UnorderedAccessView* pRT = renderTargetStep2->GetRT();
        pcontext->CSSetUnorderedAccessViews(0, 1, &pRT, NULL);

        ID3D11ShaderResourceView* pTex[3] = { renderTargetStep1->GetTex(),selectAreaTexture->GetTex(),orgiTexture->GetTex()};
        pcontext->CSSetShaderResources(0, 3, pTex);

        pcontext->CSSetConstantBuffers(0, 1, pUniformBuffer.GetAddressOf());


        //pcontext->Dispatch(static_cast<uint32_t>(ceil(orgiTexture->GetSizeW() / 16.f)), static_cast<uint32_t>(ceil(orgiTexture->GetSizeH() / 16.f)), 1);



        //int blockHeight = 512 - gaussBlurUniform.blurRadius * 2;
        //if (blockHeight == 0)blockHeight = 1;
        //uint32_t hThreadGroupNum = (orgiTexture->GetSizeH() + blockHeight - 1) / blockHeight;
        //pcontext->Dispatch((orgiTexture->GetSizeW()+1) /2, hThreadGroupNum, 1);

        int blockHeight = 1024 - gaussBlurUniform.blurRadius * 2;
        if (blockHeight == 0)blockHeight = 1;
        uint32_t hThreadGroupNum = (orgiTexture->GetSizeH() + blockHeight - 1) / blockHeight;
        pcontext->Dispatch(orgiTexture->GetSizeW(), hThreadGroupNum, 1);
    }

}

void GaussBlurContext::CalcGaussBlurKerner(float radius, BUFFER_FOLOAT* weightsMemory, int* radius_out)
{
    if (0 == radius)
    {
        weightsMemory[0] = 1;
        *radius_out = 0;
        return;
    }

    //取方差为半径的1/3
    float sigma = radius / 3.f;

    int kernerRadius = static_cast<int>(ceil(radius));
    int kernerSize = kernerRadius * 2 + 1;



    float weightSum = 0;
    for (int i = 0; i < kernerSize; i++)
    {
        int x = i - kernerRadius;
        weightsMemory[i] = expf(-(x*x)/(2*sigma*sigma));
        weightSum += weightsMemory[i];
    }

    
    for (int i = 0; i < kernerSize; i++)
    {
        weightsMemory[i].value /= weightSum;
    }

    
    ////计算特殊采样点 通过在两点之间采样来降低一半采样次数
    //for (int i = 0; i < radius; i++)
    //{
    //    float w1 = weightsMemory[2 * i];
    //    float w2 = weightsMemory[2 * i+1];
    //    weightsMemory[2 * i] = 2 * i + w2 / (w1 + w2);
    //    weightsMemory[2 * i+1] = w1 + w2;
    //}
    ////原采样点是奇数个，要特殊处理最后一个点
    //weightsMemory[kernerSize] = weightsMemory[kernerSize - 1];
    //weightsMemory[kernerSize - 1] = static_cast<float >( kernerSize - 1);
    *radius_out = kernerRadius;
}
