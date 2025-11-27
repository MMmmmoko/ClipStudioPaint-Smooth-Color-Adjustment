#include "../pch.h"
#include"D3D11Graphic.h"
#include"../MotionBlur.h"
#include "MotionBlurContext.h"
#include"../MotionBlurCS.h"
#include"../MotionBlurCS_Ver.h"
#include <corecrt_math.h>
#include <stdio.h>

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"dxgi.lib")

#define MAX_MOTIONBLUR_STRENGTH 511
//#define SHADER_THREAD_COUNT 1024
#define SHADER_THREAD_COUNT 682//受缓存大小限制需要减小线程数
bool MotionBlurContext::CreateContext()
{
    auto pdevice=D3D11Graphic::GetDevice();
    auto pcontext=D3D11Graphic::GetContext();
    


    //计算着色器 
    if (FAILED(pdevice->CreateComputeShader(MotionBlurCS, sizeof(MotionBlurCS), NULL, &motionCS)))
    {
        return false;
    }
    if (FAILED(pdevice->CreateComputeShader(MotionBlurCS_Ver, sizeof(MotionBlurCS_Ver), NULL, &motionCS_Ver)))
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
        cbDesc.ByteWidth = static_cast<UINT>(sizeof(MotionBlurUniform)+ padding);
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


    bool useYshader = false;
    bool isXDirection = fabsf(targetY) < fabsf(targetX);
    if (!isXDirection)
    {
        auto tem = targetX;
        targetX = targetY;
        targetY = tem;
        useYshader = true;
    }
    isXDirection = true;



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
    MotionBlurUniform MotionBlurUniform = {};
    //int __blurRadius;
    MotionBlurUniform.blurDirectionX_normalized= targetX;
    MotionBlurUniform.blurDirectionY_normalized= targetY;

    bool directionPositive = true;//单向时指向坐标轴正方向


    //MotionBlurUniform.radius =static_cast<int32_t> (ceil(0.5f*strength));
    //MotionBlurUniform.bDirectionFB = (direction==kBlurDirectionFB)?1:0;


    MotionBlurUniform._width =1.f/orgiTexture->GetSizeW();
    MotionBlurUniform._height =1.f/orgiTexture->GetSizeH();



    //bool isXDirection = fabsf(MotionBlurUniform.blurDirectionY_normalized) < fabsf(MotionBlurUniform.blurDirectionX_normalized);

    if (isXDirection)
    {
        if (/*direction == kBlurDirectionFB && */MotionBlurUniform.blurDirectionX_normalized < 0)
        {
            MotionBlurUniform.blurDirectionX_normalized = -MotionBlurUniform.blurDirectionX_normalized;
            MotionBlurUniform.blurDirectionY_normalized = -MotionBlurUniform.blurDirectionY_normalized;
            
            if(direction != kBlurDirectionFB)
            directionPositive = false;
        }


        CalcMotionBlurKerner(strength,
            MotionBlurUniform.blurDirectionX_normalized, MotionBlurUniform.blurDirectionY_normalized,
            MotionBlurUniform.motionWeights, direction, algorithm, &MotionBlurUniform.arrayLen, directionPositive);
    }
    else
    {

    }


    if(direction == kBlurDirectionFB)
    {

        MotionBlurUniform.blockStart = (MotionBlurUniform.arrayLen-1)/2;

        if (isXDirection)
        {
            //X方向
            if (MotionBlurUniform.blurDirectionY_normalized / MotionBlurUniform.blurDirectionX_normalized > 0)
            {
                MotionBlurUniform.xZeroPos = SHADER_THREAD_COUNT - 1 - (MotionBlurUniform.arrayLen - 1) / 2;
            }
            else
            {
                MotionBlurUniform.xZeroPos = (MotionBlurUniform.arrayLen - 1) / 2;
            }

         }
        else
        {
        }
    }
    else
    {

            //X方向
            if (MotionBlurUniform.blurDirectionY_normalized / MotionBlurUniform.blurDirectionX_normalized > 0)
            {
                if (directionPositive)
                {
                    MotionBlurUniform.blockStart = 0;
                    MotionBlurUniform.xZeroPos = SHADER_THREAD_COUNT-1 - (MotionBlurUniform.arrayLen - 1);
                }
                else
                {
                    MotionBlurUniform.blockStart = MotionBlurUniform.arrayLen - 1;
                    MotionBlurUniform.xZeroPos = SHADER_THREAD_COUNT - 1;
                }
            }
            else
            {
                if (directionPositive)
                {
                    MotionBlurUniform.blockStart =0;
                    MotionBlurUniform.xZeroPos = 0;
                }
                else
                {
                    MotionBlurUniform.blockStart = MotionBlurUniform.arrayLen - 1;
                    MotionBlurUniform.xZeroPos = MotionBlurUniform.arrayLen - 1;
                }
            }

    }








    //更新constantBuffer
    D3D11_MAPPED_SUBRESOURCE msr;
    pcontext->Map(pUniformBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &msr);
    memcpy(msr.pData, &MotionBlurUniform, sizeof(MotionBlurUniform));
    pcontext->Unmap(pUniformBuffer.Get(), 0u);




    //绑定并绘制水平方向
    {
        //为了调试，需要清楚画布
        uint32_t clearValue[4] = { 0,0,0,0 };
        pcontext->ClearUnorderedAccessViewUint(renderTarget->GetRT(),clearValue);



        //pcontext->CSSetShader(isXDirection?motionCS.Get(): motionCS_Ver.Get(), NULL, 0);
        pcontext->CSSetShader((!useYshader)?motionCS.Get(): motionCS_Ver.Get(), NULL, 0);
        ID3D11ShaderResourceView* pTex[2] = { orgiTexture->GetTex(),selectAreaTexture->GetTex() };
        pcontext->CSSetShaderResources(0,2, pTex);
        ID3D11UnorderedAccessView* pRT=renderTarget->GetRT();
        pcontext->CSSetUnorderedAccessViews(0,1, &pRT,NULL);
        pcontext->CSSetConstantBuffers(0,1, pUniformBuffer.GetAddressOf());
    



        //一个线程组能绘制的block大小
        if (!useYshader)
        {

            uint32_t blockSizeW = SHADER_THREAD_COUNT - (MotionBlurUniform.arrayLen-1);
            uint32_t blockCountX = (orgiTexture->GetSizeW() + blockSizeW - 1) / blockSizeW;
            uint32_t threadCountY = static_cast<UINT>(
                ceil(blockSizeW * fabsf(MotionBlurUniform.blurDirectionY_normalized/ MotionBlurUniform.blurDirectionX_normalized)));
            threadCountY = threadCountY + orgiTexture->GetSizeH();
            pcontext->Dispatch(blockCountX, threadCountY, 1);
        }
        else
        {
            uint32_t blockSizeH = SHADER_THREAD_COUNT - (MotionBlurUniform.arrayLen - 1);
            uint32_t blockCountY = (orgiTexture->GetSizeH() + blockSizeH - 1) / blockSizeH;
            //上方进行了XY调换，所以这里不用改顺序了
            uint32_t threadCountX = static_cast<UINT>(ceil(blockSizeH * fabsf(MotionBlurUniform.blurDirectionY_normalized / MotionBlurUniform.blurDirectionX_normalized)));
            threadCountX = threadCountX + orgiTexture->GetSizeW();
            pcontext->Dispatch(blockCountY,threadCountX, 1);
        }
    }
    





}




