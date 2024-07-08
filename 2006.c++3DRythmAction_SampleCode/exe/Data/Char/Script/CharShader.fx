//�������
float4x4 m_World;
//�����
float4x4 m_View;
//�������� ���
float4x4 m_Proj;
//ĳ������ �̵����
float4x4 m_Move;
//�Է����� ����
float4x4 m_AniMatrix[61];
//�ؽ���
texture  m_Tex;

float	 m_TexFactor;
///////////////////���� ���� ĳ���͸� ���� ������


//ĳ���� ������Ʈ
sampler Samp = sampler_state
{
    Texture    =  <m_Tex>;
    MinFilter  =  LINEAR;
    MagFilter  =  LINEAR;
    MipFilter  =  LINEAR;
};
//�Է� ���� ����ü ����
struct VS_INPUT
{
    float3 pos		: POSITION;
    float3 Normal	: NORMAL;
    float4 diff		: COLOR0;
    float2 tex		: TEXCOORD0;
    float4 index	: TEXCOORD1;
    float4 weight	: TEXCOORD2;
};
//��� ����
struct VS_OUTPUT
{
	float4 pos		: POSITION;
   	float2 tex  	: TEXCOORD0;
};
//ĳ���� �ִϸ��̼��� ���� �Լ� 
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
//�ִϸ��̼� ������ ���� �Ž��� ��� �Ҷ��� �Լ�
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

// �׸��ڸ� �׸����� �ȼ� ���̴�
float4 PS_SHADOW ( VS_OUTPUT In ) : COLOR
{
	float4 color = (float4)0;
	color.a = 0.00002f;
    return color;
}

//ĳ���Ϳ� ���� ��ũ�� ����
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
		
		//�׸��ڸ� �׷� �ֱ� ���� �о�!
	    VertexShader = compile vs_2_0 VS();
         PixelShader  = compile ps_2_0 PS_SHADOW();
    }
}

