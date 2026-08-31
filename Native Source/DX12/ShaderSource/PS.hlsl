struct Vertex { float4 position : SV_POSITION; float2 uv : TEXCOORD0; };
SamplerState g_Sampler : register(s0);
Texture2D g_Texture : register(t0);
float4 frag(Vertex aVertex) : SV_Target0 { return g_Texture.Sample(g_Sampler, aVertex.uv); }