void MotionBlurContext::DoSamplerPoint(BUFFER_POINT* weightsMemory, float x, float y, float weight,int zeroPointIndex)
{
    int xPos = static_cast<int>(floor(x));
    int yPos = static_cast<int>(floor(y));
    float pixU = x - xPos;
    float pixV = y - yPos;

    float weightLT, weightRT, weightLB, weightRB;
    weightLT = (1 - pixU) * (1 - pixV);
    weightRT = (pixU) * (1 - pixV);
    weightLB = (1 - pixU) * (pixV);
    weightRB = (pixU) * (pixV);


    //寻找左上坐标
    int pLT_X=zeroPointIndex + xPos;
    if (pLT_X < 0)pLT_X = 0;
    int pLT_Y = yPos- weightsMemory[pLT_X].offset+1;
    if (pLT_Y < 0)pLT_Y = 0;
    else if (pLT_Y > 2)pLT_Y = 2;

    //右上
    int pRT_X = zeroPointIndex + xPos +1;
    if (pRT_X < 0)pRT_X = 0;
    int pRT_Y = yPos - weightsMemory[pRT_X].offset + 1 ;
    if (pRT_Y < 0)pRT_Y = 0;
    else if (pRT_Y > 2)pRT_Y = 2;

    //左下
    int pLB_X = pLT_X;
    int pLB_Y = yPos - weightsMemory[pLB_X].offset + 1+1;
    if (pLB_Y < 0)pLB_Y = 0;
    else if (pLB_Y > 2)pLB_Y = 2;

    //右下
    int pRB_X = pRT_X;
    int pRB_Y = yPos - weightsMemory[pRB_X].offset + 1 + 1;
    if (pRB_Y < 0)pRB_Y = 0;
    else if (pRB_Y > 2)pRB_Y = 2;

    //填表
    weightsMemory[pLT_X].value[pLT_Y] += weight * weightLT;
    weightsMemory[pRT_X].value[pRT_Y] +=  weight * weightRT;
    weightsMemory[pLB_X].value[pLB_Y] +=  weight * weightLB;
    weightsMemory[pRB_X].value[pRB_Y] +=  weight * weightRB;

}



