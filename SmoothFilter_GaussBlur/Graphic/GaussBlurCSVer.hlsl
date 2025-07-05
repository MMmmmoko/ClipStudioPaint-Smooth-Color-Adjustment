


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




[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float2 uv=(float2(DTid.xy)+float2(0.5,0.5))*float2(_texWidth,_texHeight);


    float4 resultColor=float4(0,0,0,0);
    
    int loopLength=2*blurRadius+1;
    for(int i=0;i<loopLength;i+=2)
    {
    float offset=gaussWeights[i]-blurRadius;
    float2 curUV;
    curUV.x=0;
    curUV.y=offset*_texHeight;
    curUV=uv+curUV;

    float4 curColor=inputTex.SampleLevel(sampler_line,curUV,0);
    curColor.xyz*=curColor.a;
    curColor*=gaussWeights[i+1];
    resultColor+=curColor;
      
    }

    if(resultColor.a>0)
    {
    resultColor.xyz/=resultColor.a;
    }
    else
    {
    resultColor=float4(0,0,0,0);
    }

    //处理选区混合
    float4 BlendColor;
    //float selectAlpha=selectAreaTex.SampleLevel(sampler_line,uv,0).z;
    float selectAlpha=selectAreaTex.SampleLevel(sampler_line,uv,0).z;
    float4 sourceColor=origTex[DTid.xy];
    BlendColor=(resultColor-sourceColor)*selectAlpha+sourceColor;

    outTex[DTid.xy]=BlendColor;
    //outTex[DTid.xy]=resultColor;
    //outTex[DTid.xy]=float4(uv.x,uv.y,0,1);
    //outTex[DTid.xy]=float4(1,0,0,1);

}