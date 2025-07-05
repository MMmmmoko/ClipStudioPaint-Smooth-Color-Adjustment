


cbuffer gaussBlurUniform : register(b0) {

    int blurRadius;
    float _texWidth;  // 1/ÎÆÀí¿í¸ß
    float _texHeight;  
     float _padding;

    float gaussWeights[256*2+1+1] ;
};

Texture2D<float4> origTex:register(t0);
SamplerState sampler_line:register(s0);
RWTexture2D<float4> midTex:register(u0);




[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float2 uv=float2(DTid.x*_texWidth,DTid.y*_texHeight);


    float4 resultColor=float4(0,0,0,0);
    
    int loopLength=2*blurRadius+1;
    for(int i=0;i<loopLength;i+=2)
    {
    float offset=gaussWeights[i]-blurRadius;
    float2 curUV;
    curUV.x=offset*_texWidth;
    curUV.y=0;
    curUV=(float2(DTid.xy)+float2(0.5,0.5))*float2(_texWidth,_texHeight)+curUV;

    float4 curColor=origTex.SampleLevel(sampler_line,curUV,0);
    curColor.rgb*=curColor.a;
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


    midTex[DTid.xy]=resultColor;

    
    //´¹Ö±




}





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