void MotionBlurContext::CalcMotionBlurKerner(float strength, float directionX_normal,
    float directionY_normal, BUFFER_POINT* pointsMemory, kBlurDirection direction,
    kBlurAlgorithm algorithm, int* arrayLen,bool positiveDirection)
{



    if (0 == strength)
    {
        pointsMemory[0].offset = 0;
        pointsMemory[0].value[0] = 0;
        pointsMemory[0].value[1] = 1;
        pointsMemory[0].value[2] = 0;
        *arrayLen = 1;
        return;
    }




    float radius = 0.5F * strength;
    int radius_i = static_cast<int>(ceil(radius* directionX_normal));
    int radius_i_sampler = static_cast<int>(ceil(radius));
    int length = (direction == kBlurDirectionFB)?(2 * radius_i + 1): static_cast<int>(ceil(strength* directionX_normal)+1);

    *arrayLen = length;
    printf("length: %d\n", length);

    //memset(pointsMemory, 0,1+ static_cast<int>(ceil(strength * directionX_normal) *sizeof(BUFFER_POINT)));


    float lineK = directionY_normal / directionX_normal;//斜率
    //先填充offset//以线程组中第一个可绘制的点offset为0
    for (int i = 0; i < length; i++)
    {
        //相对于组内可绘制的首点的偏移
        int curXoff = (direction == kBlurDirectionFB)?(i- radius_i):i;
        pointsMemory[i].offset=static_cast<int>(round(curXoff* lineK));
    }

    //采样点[]


    if (algorithm == kBlurAlgorithmAverage)
    {
        if (direction == kBlurDirectionFB)
        {
            float temStrength = strength*0.5f;
         

            float weightInside = 1.f/(1.f + strength);
            float weightSide = (temStrength - (radius_i_sampler-1))/ (1.f + strength);



            float weightTable[2 * 256 + 1];
            float remainWeight = 1.f;
            weightTable[radius_i_sampler] = weightInside;
            remainWeight -= weightInside;
            for (int i = 0; i < radius_i_sampler ; i++)
            {
                if (remainWeight > 2 * weightInside)
                {
                    weightTable[radius_i_sampler - i - 1]= weightInside;
                    weightTable[radius_i_sampler + i + 1] = weightInside;
                    remainWeight -= 2 * weightInside;
                }
                else
                {
                    weightTable[radius_i_sampler - i - 1] = remainWeight*0.5f;
                    weightTable[radius_i_sampler + i + 1] = remainWeight * 0.5f;
                    remainWeight = 0;
                }
            }


            for (int i = 0; i < 2 * radius_i_sampler + 1; i++)
            {



                //相对于组内可绘制的首点的偏移
                int curIndex = i - radius_i_sampler;

                //计算采样点偏移位置
                float samplerPosX = directionX_normal * curIndex;
                float samplerPosY = directionY_normal * curIndex;

                float weight;
                if (i == 0 || i == 2 * radius_i_sampler)weight = weightSide;
                else weight = weightInside;
                DoSamplerPoint(pointsMemory, samplerPosX, samplerPosY, weightTable[i], radius_i);
            }


        }
        else
        {
            float weightInside = 1.f / (1.f + strength);
            int samplerLen =1+ static_cast<int>(ceil(strength));
            float weightSide = (strength-floorf(strength))/ (1.f + strength);



            float weightTable[2 * 256 + 1];
            float remainWeight = 1.f;
            weightTable[0] = weightInside;
            remainWeight -= weightInside;
            for (int i = 0; i < samplerLen; i++)
            {
                if (remainWeight >  weightInside)
                {
                    weightTable[i] = weightInside;
                    remainWeight -=  weightInside;
                }
                else
                {
                    weightTable[i] = remainWeight;
                    remainWeight = 0;
                }

            }

            for (int i = 0; i < samplerLen; i++)
            {






                //相对于组内可绘制的首点的偏移
                int curIndex = i;

                //计算采样点偏移位置
                float samplerPosX = directionX_normal * curIndex;
                float samplerPosY = directionY_normal * curIndex;

                if (!positiveDirection)
                {
                    samplerPosX = -samplerPosX;
                    samplerPosY = -samplerPosY;
                }

                float weight;
                if (positiveDirection)weight = weightTable[i];
                else
                    weight = weightTable[samplerLen - i - 1];




                DoSamplerPoint(pointsMemory, samplerPosX, samplerPosY, weight, positiveDirection?0:(length-1));
            }


        }
    }
    else
    {
        float weightTable[2 * 256 + 1];
        if (direction == kBlurDirectionFB)
        {
            //选择平滑时采用高斯模糊

            float sigma = radius / 3.f;

            int kernerRadius = radius_i_sampler;
            int kernerSize = kernerRadius * 2 + 1;


            float weightSum = 0;
            for (int i = 0; i < kernerSize; i++)
            {
                int x = i - kernerRadius;
                weightTable[i] = expf(-(x * x) / (2 * sigma * sigma));
                weightSum += weightTable[i];
            }

            for (int i = 0; i < kernerSize; i++)
            {
                weightTable[i] = weightTable[i] / weightSum;
            }


            //for (int i = 0; i < 2 * radius_i_sampler + 1; i++)
            for (int i = 0; i < 2 * radius_i_sampler + 1; i++)
            {
                //相对于组内可绘制的首点的偏移
                int curIndex = i - radius_i_sampler;

                //计算采样点偏移位置
                float samplerPosX = directionX_normal * curIndex;
                float samplerPosY = directionY_normal * curIndex;

                DoSamplerPoint(pointsMemory, samplerPosX, samplerPosY, weightTable[i],radius_i);
            }


        }
        else
        {
            float sigma = strength / 3.f;


            int kernerSize = 1+ static_cast<int>(ceil(strength));

            float weightSum = 0;
            for (int i = 0; i < kernerSize; i++)
            {
                int x =positiveDirection?i: (kernerSize-1-i);
                weightTable[i] = expf(-(x * x) / (2 * sigma * sigma));
                weightSum += weightTable[i];
            }

            for (int i = 0; i < kernerSize; i++)
            {
                weightTable[i] = weightTable[i] / weightSum;
            }


            for (int i = 0; i < kernerSize; i++)
            {



                //相对于组内可绘制的首点的偏移
                int curIndex = i;

                //计算采样点偏移位置
                float samplerPosX = directionX_normal * curIndex;
                float samplerPosY = directionY_normal * curIndex;
                if (!positiveDirection)
                {
                    samplerPosX = -samplerPosX;
                    samplerPosY = -samplerPosY;
                }



                DoSamplerPoint(pointsMemory, samplerPosX, samplerPosY, weightTable[positiveDirection?i:(kernerSize-1-i)], positiveDirection ? 0: (length - 1 ));
            }

        }




    }



}
