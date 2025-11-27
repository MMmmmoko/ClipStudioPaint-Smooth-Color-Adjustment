

struct POINTWEIGHTSINFO
{
    int offset;
    float3 value;
};

cbuffer MotionBlurUniform : register(b0) {

    float directionX;
    float directionY;
    //int arrayRadius; //数组半径 始终为正 实际半径的x分量
    int cacheReadIndexOff;
    int arrayLen; //数组长度

    float _texWidth;  // 1/纹理宽高
    float _texHeight;
    int blockStart; //线程组中一个可绘制的区块的起始索引，双向可能为arrayRadius，单向可能为0..
    int xZeroPos;//线程组中，x定位0点的索引，双向为半径处 单向为0或者2倍半径

    //int sampleOffsetArray[256*2+1+3] ;//+3：padding
    //float weights[256*2+1+3] ;//+3：padding
    //int2 pointIndex[256 * 2 + 1][4];
    
    
    //这个数据是给单个点使用的，不是整个线程组
    POINTWEIGHTSINFO PointsWeightsInfo[256 * 2 + 1]; //XYZ分行，Wpadding
};

Texture2D<float4> origTex:register(t0);
Texture2D<float4> selectAreaTex:register(t1);
SamplerState sampler_line:register(s0);
RWTexture2D<float4> rtTex:register(u0);


//#define ThreadCount 1024
#define ThreadCount 682//cache缓存大小限制

//一次计算一排的ThreadCount个像素，)(缓存3排)
groupshared float4 cache[ThreadCount][3];


[numthreads(ThreadCount, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID,
uint3 GTid : SV_GroupThreadID,
          uint3 Gid : SV_GroupID)
{
    
    //根据directtion计算采样偏移
    //以block左侧的偏移为0
    //int(GTid.x)神经病 查了半天bug发现这里必须手动从uint转int 不然负数时结果是无限大
    int offPixHeight = int(round((int(GTid.x) - xZeroPos) * (directionY / directionX)));
    //int offPixHeight = sampleOffsetArray[GTid.x];//由CPU计算，减少GPU压力并保证数据一致性
    //int blockWidth = ThreadCount - arrayRadius * 2;
    int blockWidth = ThreadCount - (arrayLen-1);
    int2 readPos;
    readPos.x = int(Gid.x * blockWidth + GTid.x) - blockStart;
    readPos.y = int(DTid.y) + offPixHeight;
    
    
    //rtTex[readPos] = float4(readPos * float2(_texWidth, _texHeight), 0, 1);
    //return;
    
    
    
    cache[GTid.x][0] = origTex.SampleLevel(sampler_line, 
    float2(_texWidth, _texHeight) * (readPos + float2(0.5, -0.5)), 0);
    cache[GTid.x][1] = origTex.SampleLevel(sampler_line, 
    float2(_texWidth, _texHeight) * (readPos + float2(0.5, 0.5)), 0);
    cache[GTid.x][2] = origTex.SampleLevel(sampler_line, 
    float2(_texWidth, _texHeight) * (readPos + float2(0.5, 1.5)), 0);
    cache[GTid.x][0].rgb *= cache[GTid.x][0].a;
    cache[GTid.x][1].rgb *= cache[GTid.x][1].a;
    cache[GTid.x][2].rgb *= cache[GTid.x][2].a;
    
    GroupMemoryBarrierWithGroupSync();
            //边界像素判断 //横向暂时不考虑对Y方向越界进行判断
    if (GTid.x < blockStart || GTid.x >= blockStart + blockWidth)
        return;
    if (readPos.x < 0 || readPos.x * _texWidth >= 1)
        return;

    
    float4 resultColor = float4(0, 0, 0, 0);
    for (int i = 0; i < arrayLen; i++)
    {
        int weightsReadIndex = i;
        int cacheReadIndex = GTid.x + i - blockStart;
        
        
        resultColor+= cache[cacheReadIndex][0] * PointsWeightsInfo[weightsReadIndex].value.x;
        resultColor+= cache[cacheReadIndex][1] * PointsWeightsInfo[weightsReadIndex].value.y;
        resultColor+= cache[cacheReadIndex][2] * PointsWeightsInfo[weightsReadIndex].value.z;
    }
    
    if (resultColor.a > 0)
        resultColor.rgb /= resultColor.a;
    //readPos=clamp(readPos, float2(0, 0), float2(1 / _texWidth, 1 / _texHeight));
    //混合
        float4 origColor = origTex[readPos];
    float selectAlpha = selectAreaTex.SampleLevel(sampler_line, float2(_texWidth, _texHeight) * (readPos + float2(0.5, 0.5)), 0).z;
    float4 BlendColor = (resultColor - origColor) * selectAlpha + origColor;

    
    
    
    rtTex[readPos] = BlendColor;
    
    
    
    //rtTex[readPos] = float4(1, 0, 0, 1);
    //rtTex[readPos] = origColor;
    //rtTex[readPos] = float4(float3(1, 1, 1) - origColor.xyz, origColor.a);
     
    //rtTex[int2(readPos.x, DTid.y)] = float4(readPos * float2(_texWidth, _texHeight), 0, 1);
    //float testValue = abs(offPixHeight * _texHeight);
    //float testValue = abs((int(GTid.x) - xZeroPos) / 10000.f);
    //rtTex[int2(readPos.x, DTid.y)] = float4(testValue, testValue, testValue, 1);
}




