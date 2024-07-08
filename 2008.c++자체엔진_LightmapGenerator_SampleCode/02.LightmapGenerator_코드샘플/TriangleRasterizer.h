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
									Vec2 *v2SecondUV,			// triangle의 정점당 second UV 좌표 (linked Group min max 1-0좌표)	
									Vec3 *v3Pos,				// triangle의 정점당 world position
									Vec3 *v3Normal,				// triangle의 정점당 v3Normal
									CAlphaTexture &CurAlphaTexture,	// alpha channel만 CurAlphaTexture.m_alpha_data[idx] 구성
									Vec2 *v2FirstUV  );			// parsing된 정점당 UV 좌표

	void RenderTriangleTexel_HorizontalLine( vector<CLightComp>& vecLights,
											CLinkedPolygon &linfo,
											CAlphaTexture &otex,
											tTexelState *start_edgeparam,
											tTexelState *end_edgeparam,
											bool mix_flag );

	DWORD ComputeLight_Render_TexelPoint(	vector<CLightComp>& vecLights,
											CLinkedPolygon &linfo,
											tTexelState *CurTexel );


	// 실 Ray 충돌되는 Tri들의 Alpha Visibility 갱신
	// arg1:light, 
	// arg2:CurLinkedPolygon, 
	// arg3:현재점 한개에 해당하는 edgeparameter, 
	// 해당 light가 node검색을 통해 보유한 visibility triangle set을 통해서
	// iResultVisibility -= aa; // 해당 지점의 alpha 값만큼 초기 255 에서 감산연산 으로 최종 visibility 수치 지정후 반환
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

	// 결과 lum -> lightmap[x,y] 적용 
	// : 이미 라이트가 미치는 범위 여부연산은 한 상태로 값이 넘어온다.
	// point light R,G,B color 지수 : attenuation 공식 적용
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
