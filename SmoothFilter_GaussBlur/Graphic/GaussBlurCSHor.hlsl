//如果使用mip方案
//假设有一块512*512纹理 kernalRadius=3
//计算线性插值不应该按实际硬件插值[][][][][]->[  ][  ][ ,而是 ][  ][ 
//LeveL 0 [0][0][0][1][0][0][0] //512*512
//LeveL 1 [0][0][1/4][1/2][1/4][0][0] //采样时不再只有自己的信息
//LeveL 2 [1 /16][1/8][3/16][1/4][3/16][1/8][1/16] 


cbuffer gaussBlurUniform : register(b0) {

    int blurRadius;
    float _texWidth;  // 1/纹理宽高
    float _texHeight;  
     float _padding;
    
    
    
    float gaussWeights[256*2+1+1] ;
};

Texture2D<float4> origTex:register(t0);
SamplerState sampler_line:register(s0);
RWTexture2D<float4> midTex:register(u0);




#define ThreadCount 1024



//继续优化OPTMIZED：可以不只使用ThreadCount的cache
//cache最大为32,768，也就是实际上最优的情况下，
//在一个着色器过程中处理多个像素点，无论如何都可以实现一次性处理整行而不进行切分

//一次计算一排的ThreadCount个像素，
// 共享内存：缓存一行数据
groupshared float4 cache[ThreadCount][1];

//考虑预缓存部分值的图像或者用户在调节的时候实时缓存
//基于共享内存的方案实现
[numthreads(ThreadCount, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID,
uint3 GTid : SV_GroupThreadID,
          uint3 Gid : SV_GroupID)
{
    //return;
    

    
    //计算自己的实际坐标。
    //blurRadius为边界外面需要读取的像素数
    
    int blockWidth = ThreadCount - blurRadius * 2;
    int imageReadStartPos = Gid.x * blockWidth - blurRadius;
    //int posInGroup = DTid.x - Gid.x * ThreadCount;//GTID
    
    
    //int2 readPos = int2(imageReadStartPos + posInGroup, DTid.y);
    int2 readPos = int2(imageReadStartPos + GTid.x, DTid.y);
    
    cache[GTid.x][GTid.y] = origTex.SampleLevel(sampler_line, float2(_texWidth, _texHeight) * (readPos + float2(0.5,0.5)), 0);
    //cache[GTid.x][GTid.y] = origTex.Load(int3(readPos, 0));
    //cache[GTid.x][GTid.y].rgb *= cache[GTid.x][GTid.y].a;
    

    GroupMemoryBarrierWithGroupSync();
            //边界像素判断
    if (GTid.x < blurRadius || GTid.x >= ThreadCount - blurRadius)
        return;
    if (readPos.x < 0 || readPos.x * _texWidth > 1.001)
        return;

    
    
    
    
    float4 resultColor = float4(0, 0, 0, 0);
    int loopLength = 2 * blurRadius + 1;
    for (int i = 0; i < loopLength; i++)
    {
        //int2 curReadPos = int2(GTid.x - blurRadius + i, GTid.y);
        float4 curColor = cache[GTid.x - blurRadius + i][GTid.y];
        curColor.rgb *= curColor.a;
        curColor *= gaussWeights[i];
        resultColor += curColor;
    }
     
    if (resultColor.a > 0)
    {
        resultColor.xyz /= resultColor.a;
    }
    
    midTex[readPos] = resultColor;

    
    //垂直
}














//上版本实现
//[numthreads(32, 32, 1)]
//void main( uint3 DTid : SV_DispatchThreadID )
//{
//    float2 uv=float2(DTid.x*_texWidth,DTid.y*_texHeight);


//    float4 resultColor=float4(0,0,0,0);
    
//    int loopLength=2*blurRadius+1;
//    for(int i=0;i<loopLength;i+=2)
//    {
//    float offset=gaussWeights[i]-blurRadius; 
//    float2 curUV;
//    curUV.x=offset*_texWidth;
//    curUV.y=0;
//    curUV=(float2(DTid.xy)+float2(0.5,0.5))*float2(_texWidth,_texHeight)+curUV;

//    float4 curColor=origTex.SampleLevel(sampler_line,curUV,0);
//    curColor.rgb*=curColor.a;
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


//    midTex[DTid.xy]=resultColor;

    
//    //垂直

//}






















//[numthreads(32, 32, 1)]
//void main( uint3 DTid : SV_DispatchThreadID )
//{
//    float2 uv=float2(DTid.x*_texWidth,DTid.y*_texHeight);


//    float4 resultColor=float4(0,0,0,0);
    
//    int loopLength=2*blurRadius+1;
//    for(int i=0;i<loopLength;i+=2)
//    {
//    float offset=gaussWeights[i]-blurRadius;
//    float2 curUV;
//    curUV.x=offset*_texWidth;
//    curUV.y=0;
//    curUV=(float2(DTid.xy)+float2(0.5,0.5))*float2(_texWidth,_texHeight)+curUV;

//    float4 curColor=origTex.SampleLevel(sampler_line,curUV,0);

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
//    outTex[DTid.xy]=resultColor;
//}