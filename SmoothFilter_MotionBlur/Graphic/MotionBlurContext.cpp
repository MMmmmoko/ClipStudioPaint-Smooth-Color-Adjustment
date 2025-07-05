#include "../pch.h"
#include"D3D11Graphic.h"
#include"../MotionBlur.h"
#include "MotionBlurContext.h"
#include"../Shader//MotionBlurCS.h"
#include <corecrt_math.h>


#define MAX_MOTIONBLUR_STRENGTH 512

bool MotionBlurContext::CreateContext()
{
    auto pdevice=D3D11Graphic::GetDevice();
    auto pcontext=D3D11Graphic::GetContext();
    


    //计算着色器
    if (FAILED(pdevice->CreateComputeShader(MotionBlurCS, sizeof(MotionBlurCS), NULL, &motionCS)))
    {
        return false;
    }

    //计算着色器常数
    {
        D3D11_BUFFER_DESC cbDesc;
        ZeroMemory(&cbDesc, sizeof(cbDesc));
        //对齐16字节
        auto padding = 16 - sizeof(MotionBlurUniform) % 16;
        if (padding == 16)padding = 0;
        cbDesc.ByteWidth = sizeof(MotionBlurUniform)+ padding;
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
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
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

void MotionBlurContext::BindAndDraw(RenderTexture* renderTarget,Texture* orgiTexture,Texture* selectAreaTexture,
    float targetX, float targetY,kBlurDirection direction,kBlurAlgorithm algorithm)
{

    //计算高斯核更新参数
    if (needInit)
    {
        needInit = false;
        CreateContext();
    }

    //方向

    if (direction == kBlurDirectionB)
    {
        targetX *= -1;
        targetY *= -1;
    }


    float strength = sqrtf(targetX* targetX+ targetY* targetY);

    if (0 != strength)
    {
        targetX /= strength;
        targetY /= strength;
    }

    //防止越界
    if (strength > MAX_MOTIONBLUR_STRENGTH)
        strength = MAX_MOTIONBLUR_STRENGTH;




    auto pdevice = D3D11Graphic::GetDevice();
    auto pcontext = D3D11Graphic::GetContext();


    //设置参数
    MotionBlurUniform MotionBlurUniform;
    //int __blurRadius;
    MotionBlurUniform.blurDirectionX_normalized= targetX/= orgiTexture->GetSizeW();
    MotionBlurUniform.blurDirectionY_normalized= targetY /= orgiTexture->GetSizeH();
    MotionBlurUniform.radius = ceil(0.5f*strength);
    MotionBlurUniform.bDirectionFB = (direction==kBlurDirectionFB)?1:0;
    CalcMotionBlurKerner(strength,MotionBlurUniform.motionWeights,direction,algorithm);
    MotionBlurUniform._width =1.f/orgiTexture->GetSizeW();
    MotionBlurUniform._height =1.f/orgiTexture->GetSizeH();


    //更新constantBuffer
    D3D11_MAPPED_SUBRESOURCE msr;
    pcontext->Map(pUniformBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &msr);
    memcpy(msr.pData, &MotionBlurUniform, sizeof(MotionBlurUniform));
    pcontext->Unmap(pUniformBuffer.Get(), 0u);





    //绑定并绘制水平方向
    {
        pcontext->CSSetShader(motionCS.Get(),NULL,0);
        ID3D11ShaderResourceView* pTex[2] = { orgiTexture->GetTex(),selectAreaTexture->GetTex() };
        pcontext->CSSetShaderResources(0,2, pTex);
        ID3D11UnorderedAccessView* pRT=renderTarget->GetRT();
        pcontext->CSSetUnorderedAccessViews(0,1, &pRT,NULL);
        
        pcontext->CSSetConstantBuffers(0,1, pUniformBuffer.GetAddressOf());
    
        pcontext->Dispatch((ceil(orgiTexture->GetSizeW()/16.f)), (ceil(orgiTexture->GetSizeH() / 16.f)),1);
    }
    





}

void MotionBlurContext::CalcMotionBlurKerner(float strength, BUFFER_FOLOAT* weightsMemory, kBlurDirection direction,kBlurAlgorithm algorithm)
{
    if (0 == strength)
    {
        *weightsMemory = 1;
        return;
    }

    float radius = 0.5 * strength;
    int radius_i =ceil(radius);
    int length = 2 * radius_i + 1;

    if (algorithm == kBlurAlgorithmAverage)
    {
        if (direction == kBlurDirectionFB)
        {
            //均值 前后
            float temStrength = strength;
            weightsMemory[radius_i] = 1;
            for (int i = 0; i <= radius_i; i++)
            {
                //中心点
                if (temStrength > 2)
                {
                    weightsMemory[radius_i - i] = 1;
                    weightsMemory[radius_i + i] = 1;
                    temStrength -= 2;
                }
                else
                {
                    weightsMemory[radius_i - i] = temStrength * 0.5f;
                    weightsMemory[radius_i +i] = temStrength * 0.5f;
                    temStrength = 0;
                }

            }


            float sum = 0;
            for (int i = 0; i < 2 * radius_i+1; i++)
            {
                sum += weightsMemory[i];
            }
            for (int i = 0; i < 2*radius_i+1; i++)
            {
                weightsMemory[i]= weightsMemory[i]/sum;
            }
        }
        else
        {
            float temStrength = strength;
            weightsMemory[0] = 1;
            for (int i = 1; i < length; i++)
            {
                if (temStrength > 1)
                {
                    weightsMemory[i] = 1;
                    temStrength -= 1;
                }
                else 
                {
                    weightsMemory[i] = temStrength;
                    temStrength = 0;
                }

            }
            float sum = 0;
            for (int i = 0; i < length; i++)
            {
                sum += weightsMemory[i];
            }
            for (int i = 0; i < length; i++)
            {
                weightsMemory[i] = weightsMemory[i] / sum;
            }
        }
    }
    else
    {
        if (direction == kBlurDirectionFB)
        {
            //平滑的话用高斯模糊
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
                weightsMemory[i] = weightsMemory[i]/weightSum;
            }
        }
        else
        {
            float sigma = strength / 3.f;
            
            int kernerRadius = ceil(radius);
            int kernerSize = kernerRadius * 2 + 1;

            float weightSum = 0;
            for (int i = 0; i < kernerSize; i++)
            {
                int x = i;
                weightsMemory[i] = expf(-(x * x) / (2 * sigma * sigma));
                weightSum += weightsMemory[i];
            }

            for (int i = 0; i < kernerSize; i++)
            {
                weightsMemory[i] = weightsMemory[i]/weightSum;
            }
        }
    }


}
