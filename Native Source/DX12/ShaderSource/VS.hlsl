struct Vertex { float4 position : POSITION; float2 uv : TEXCOORD0; };
struct Pixel { float4 position : SV_POSITION; float2 uv : TEXCOORD0; };
Pixel vert(Vertex aVertex) { return aVertex; }