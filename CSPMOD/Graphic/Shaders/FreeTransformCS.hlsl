cbuffer Transform : register(b0)
{
    float4 sourceX;
    float4 sourceY;
    float4 denominator;
    int2 sourceOrigin;
    uint2 sourceSize;
    uint2 outputSize;
    uint bilinear;
    uint padding;
};

Texture2D<uint2> sourceImage : register(t0);
RWTexture2D<uint2> outputImage : register(u0);

uint2 ReadPixel(int2 position)
{
    position -= sourceOrigin;
    if (any(position < 0) || any(uint2(position) >= sourceSize))
        return uint2(0xffffffff, 0);
    return sourceImage.Load(int3(position, 0));
}

uint4 UnpackColor(uint packed)
{
    return (packed >> uint4(0, 8, 16, 24)) & 255;
}

uint PackColor(uint4 color)
{
    return color.x | (color.y << 8) | (color.z << 16) | (color.w << 24);
}

[numthreads(16, 16, 1)]
void main(uint3 index : SV_DispatchThreadID)
{
    if (any(index.xy >= outputSize))
        return;

    const float3 destination = float3(index.xy, 1.0);
    const float w = dot(denominator.xyz, destination);
    if (abs(w) < 0.0005)
    {
        outputImage[index.xy] = uint2(0xffffffff, 0);
        return;
    }
    const int2 fixedPoint = int2(float2(dot(sourceX.xyz, destination), dot(sourceY.xyz, destination)) * (256.0 / w));
    const int2 pixel = fixedPoint >> 8;
    if (!bilinear)
    {
        outputImage[index.xy] = ReadPixel(pixel);
        return;
    }

    const uint2 fraction = uint2(fixedPoint) & 255;
    const uint2 complement = 256 - fraction;
    const uint2 p00 = ReadPixel(pixel);
    const uint2 p10 = ReadPixel(pixel + int2(1, 0));
    const uint2 p01 = ReadPixel(pixel + int2(0, 1));
    const uint2 p11 = ReadPixel(pixel + int2(1, 1));
    const uint4 weight = uint4(complement.x * complement.y, fraction.x * complement.y,
        complement.x * fraction.y, fraction.x * fraction.y) * uint4(p00.y, p10.y, p01.y, p11.y);
    const uint alpha = weight.x + weight.y + weight.z + weight.w;
    const uint4 weightedColor = UnpackColor(p00.x) * weight.x + UnpackColor(p10.x) * weight.y +
        UnpackColor(p01.x) * weight.z + UnpackColor(p11.x) * weight.w;
    outputImage[index.xy] = alpha ? uint2(PackColor(weightedColor / alpha), alpha >> 16) : uint2(0xffffffff, 0);
}
