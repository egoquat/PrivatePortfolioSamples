//월드행렬
float4x4 m_World;
//뷰행렬
float4x4 m_View;
//프로젝션 행렬
float4x4 m_Proj;
//캐릭터의 이동행렬
float4x4 m_Move;
//입력정점 형식
float4x4 m_AniMatrix[61];
//텍스쳐
texture  m_Tex;

float	 m_TexFactor;
///////////////////여기 까지 캐릭터를 위한 변수들


//캐릭터 스테이트
sampler Samp = sampler_state
{
    Texture    =  <m_Tex>;
    MinFilter  =  LINEAR;
    MagFilter  =  LINEAR;
    MipFilter  =  LINEAR;
};
//입력 받을 구조체 형식
struct VS_INPUT
{
    float3 pos		: POSITION;
    float3 Normal	: NORMAL;
    float4 diff		: COLOR0;
    float2 tex		: TEXCOORD0;
    float4 index	: TEXCOORD1;
    float4 weight	: TEXCOORD2;
};
//출력 형식
struct VS_OUTPUT
{
	float4 pos		: POSITION;
   	float2 tex  	: TEXCOORD0;
};
//캐릭터 애니메이션을 위한 함수 
VS_OUTPUT VS (VS_INPUT In)           
{
    VS_OUTPUT Out = (VS_OUTPUT)0;
    
    float4x4 m_Mat = mul( mul( m_World, m_View), m_Proj);
    
    float3 fVertex = float3( 0, 0, 0 );
	fVertex += mul( float4(In.pos, 1), m_AniMatrix[In.index.x] ) * In.weight.x;
	fVertex += mul( float4(In.pos, 1), m_AniMatrix[In.index.y] ) * In.weight.y;
	fVertex += mul( float4(In.pos, 1), m_AniMatrix[In.index.z] ) * In.weight.z;
	fVertex += mul( float4(In.pos, 1), m_AniMatrix[In.index.w] ) * In.weight.w;
	
	Out.pos = mul( mul(float4(fVertex , 1), m_Move), m_Mat); 
		
	Out.tex = In.tex;
    
    return Out;
}
//애니메이션 정보는 없이 매쉬만 출력 할때의 함수
VS_OUTPUT VS2 (VS_INPUT In)           
{
    VS_OUTPUT Out = (VS_OUTPUT)0;
    
    float4x4 m_Mat = mul( mul( m_World, m_View), m_Proj);
  
	Out.pos = mul( mul(float4(In.pos, 1), m_Move), m_Mat); 

	Out.tex = In.tex;
    
    return Out;
}

float4 PS ( VS_OUTPUT In ) : COLOR
{
    float4 t = tex2D( Samp, In.tex );
    t.a = m_TexFactor;
   
    return t;
}

float4 PS2 ( VS_OUTPUT In ) : COLOR
{
    float4 t = tex2D( Samp, In.tex );
     
    return t;
}

// 그림자를 그릴때의 픽셀 셰이더
float4 PS_SHADOW ( VS_OUTPUT In ) : COLOR
{
	float4 color = (float4)0;
	color.a = 0.00002f;
    return color;
}

//캐릭터에 대한 테크닉 선언
technique Shader
{
    pass P0
    {
		//SetRenderState
		LASTPIXEL			= true;
		CullMode			= NONE;
		ZEnable				= true;
        ZWRITEENABLE		= true; 
        Lighting			= true;
		ALPHABLENDENABLE	= true;
		ALPHATESTENABLE		= true;
		SRCBLEND			= SRCALPHA;
		DESTBLEND			= INVSRCALPHA;
		ALPHAREF			= 0xe0;
		ALPHAFUNC			= GREATEREQUAL;
		TextureFactor		= <m_TexFactor>;

        // texture stages
		COLOROP[0]			=  MODULATE;	
		COLORARG1[0]		=  TEXTURE;
		COLORARG2[0]		=  DIFFUSE;
		ALPHAOP[0]			=  MODULATE;
		ALPHAARG1[0]		=  TEXTURE;	
		ALPHAARG2[0]		=  TFACTOR;
				
				
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS2();
    }
    pass P1
    {
		//SetRenderState
		LASTPIXEL			= true;
		CullMode			= NONE;
		ZEnable				= true;
        ZWRITEENABLE		= true; 
        Lighting			= true;
		ALPHABLENDENABLE	= true;
		ALPHATESTENABLE		= false;
		SRCBLEND			= SRCALPHA;
		DESTBLEND			= INVSRCALPHA;
		ALPHAREF			= 0xe0;
		ALPHAFUNC			= GREATEREQUAL;
		TextureFactor		= <m_TexFactor>;

        // texture stages
		COLOROP[0]			=  MODULATE;	
		COLORARG1[0]		=  TEXTURE;
		COLORARG2[0]		=  DIFFUSE;
		ALPHAOP[0]			=  MODULATE;
		ALPHAARG1[0]		=  TEXTURE;	
		ALPHAARG2[0]		=  TFACTOR;
				
				
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
    pass P2
    {
    
		ALPHABLENDENABLE		= false;
		CULLMODE				= NONE;
		ZENABLE					= false;
		
		TEXTUREFACTOR			= 0xff00ff;
		COLOROP[0]				= SELECTARG1;
		COLORARG1[0]			= TFACTOR;
		
		ALPHAOP[0]				= SELECTARG1;
		ALPHAARG1[0]			= TFACTOR;
		
		//그림자를 그려 주기 위한 패쑤!
	    VertexShader = compile vs_2_0 VS();
         PixelShader  = compile ps_2_0 PS_SHADOW();
    }
}

