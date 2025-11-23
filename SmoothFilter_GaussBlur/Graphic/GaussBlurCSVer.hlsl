


cbuffer gaussBlurUniform : register(b0) {

    int blurRadius;
        float _texWidth;  // 1/纹理宽高
    float _texHeight;  
        float _padding;


    float gaussWeights[256*2+1+1] ;




};

Texture2D<float4> inputTex:register(t0);
Texture2D<float4> selectAreaTex:register(t1);
Texture2D<float4> origTex:register(t2);
SamplerState sampler_line:register(s0);
RWTexture2D<float4> outTex:register(u0);




#define ThreadCount 1024

groupshared float4 cache[1][ThreadCount];


[numthreads(1, ThreadCount, 1)]
void main(uint3 DTid : SV_DispatchThreadID,
uint3 GTid : SV_GroupThreadID,
          uint3 Gid : SV_GroupID)
{
    
    //这里使用mipmap直接获取缩小的图像再放大产生的模糊方块感很重
    #if 0
    if (GTid.y < blurRadius || GTid.y >= ThreadCount - blurRadius)
        return;
    
    
    int blockHeight = ThreadCount - blurRadius * 2;
    int imageReadStartPos = Gid.y * blockHeight - blurRadius;
    int2 readPos = int2(DTid.x, imageReadStartPos + GTid.y);
    float w;
    float h;
    uint levels;
    origTex.GetDimensions(0, w, h, levels);
    
    
    //float slevel = float(blurRadius / 256) *(levels - 1);
    float slevel = (float(blurRadius) / 256.f) * (levels - 1);
    
    float4 sourceColor = origTex.SampleLevel(sampler_line, float2(_texWidth, _texHeight) * (readPos + float2(0.5, 0.5)), slevel);
    
    outTex[readPos] = sourceColor;
    
    
    
    return;
    
    
    #else
    
    int blockHeight = ThreadCount - blurRadius * 2;
    int imageReadStartPos = Gid.y * blockHeight - blurRadius;
    
    int2 readPos = int2(DTid.x,imageReadStartPos + GTid.y);
    
    cache[GTid.x][GTid.y] = inputTex.SampleLevel(sampler_line, float2(_texWidth, _texHeight) * (readPos + float2(0.5, 0.5)), 0);
    //cache[GTid.x][GTid.y] = inputTex.Load(int3(readPos, 0));
    

    GroupMemoryBarrierWithGroupSync();
    
       //边界像素判断
    if (GTid.y < blurRadius || GTid.y >= ThreadCount - blurRadius)
        return;
    if (readPos.y < 0 || readPos.y * _texHeight >= 1.001)
        return;
    
    
    

    float4 resultColor = float4(0, 0, 0, 0);
    
    int loopLength = 2 * blurRadius + 1;
    for (int i = 0; i < loopLength; i++)
    {
         //int2 curReadPos = int2(GTid.x - blurRadius + i, GTid.y);
        float4 curColor = cache[GTid.x][GTid.y - blurRadius + i];
        curColor.rgb *= curColor.a;
        curColor *= gaussWeights[i];
        resultColor += curColor;
    }

    if (resultColor.a > 0)
    {
        resultColor.xyz /= resultColor.a;
    }
    else
    {
        resultColor = float4(0, 0, 0, 0);
    }

    
    //处理选区混合
    float4 BlendColor;
    
    //float selectAlpha = selectAreaTex.Load(int3(readPos,0)).w;
    
    float selectAlpha = selectAreaTex.SampleLevel(sampler_line, float2(_texWidth, _texHeight) * (readPos + float2(0.5, 0.5)), 0).z;
    //selectAlpha = 1;
    float4 sourceColor = origTex[readPos];
    BlendColor = (resultColor - sourceColor) * selectAlpha + sourceColor;
    outTex[readPos] = BlendColor;
    
    
    //outTex[readPos] = inputTex[readPos];
#endif
}




























//[numthreads(32, 32, 1)]
//void main( uint3 DTid : SV_DispatchThreadID )
//{
//    float2 uv=(float2(DTid.xy)+float2(0.5,0.5))*float2(_texWidth,_texHeight);


//    float4 resultColor=float4(0,0,0,0);
    
//    int loopLength=2*blurRadius+1;
//    for(int i=0;i<loopLength;i+=2) 
//    {
//    float offset=gaussWeights[i]-blurRadius;
//    float2 curUV;
//    curUV.x=0;
//    curUV.y=offset*_texHeight;
//    curUV=uv+curUV;

//    float4 curColor=inputTex.SampleLevel(sampler_line,curUV,0);
//    curColor.xyz*=curColor.a;
//    curColor*=gaussWeights[i+1];
//    resultColor+=curColor;
      
//    }

//    if(resultColor.a>0)
//    {
//    resultColor.xyz/=resultColor.a;
//    }
//    else
//    {
//    resultColor=float4(0,0,0,0);
//    }

//    //处理选区混合
//    float4 BlendColor;
//    //float selectAlpha=selectAreaTex.SampleLevel(sampler_line,uv,0).z;
//    float selectAlpha=selectAreaTex.SampleLevel(sampler_line,uv,0).z;
//    float4 sourceColor=origTex[DTid.xy];
//    BlendColor=(resultColor-sourceColor)*selectAlpha+sourceColor;

//    outTex[DTid.xy]=BlendColor;
//    //outTex[DTid.xy]=resultColor;
//    //outTex[DTid.xy]=float4(uv.x,uv.y,0,1);
//    //outTex[DTid.xy]=float4(1,0,0,1);

//}