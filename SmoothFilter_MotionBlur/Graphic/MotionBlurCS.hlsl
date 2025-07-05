


cbuffer MotionBlurUniform : register(b0) {

    float directionX;
    float directionY;
    int radius;
    int bDirectionFB;

    float _texWidth;  // 1/ÎÆÀí¿í¸ß
    float _texHeight; 
     float _padding1;
     float _padding2;

    float weights[256*2+1] ;
};

Texture2D<float4> origTex:register(t0);
Texture2D<float4> selectAreaTex:register(t1);
SamplerState sampler_line:register(s0);
RWTexture2D<float4> rtTex:register(u0);




[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float2 uv=(float2(DTid.xy)+float2(0.5,0.5))*float2(_texWidth,_texHeight);


    float4 origColor =origTex.SampleLevel(sampler_line,uv,0);

    if(radius==0)
    {
    rtTex[DTid.xy]=origColor;
        return;
    }
    
    float4 resultColor = float4(0,0,0,0);


    for(int i=0;i<(2*radius+1);i++)
    {
    int offset = i-radius*bDirectionFB;

    float2 curUV = float2(directionX,directionY);
    curUV=uv+curUV*offset;
    
    float4 curColor=origTex.SampleLevel(sampler_line,curUV,0);
    curColor.xyz*=curColor.a;

    curColor*=weights[i];
    resultColor+=curColor;
    }

    if(resultColor.a>0)
    {
    resultColor.xyz/=resultColor.a;
    }
    else
    {
    resultColor= float4(0,0,0,0);
    }


    //»ìºÏ
    float selectAlpha=selectAreaTex.SampleLevel(sampler_line,uv,0).z;
    float4 BlendColor=(resultColor-origColor)*selectAlpha+origColor;

    rtTex[DTid.xy]=resultColor;


}




