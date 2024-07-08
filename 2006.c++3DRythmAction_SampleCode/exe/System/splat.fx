// 전역변수
float4x4	mWVP;

//-------------------------------------------------------------------------------------
// ps_1_1용 
texture		TexAlpha;		// 알파맵
texture		Tex1;
texture		Tex2;
texture		Tex3;

sampler Samp0 = sampler_state
{
	Texture = <TexAlpha>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = NONE;
	
	AddressU = Clamp;
	AddressV = Clamp;
};

sampler Samp1 = sampler_state
{
	Texture = <Tex1>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = NONE;
	
	AddressU = Clamp;
	AddressV = Clamp;
};

sampler Samp2 = sampler_state
{
	Texture = <Tex2>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = NONE;
	
	AddressU = Clamp;
	AddressV = Clamp;
};

sampler Samp3 = sampler_state
{
	Texture = <Tex3>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = NONE;
	
	AddressU = Clamp;
	AddressV = Clamp;
};

struct VS_OUTPUT
{
	float4 Pos : POSITION;
	float2 TexAlpha : TEXCOORD0;
	float2 Tex1 : TEXCOORD1;
	float2 Tex2 : TEXCOORD2;
	float2 Tex3 : TEXCOORD3;
};

// 정점 셰이더
VS_OUTPUT VS ( VS_OUTPUT In )
{
	VS_OUTPUT Out = (VS_OUTPUT)0;
	
	Out.Pos = mul( In.Pos, mWVP );
	Out.TexAlpha = In.TexAlpha;
	Out.Tex1 = In.Tex1;
	Out.Tex2 = In.Tex2;
	Out.Tex3 = In.Tex3;
	
	return Out;
}

// 픽셀 셰이더
float4 PS ( VS_OUTPUT In ) : COLOR
{
	float4 a = tex2D(Samp0, In.TexAlpha);
	
	float4 f, c;
	float1 sum;
	
	f = tex2D(Samp1, In.Tex1);
	
	sum = a.a + a.r;
	c = tex2D(Samp2, In.Tex2);
	f = lerp(c, f, 1-(a.r/min(1,sum)));
	f.a = max(a.a, a.r);
	
	sum = f.a + a.g;
	c = tex2D(Samp3, In.Tex3);
	f = lerp(c, f, 1-(a.g/min(1,sum)));
	f.a = max(f.a, a.g);
	
	return f;
/*
	float4 f;
	float4 a = tex2D(Samp0, In.TexAlpha);
	float4 c1 = tex2D(Samp1, In.Tex1);
	float4 c2 = tex2D(Samp2, In.Tex2);
	float4 c3 = tex2D(Samp3, In.Tex3);

	f = lerp(lerp(c1 * a.a, c2, a.r), c3, a.g);
	f.a = max( max(a.a, a.r), a.g );
	return f;*/
}

//-------------------------------------------------------------------------------------
// 고정파이프라인용 텍스처
texture FixedTex0;			// 알파맵
texture FixedTex1;			// 컬러맵

sampler FixedSamp0 = sampler_state
{
	Texture = <FixedTex0>;
	MinFilter = POINT;
	MagFilter = POINT;
	MipFilter = NONE;
	
	AddressU = Wrap;
	AddressV = Wrap;
};

sampler FixedSamp1 = sampler_state
{
	Texture = <FixedTex1>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = NONE;
	
	AddressU = Wrap;
	AddressV = Wrap;
};

sampler FixedSampShadow = sampler_state
{
	Texture = <FixedTex0>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	
	AddressU = Clamp;
	AddressV = Clamp;
};



//-------------------------------------------------------------------------------------
// 셰이더 사용 테크닉
technique TShader
{
	pass P0
	{
		AlphaTestEnable = FALSE;
		AlphaBlendEnable = TRUE;
		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;
		Lighting = FALSE;
		CullMode = NONE;

		TexCoordIndex[0] = 0;
		TexCoordIndex[1] = 1;
		TexCoordIndex[2] = 2;
		TexCoordIndex[3] = 3;
		
		//TextureTransformFlags[0] = Disable;
		//TextureTransformFlags[1] = Disable;
		//TextureTransformFlags[2] = Disable;
		//TextureTransformFlags[3] = Disable;
		
		//Sampler[0] = (Samp0);
		//Sampler[1] = (Samp1);
		//Sampler[2] = (Samp2);
		//Sampler[3] = (Samp3);
		
		VertexShader = compile vs_1_1 VS();
		PixelShader = compile ps_2_0 PS();
	}
}

// 고정파이프라인 사용 테크닉
technique TFixed
{
	pass P0
	{
		vertexshader = NULL;
		pixelshader = NULL;
		
		AlphaOp[0]	 = SelectArg1;
		AlphaArg1[0] = Texture;
		ColorOp[0]   = SelectArg1;
		ColorArg1[0] = Texture;

		AlphaOp[1]   = SelectArg1;
		AlphaArg1[1] = Current;
		ColorOp[1]   = SelectArg1;
		ColorArg1[1] = Texture;

		AlphaTestEnable = FALSE;
		AlphaBlendEnable = TRUE;
		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;
		
		Sampler[0] = (FixedSamp0);
		Sampler[1] = (FixedSamp1);

		Lighting = FALSE;
		CullMode = NONE;
		
		TexCoordIndex[1] = PassThru;
		TextureTransformFlags[1] = Disable;
	}
	
	pass P1	// 스플래팅한 부위에 그림자 덮기
	{
		vertexshader = NULL;
		pixelshader = NULL;
		
		TexCoordIndex[0] = CameraSpacePosition;
		TextureTransformFlags[0] = Projected | Count4;

		AlphaBlendEnable = TRUE;
		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;

		AlphaTestEnable = FALSE;
		AlphaRef = 0x80;
		AlphaFunc = GreaterEqual;
		
		Lighting = FALSE;
		CullMode = NONE;

		ColorOp[0]   = SelectArg1;
		ColorArg1[0] = Texture;
		AlphaOp[0]   = SelectArg1;
		AlphaArg1[0] = Texture;
		
		Sampler[0] = (FixedSampShadow);
	}
}
