
Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

cbuffer vscbSprite : register(b0)
{
	row_major float2x4 g_transform;
}

struct VS_INPUT
{
	float2 position : POSITION;
	float2 tex : TEXCOORD0;
	float4 color : COLOR0;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float4 color : COLOR0;
};

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output;

	output.position.xy = g_transform._13_14 + input.position.x * g_transform._11_12 + input.position.y * g_transform._21_22;
	output.position.z = g_transform._23;
	output.position.w = g_transform._24;

	output.tex = input.tex;

	output.color = input.color;

	return output;
}

float toonLight(float brightness)
{
	return 0.1 + brightness - fmod(brightness, 0.3);
}

float4 PS(VS_OUTPUT input) : SV_Target
{
	float4 color = texture0.Sample(sampler0, input.tex) * input.color;

	float3 light = normalize(float3(1, -1, 0));

	float x = input.tex.x;
	float y = input.tex.y;

	if (color.g < 0.1 && color.r < 0.1)
	{
		return float4(0, 0, 0, 0);
	}

	float height;
	float3 tangent;
	float3 binormal;
	//—‘‰©
	if (0.1 < color.g)
	{
		height = pow(saturate(color.g), 0.003);
		tangent = normalize(ddx(float3(x, height, y)));
		binormal = normalize(ddy(float3(x, height, y)));
	}
	//—‘”’
	else
	{
		height = pow(saturate(color.r), 0.01);
		tangent = normalize(ddx(float3(x, height, y)));
		binormal = normalize(ddy(float3(x, height, y)));
	}
	
	const float3 normal = normalize(cross(tangent, binormal));
	const float diffuse = dot(normal, light);

	//—‘‰©
	if (0.1 < color.g)
	{
		return float4(float3(1, 0.8, 0)*toonLight(diffuse), 1);
	}
	//—‘”’
	else
	{
		return float4(1, 1, 1, toonLight(diffuse));
	}
}

