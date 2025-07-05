struct VS_INPUT 
{ 
float2 pos : POSITION; 
float2 tc : TEXCOORD;
};
struct VS_OUTPUT 
{ 
float4 pos : SV_POSITION; 
float2 tc : TEXCOORD; 
}; 

VS_OUTPUT main(VS_INPUT input) 
{
VS_OUTPUT output;
output.pos = float4(input.pos,0,1);
 output.tc = input.tc;
  return output;
  }