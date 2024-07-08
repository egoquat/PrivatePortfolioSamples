#pragma once

#include "CommonWz.h"

#include "DefMath.h"

#include "DefLightmap.h"

#include "Triangle.h"

#include "_parserlightmapmesh.h"

#include "LightComp.h"

#include "KDTree.h"

#include "Global_.h"

#include "BoundingVolume.h"

struct tRasterTriangleCursor
{
	float start_v,end_v,v_delta;
	float v;
};

struct tTexelState
{
	Vec2 v2SecondUV, v2FirstUV;
	Vec3 v3Pos,v3Normal;
};

class CTriangleRasterizer
{
public:
	Vec3		m_v3TesselArr[MAX_A_TRI_TESSEL_CNT];
	CGlobal_*	m_pGlobal;
public:

	void DrawPoint(CTextureSegment *tex,int x,int y,DWORD co);

	void MixPoint(CTextureSegment *tex,int x,int y,DWORD co);

	void DrawPoint( CTextureSegment *tex,
						float u,
						float v,
						DWORD co);

	void DrawPoint_Blend( CTextureSegment *tex,
						float u,
						float v,
						DWORD co);

	void DrawPoly_AColor(CTextureSegment *tex, DWORD co);

	void CollectApproximateTri_RayTrace_DirectionalLight( const CKDTree	*pKDTreeRoot, 
														const Vec3		*arv3PolyPos,
														const UINT		uiRasterTriSeq,
														CLightComp		*pCurLight );

	void CollectApproximateTri_RayTrace_PointLight( const CKDTree	*pKDTreeRoot, 
													const Vec3		*arv3PolyPos,
													const UINT		uiRasterTriSeq,
													CLightComp		*pCurLight );

	// For Debug
	void CollectTriangel_LineDebug( const CKDTree	*pKDTreeRoot, 
									CLineDebugContainer &containerLineDebug );

	BOOL DeleteSelfTri( set<int>& vtris, UINT uiRasterTriSeq );

	Vec3 *TesselateRasterizationPos(const Vec3 *v3Pos,int *ntpos);

	//void RenderTriangle(int i,WzMeshGroup *pWzMeshGroup,_CParserLightMapMesh &oinstance);
	
	void RenderRasterizeTriangle(	vector<CLightComp>& vecLights,		// Light Component Container
									CLinkedPolygon &linfo,		// linked Group Info
									Vec2 *v2SecondUV,			// triangle�� ������ second UV ��ǥ (linked Group min max 1-0��ǥ)	
									Vec3 *v3Pos,				// triangle�� ������ world position
									Vec3 *v3Normal,				// triangle�� ������ v3Normal
									CAlphaTexture &CurAlphaTexture,	// alpha channel�� CurAlphaTexture.m_alpha_data[idx] ����
									Vec2 *v2FirstUV  );			// parsing�� ������ UV ��ǥ

	void RenderTriangleTexel_HorizontalLine( vector<CLightComp>& vecLights,
											CLinkedPolygon &linfo,
											CAlphaTexture &otex,
											tTexelState *start_edgeparam,
											tTexelState *end_edgeparam,
											bool mix_flag );

	DWORD ComputeLight_Render_TexelPoint(	vector<CLightComp>& vecLights,
											CLinkedPolygon &linfo,
											tTexelState *CurTexel );


	// �� Ray �浹�Ǵ� Tri���� Alpha Visibility ����
	// arg1:light, 
	// arg2:CurLinkedPolygon, 
	// arg3:������ �Ѱ��� �ش��ϴ� edgeparameter, 
	// �ش� light�� node�˻��� ���� ������ visibility triangle set�� ���ؼ�
	// iResultVisibility -= aa; // �ش� ������ alpha ����ŭ �ʱ� 255 ���� ���꿬�� ���� ���� visibility ��ġ ������ ��ȯ
	BOOL CheckVisibility_( set<int>& vtris,
							const Vec3& v3TexelPos,
							const Vec3& v3LightPos,
							const Vec3& v3LightDir,
							float fEpsilon_Offset_IntersectTri );

	int CheckVisibility_ProcessAlpha(set<int>& vtris,
							Vec3& v3TexelPos,
							Vec3& v3LightPos,
							Vec3& v3LightDir,
							float fLightIntensity,
							float fEpsilon_Offset_IntersectTri );

	// ��� lum -> lightmap[x,y] ���� 
	// : �̹� ����Ʈ�� ��ġ�� ���� ���ο����� �� ���·� ���� �Ѿ�´�.
	// point light R,G,B color ���� : attenuation ���� ����
	// tessel point color = original color * light intensity * attenuation * visibility factor
	//		attenuation : attenuation = (light.fAttenuation_Far - current fDistance_Square) / light.fAttenuation_Delta
	// arg1:current light
	// arg2:linked Group Infor
	// arg3:current texture info
	// arg4:tessel info(v3Pos,v3Normal,v2SecondUV, v2FirstUV)
	BOOL ComputePointLight_TexelPoint_	(CLightComp &light
										,CLinkedPolygon &linfo
										,tTexelState *CurTexel
										,DWORD &dwResultVisibility );

	DWORD ComputeDirectionalLight_TexelPoint_(CLightComp &light,
											CLinkedPolygon &linfo,
											tTexelState *CurTexel );

public:
		CTriangleRasterizer(void);
		~CTriangleRasterizer(void);
};
