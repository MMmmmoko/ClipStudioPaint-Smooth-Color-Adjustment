#include "../pch.h"
#include"D3D11Graphic.h"
#include "GaussBlurContext.h"
#include"../Shader/VertexShader.h"
#include"../Shader//GaussBlurCSHor.h"
#include"../Shader/GaussBlurCSVer.h"
#include <corecrt_math.h>




bool GaussBlurContext::CreateContext()
{
    auto pdevice=D3D11Graphic::GetDevice();
    auto pcontext=D3D11Graphic::GetContext();
    


    //���㻺��
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


    //// �������벼��
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



    ////��ɫ��
    ////������ɫ��
    //Microsoft::WRL::ComPtr<ID3D11VertexShader> pVS;
    //if (FAILED( pdevice->CreateVertexShader(VertexShader, sizeof(VertexShader), NULL, &pVS)))
    //{
    //    return false;
    //}


    //������ɫ��
    if (FAILED(pdevice->CreateComputeShader(GaussBlurCSHor, sizeof(GaussBlurCSHor), NULL, &gaussCSHor)))
    {
        return false;
    }
    if (FAILED(pdevice->CreateComputeShader(GaussBlurCSVer, sizeof(GaussBlurCSVer), NULL, &gaussCSVer)))
    {
        return false;
    }


    //������ɫ������
    {
        D3D11_BUFFER_DESC cbDesc;
        ZeroMemory(&cbDesc, sizeof(cbDesc));
        //����16�ֽ�
        auto padding = 16 - sizeof(GaussBlurUniform) % 16;
        if (padding == 16)padding = 0;
        cbDesc.ByteWidth = sizeof(GaussBlurUniform)+ padding;
        cbDesc.Usage = D3D11_USAGE_DYNAMIC;      // ����CPU��̬����
        cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // CPU��д��
        if (FAILED(pdevice->CreateBuffer(&cbDesc, nullptr, &pUniformBuffer)))
        {
            return false;
        }
    }
    //������

    D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC{ CD3D11_DEFAULT{} };
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    Microsoft::WRL::ComPtr <ID3D11SamplerState> pSamplerStates;
    if (FAILED(pdevice->CreateSamplerState(&samplerDesc, &pSamplerStates)))
    {
        return false;
    }



    //����߰󶨹̶�����
    //����
    //pcontext->IASetVertexBuffers(0u, 1u, pVertexBuffer.GetAddressOf(), &bd.StructureByteStride, 0);
    //pcontext->IASetInputLayout(pInputLayout.Get());
    //pcontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    //pcontext->VSSetShader(pVS.Get(), nullptr, 0u);
    
    //��դ
    pcontext->CSSetSamplers(0,1, pSamplerStates.GetAddressOf());


	return true;
}

void GaussBlurContext::BindAndDraw(RenderTexture* renderTargetStep1, RenderTexture* renderTargetStep2, Texture* orgiTexture, Texture* selectAreaTexture, float blurRadius)
{

    //�����˹�˸��²���
    if (needInit)
    {
        needInit = false;
        CreateContext();
    }

    //��ֹ����Խ��
    if (blurRadius > MAX_GAUSSBLUR_RADIUS)blurRadius = MAX_GAUSSBLUR_RADIUS;


    auto pdevice = D3D11Graphic::GetDevice();
    auto pcontext = D3D11Graphic::GetContext();


    //���ò���
    GaussBlurUniform gaussBlurUniform;
    //int __blurRadius;
    CalcGaussBlurKerner(blurRadius,gaussBlurUniform.gaussWeights,&gaussBlurUniform.blurRadius);
    gaussBlurUniform._width =1.f/orgiTexture->GetSizeW();
    gaussBlurUniform._height =1.f/orgiTexture->GetSizeH();


    //����constantBuffer
    D3D11_MAPPED_SUBRESOURCE msr;
    pcontext->Map(pUniformBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &msr);
    memcpy(msr.pData, &gaussBlurUniform, sizeof(gaussBlurUniform));
    pcontext->Unmap(pUniformBuffer.Get(), 0u);





    //�󶨲�����ˮƽ����
    {
        pcontext->CSSetShader(gaussCSHor.Get(),NULL,0);
        ID3D11ShaderResourceView* pTex = orgiTexture->GetTex();
        pcontext->CSSetShaderResources(0,1, &pTex);
        ID3D11UnorderedAccessView* pRT=renderTargetStep1->GetRT();
        pcontext->CSSetUnorderedAccessViews(0,1, &pRT,NULL);
        
        pcontext->CSSetConstantBuffers(0,1, pUniformBuffer.GetAddressOf());
    
        pcontext->Dispatch((ceil(orgiTexture->GetSizeW()/16.f)), (ceil(orgiTexture->GetSizeH() / 16.f)),1);
    }
    


    //�󶨲����ƴ�ֱ����
    {
        pcontext->CSSetShader(gaussCSVer.Get(), NULL, 0);

        ID3D11UnorderedAccessView* pRT = renderTargetStep2->GetRT();
        pcontext->CSSetUnorderedAccessViews(0, 1, &pRT, NULL);

        ID3D11ShaderResourceView* pTex[3] = { renderTargetStep1->GetTex(),selectAreaTexture->GetTex(),orgiTexture->GetTex()};
        pcontext->CSSetShaderResources(0, 3, pTex);

        pcontext->CSSetConstantBuffers(0, 1, pUniformBuffer.GetAddressOf());
        pcontext->Dispatch((ceil(orgiTexture->GetSizeW() / 16.f)), (ceil(orgiTexture->GetSizeH() / 16.f)), 1);
    }






}

void GaussBlurContext::CalcGaussBlurKerner(float radius, BUFFER_FOLOAT* weightsMemory, int* radius_out)
{
    if (0 == radius)
    {
        weightsMemory[0] = 0;
        weightsMemory[1] = 1;
        *radius_out = 0;
        return;
    }

    //ȡ����Ϊ�뾶��1/3
    float sigma = radius / 3.f;

    int kernerRadius = ceil(radius);
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

    
    //������������� ͨ��������֮�����������һ���������
    for (int i = 0; i < radius; i++)
    {
        float w1 = weightsMemory[2 * i];
        float w2 = weightsMemory[2 * i+1];
        weightsMemory[2 * i] = 2 * i + w2 / (w1 + w2);
        weightsMemory[2 * i+1] = w1 + w2;
    }
    //ԭ����������������Ҫ���⴦�����һ����
    weightsMemory[kernerSize] = weightsMemory[kernerSize - 1];
    weightsMemory[kernerSize-1] = kernerSize - 1;
    *radius_out = kernerRadius;
}
