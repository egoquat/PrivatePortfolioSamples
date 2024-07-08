#include ".\trianglerasterizer.h"


void CTriangleRasterizer::DrawPoint(CTextureSegment *tex,int x,int y,DWORD co)
{
	if (x<0 || x>= tex->_width) return;
	if (y<0 || y>= tex->_height) return;

	int idx = x + y * tex->_width;

	tex->texture_color_data[idx] = co;

}


void CTriangleRasterizer::MixPoint(CTextureSegment *tex,int x,int y,DWORD co)
{
	if (x<0 || x>= tex->_width) return;
	if (y<0 || y>= tex->_height) return;

	int idx = x + y * tex->_width;

	DWORD ot = tex->texture_color_data[idx];

	if (ot == co) return;

	int rr = (int(PickR(co)) + int(PickR(ot)))>>1;
	int gg = (int(PickG(co)) + int(PickG(ot)))>>1;
	int bb = (int(PickB(co)) + int(PickB(ot)))>>1;

	// co와 texture상에 요청지점의 color를 + 연산
	tex->texture_color_data[idx] = COLOR_RGBA(rr,gg,bb,255);
}


void CTriangleRasterizer::DrawPoint( CTextureSegment *tex,
			   float u,
			   float v,
			   DWORD co)
{
	DrawPoint(tex,
		int(float(tex->_width-1) * u + 0.5f),
		int(float(tex->_height-1) * v + 0.5f),
		co);
}


void CTriangleRasterizer::DrawPoint_Blend( CTextureSegment *tex,
					 float u,
					 float v,
					 DWORD co)
{ 
	MixPoint(tex,
		int(float(tex->_width-1) * u + 0.5f),
		int(float(tex->_height-1) * v + 0.5f),
		co);
}

void CTriangleRasterizer::DrawPoly_AColor(CTextureSegment *tex, DWORD co)
{
	UINT iIdx = 0;

	for( UINT iX = 0; iX < (UINT)tex->_width; ++iX )
	{
		for( UINT iY = 0; iY < (UINT)tex->_height; ++iY )
		{
			iIdx = ( iY * tex->_width ) + iX;

			tex->texture_color_data[iIdx] = co;
		}
	}
}


// light별 추정되는 visible tringle들을 gathering 한다.
// triangle tesselation position 에 모든 각 light별로 
// 모든 하위 노드를 검색해서 해당 tesselation position 과 light position 둘중 하나라도
// 존재하는 노드에 속해 있는 모든 triangle index들을 해당 light의 visible triangle set collect
// 의문사항:light와 vertex사이간 intersection 검증은 왜 않하는지?
void CTriangleRasterizer::CollectApproximateTri_RayTrace_DirectionalLight( 
													const CKDTree	*pKDTreeRoot, 
													const Vec3		*arv3PolyPos,
													const UINT		uiRasterTriSeq,
													CLightComp *pCurLight )
{	
	pCurLight->bHasApproximatedCastShadowTri = true;
	{
		// to do : 1 미터 보다 큰 삼각형은
		// 쪼개서 체크하도록 바꿔야함.
		int ntpos;

		// 현재 triangle의 각 면적을 tesselation으로 구성
		Vec3 *tpos = TesselateRasterizationPos(arv3PolyPos,&ntpos);

		if( 0 < pCurLight->vtris.size() )
		{
			pCurLight->vtris.clear();		// visible triangle set을 blank
		}
		Vec3 v3Pos, v3Light;
		int i=0,ni = ntpos;

		// Shadow 최적화 사용 : Texel 두개중 하나는 건너뜀
		if( TRUE == m_pGlobal->g_pLMOption->_bUsingShadowOptimization)
		{
			for( i = 0 ; i < ni ; ++i )
			{
				if( 0 != i % 2  )
				{
					// light_1000 최대 길이값을 지점에서 뺀 만큼을 임시적으로 라이트 지점으로 취급
					// 계산 방식은 point pCurLight와 동일
					v3Pos = tpos[i];
					v3Light = v3Pos - pCurLight->v3Dir_Far;

					// 추정되는 triangle들만 수집한다.
					LineSegment		LSeg;
					LSeg.CalSegment( v3Light, v3Pos );

					const_cast<CKDTree*>(pKDTreeRoot)->CollectTriangleFromNode_Slab( LSeg,pCurLight->vtris );
					//const_cast<CKDTree*>(pKDTreeRoot)->CollectTriangleFromNode(v3Pos,v3Light,pCurLight->vtris);

					//const_cast<CKDTree*>(pKDTreeRoot)->CollectVisibleNodesChecker( v3Pos, v3Light, pCurLight->vtris);	
					//const_cast<CKDTree*>(pKDTreeRoot)->CollectVisibleNodes_Temp(v3Pos,v3Light,pCurLight->vtris);
				}
			}
		}
		else
		{
			for( i = 0 ; i < ni ; ++i )
			{
				// light_1000 최대 길이값을 지점에서 뺀 만큼을 임시적으로 라이트 지점으로 취급
				// 계산 방식은 point pCurLight와 동일
				v3Pos = tpos[i];
				v3Light = v3Pos - pCurLight->v3Dir_Far;

				// 추정되는 triangle들만 수집한다.
				LineSegment		LSeg;
				LSeg.CalSegment( v3Light, v3Pos );

				const_cast<CKDTree*>(pKDTreeRoot)->CollectTriangleFromNode_Slab( LSeg,pCurLight->vtris );
				//const_cast<CKDTree*>(pKDTreeRoot)->CollectTriangleFromNode(v3Pos,v3Light,pCurLight->vtris);
				//const_cast<CKDTree*>(pKDTreeRoot)->CollectVisibleNodesChecker( v3Pos, v3Light, pCurLight->vtris);	
				//const_cast<CKDTree*>(pKDTreeRoot)->CollectVisibleNodes_Temp(v3Pos,v3Light,pCurLight->vtris);
			}
		}
	}

	//// 자기 자신의 Raster Tri는 제외 시킨다.
	//if( TRUE == pCurLight->bHasApproximatedCastShadowTri )
	//{
	//	DeleteSelfTri( pCurLight->vtris, uiRasterTriSeq );
	//}

#if SHOW_DEBUG_MODE

	if( pCurLight->vtris.size() > 0 )
	{
		char		szDebug[1024];
		UINT		uiBuf = 0, uiCount = 0;

		set<int>::iterator it, et;
		uiCount = pCurLight->vtris.size();

		it = pCurLight->vtris.begin();		et = pCurLight->vtris.end();

		uiBuf = uiBuf + sprintf( szDebug + uiBuf, " CollectVisibleTriFromNodes 검색된 Triangle 갯수 : %d", uiCount );

		uiBuf = uiBuf + sprintf( szDebug + uiBuf, "\n" );

		OutputDebugString( szDebug );
	}

#endif
}



void CTriangleRasterizer::CollectApproximateTri_RayTrace_PointLight( 
																const CKDTree	*pKDTreeRoot, 
																const Vec3		*arv3PolyPos,
																const UINT		uiRasterTriSeq,
																CLightComp		*pCurLight )
{	
	pCurLight->bHasApproximatedCastShadowTri = true;

	int ntpos;								
	// position 을 기준으로 tesselatePosition 을 가져온다?
	// edge의 가장 긴변으로 지정된 만큼 정사각형 접근으로 각 position 위치값
	// position0 번을 기준으로 1차원 배열인 g_pos[] 를 구성 하는데
	// g_pos 의 index는 텍셀 계산된 width와 height값에 현재 지점의 가로 지점과 세로 지점이 반영되고,
	// 실제 vector 위치 값에는 각 edge의 길이 * current 0.0f ~ 1.0f 비율적용된 지점 구성 
	// arv3PolyPos[0] + a * ar + b * br;로 계산
	// ntpos = tesselateposition 갯수
	Vec3 *tpos = TesselateRasterizationPos(arv3PolyPos,&ntpos);	// ntpos : tri의 전체 tessel 수	// Tesselate Rasterization Pos로 변환하여 적재 

	if( 0 < pCurLight->vtris.size() )
	{
		pCurLight->vtris.clear();
	}

	Vec3 v3Pos, v3Light;

	// triangle이 가진 모든 1차원 tesselation position 접근 
	for( int i = 0 ; i < ntpos ; ++i )
	{
		v3Pos = tpos[i];
		v3Light = pCurLight->position;
		
		// 노드별 추정 triangle들만 수집
		LineSegment		LSeg;
		LSeg.CalSegment( v3Light, v3Pos );

		const_cast<CKDTree*>(pKDTreeRoot)->CollectTriangleFromNode_Slab( LSeg, pCurLight->vtris);
		//const_cast<CKDTree*>(pKDTreeRoot)->CollectTriangleFromNode( v3Pos, v3Light, pCurLight->vtris);
		//const_cast<CKDTree*>(pKDTreeRoot)->CollectVisibleNodesChecker( v3Pos, v3Light, pCurLight->vtris);	
		//const_cast<CKDTree*>(pKDTreeRoot)->CollectVisibleNodes_Temp( v3Pos, v3Light, pCurLight->vtris);
	}

	// 자기 자신의 Raster Tri는 제외 시킨다.
	/*if( TRUE == pCurLight->bHasApproximatedCastShadowTri )
	{
		DeleteSelfTri( pCurLight->vtris, uiRasterTriSeq );
	}*/

#if SHOW_DEBUG_MODE

	if( pCurLight->vtris.size() > 0 )
	{
		char		szDebug[1024];
		UINT		uiBuf = 0, uiCount = 0;

		set<int>::iterator it, et;
		uiCount = pCurLight->vtris.size();

		it = pCurLight->vtris.begin();		et = pCurLight->vtris.end();

		uiBuf = uiBuf + sprintf( szDebug + uiBuf, " CollectVisibleTriFromNodes 검색된 Triangle 갯수 : %d", uiCount );

		uiBuf = uiBuf + sprintf( szDebug + uiBuf, "\n" );

		OutputDebugString( szDebug );
	}

#endif
}



void CTriangleRasterizer::CollectTriangel_LineDebug( const CKDTree	*pKDTreeRoot, 
													CLineDebugContainer &containerLineDebug )
{	
	
	for( UINT ui_ = 0; ui_ < containerLineDebug.GetSize(); ++ui_ )
	{
		stLineDebug		&LineDebug = containerLineDebug.Get( ui_ );

		LineSegment		LSeg;
		LSeg.CalSegment( LineDebug.v3LStart, LineDebug.v3LEnd );

		//const_cast<CKDTree*>(pKDTreeRoot)->CollectTriangleFromNode_Slab_LineDebug( LSeg );
		const_cast<CKDTree*>(pKDTreeRoot)->CollectTriangleFromNode_LineDebug( LSeg );
	}

	
}



BOOL CTriangleRasterizer::DeleteSelfTri( set<int>& vtris, UINT uiRasterTriSeq )
{
	set<int>::iterator it;
	int		iCur = 0;
	it = vtris.find( uiRasterTriSeq );	

	if( vtris.end() == it ) 
	{
		return FALSE;
	}

	iCur = *it;

	vtris.erase( it );

	return TRUE;
}


// 현재 삼각형 pos1,2,3 에 면적에 대한 모든 Tessel 지점들 저장
// arg1 : triangle 의 world position 
Vec3* CTriangleRasterizer::TesselateRasterizationPos(const Vec3 *v3Pos,int *ntpos)
{
	// triangle 의 1번 edge의 길이
	float delta = (v3Pos[0]-v3Pos[1]).Length();
	float delta_max;

	delta_max = delta;

	// triangle 의 2번 edge의 길이
	delta = (v3Pos[1]-v3Pos[2]).Length();

	if (delta > delta_max)
		delta_max = delta;

	// triangle 의 3번 edge의 길이
	delta = (v3Pos[2]-v3Pos[0]).Length();

	// 가장 긴 길이의 edge를 deltamax로 설정
	if (delta > delta_max)
		delta_max = delta;

	//int ntess = 5 * delta_max / (DEFAULT_KDTREERES);
	int ntess = delta_max;

	*ntpos = 0;

	// edge 1, 2 directional vector
	Vec3 vEdgeA = v3Pos[1] - v3Pos[0];		// 
	Vec3 vEdgeB = v3Pos[2] - v3Pos[1];

	// 각 tesselation 지점의 0.0 ~ 1.0 사이 비율 
	int i=0,ni = ntess + 1, iTemp = 0;		// Edge길이 접근 
	UINT	uiNi = ni * ni;

	if( uiNi > MAX_A_TRI_TESSEL_CNT )
	{
		char		szCriticalMsg[256];
		sprintf( szCriticalMsg, "현재의 Texel Count는 %d이고, 최대 texel 버퍼 Count %d로 최대 Texel 버퍼 Count를 늘려야 합니다. 강제로 하향 조정 합니다.", 
			MAX_A_TRI_TESSEL_CNT, uiNi );

		//MessageBox(m_pGlobal->g_hwnd, szCriticalMsg,"...",MB_OK);
		//OutputDebugString( szCriticalMsg );

		iTemp = ni;

		while( uiNi > MAX_A_TRI_TESSEL_CNT )
		{
			--iTemp;
			uiNi = iTemp * iTemp;
		}

		ni = iTemp;
	}

	if( 1 < ni )
	{
		for( i = 0 ; i < ni ; ++i )
		{
			int j = 0 ,nj = i+1;			// Edge길이로 접근되는 현재 값 + 1만큼
			for( j = 0 ; j < nj ; ++j )
			{
				float ar,br;
				ar = (float(i)/float(ni-1));		// 0.0f ~ 1.0f 비율
				if (nj ==1)							 
					br = 0;
				else
					br = (float(j)/float(nj-1));	// 0.0f ~ 1.0f 비율

				// 실제 현재 지점의 위치 값으로 지정 
				// 3d 좌표 길이만큼 각 지점의 실제 위치값으로 변환
				// triangle의 0번 position + (edge 1번 * (current 0.0f ~ 1.0f 비율))
				//						   + (edge 2번 * (current 0.0f ~ 1.0f 비율))
				m_v3TesselArr[(*ntpos)++] = v3Pos[0] + vEdgeA * ar + vEdgeB * br;	
				//(*ntpos)++;		// 순번 index
			}
		}
	}
	else
	{
		m_v3TesselArr[(*ntpos)++] = v3Pos[0];
	}
	
	return &m_v3TesselArr[0];
}

// -. first UV와 Second UV :1. second UV 좌표는 linked Group min, max boundary 상에 위치한 
//							현재 triangle의 각 정점의 상대적 위치 값
//							2. first UV 의 경우 parsing을 통해 읽어온 texture상에 UV 좌표
// triangle 빗변별 tessel color와 light color 연산을 통해 CurLinkedPolygon.m_tex->m_tex_data[i].color 구성

// 1.rasteraization 2.calculate lumel 
// 3.calculate visiblity color from gathering triangle 4.draw Texel Color
void CTriangleRasterizer::RenderRasterizeTriangle( vector<CLightComp>& vecLights,		// Light Component Container
													CLinkedPolygon &linfo,		// linked Group Info
													Vec2 *v2SecondUV,			// triangle의 정점당 second UV 좌표 (linked Group min max 1-0좌표)	
													Vec3 *v3Pos,				// triangle의 정점당 world position
													Vec3 *v3Normal,				// triangle의 정점당 v3Normal
													CAlphaTexture &CurAlphaTexture,	// alpha channel만 CurAlphaTexture.m_alpha_data[idx] 구성
													Vec2 *v2FirstUV )		// parsing된 정점당 UV 좌표
{
	const int big_tri = 100*100;
	bool verbose = false;
	char old_text[1024],buf[1024];
	int line_counter = 0;
	const int freq = 10;

	// triangle 텍스쳐 면적이 지정된 10000 보다 크면 error
	if (linfo.m_tex._width * linfo.m_tex._height > big_tri)
	{
		verbose = true;
		GetWindowText(m_pGlobal->g_hwnd,old_text,1024);
	}

	verbose = false;
	

	CTextureSegment &rtex = linfo.m_tex;
	float tex_height = float(linfo.m_tex._height);		// linkinfo Group 상에 texture height


	const float fTOOSMALL_EDGELENGTH = 1.0;

	DWORD tick = timeGetTime();


	int pre_index[3] = {0,1,2};
	bool do_A_phase_flag = true;
	bool do_B_phase_flag = true;

	// 1. 일단 y 높이 크기 순서로 소트 : y 높이 기준으로  0 < 1 < 2 정렬
	// 2. UV 길이값이 너무 작은경우 플래그 = false
	//	  Edge A Validate flag, Edge B Validate flag
	if (v2SecondUV[pre_index[0]].y > v2SecondUV[pre_index[1]].y) Swapi(pre_index[0],pre_index[1]);
	if (v2SecondUV[pre_index[0]].y > v2SecondUV[pre_index[2]].y) Swapi(pre_index[0],pre_index[2]);
	if (v2SecondUV[pre_index[1]].y > v2SecondUV[pre_index[2]].y) Swapi(pre_index[1],pre_index[2]);

	// 2. UV 길이값이 너무 작은경우 ( 현재 값 1.0f ) 0-1간 또는 2-1간 길이가 너무 작으면 계산하지 않는다.
	if ((v2SecondUV[pre_index[1]].y - v2SecondUV[pre_index[0]].y) * tex_height < fTOOSMALL_EDGELENGTH) 
	{
		do_A_phase_flag = false;
	}
		
	if ((v2SecondUV[pre_index[2]].y - v2SecondUV[pre_index[1]].y) * tex_height < fTOOSMALL_EDGELENGTH) 
	{
		do_B_phase_flag = false;
	}
		

	tRasterTriangleCursor	scur,ecur;
	tTexelState		StartTexel,EndTexel,poll,polla,sdelta,edelta;

	// y 의 높이로 ascending sorting 순서 index 값들
	int idx_0 = pre_index[0];	
	int idx_1 = pre_index[1];	
	int idx_2 = pre_index[2];	

	// 상단으로부터 첫번째 빗변  그릴것이 있는 경우 : 빗변 a-b
	if (do_A_phase_flag)
	{
		poll.v2FirstUV		= v2FirstUV[idx_0];				// first UV a   : 자신이 보유한 Texture의 UV
		poll.v2SecondUV		= v2SecondUV[idx_0];			// second UV a  : Light 계산되어 입혀질 UV
		poll.v3Pos			= v3Pos[idx_0];					// position a	: Triangle 0번 Vertex 위치
		poll.v3Normal		= v3Normal[idx_0];				// position v3Normal a

		// cursor 계산 : 위치값의 y값만으로 계산됨			// index a-b, 
		scur.start_v		= v2SecondUV[idx_0].y * tex_height;	// 
		scur.end_v			= v2SecondUV[idx_1].y * tex_height;	// UV 위치
		scur.v_delta		= scur.end_v - scur.start_v;			// b -> a 거리벡터 
		scur.v				= scur.start_v;							// UV 시작값

														// index a-c
		ecur.start_v		= v2SecondUV[idx_0].y * tex_height;	// 텍스쳐 상에 index a 의 실제 y 값
		ecur.end_v			= v2SecondUV[idx_2].y * tex_height;	//		"			 c			"
		ecur.v_delta		= ecur.end_v - ecur.start_v;		// c -> a 간 거리 방향 벡터
		ecur.v				= ecur.start_v;							// a 의 텍스쳐상에 실제 y 시작값

		// delta 계산									// index a-b
		sdelta.v3Pos		= v3Pos[idx_1] - v3Pos[idx_0];			// world position 상의 b의 a를 0으로 a에 대한 상대 위치값
		sdelta.v3Normal		= v3Normal[idx_1] - v3Normal[idx_0];	// v3Normal b - v3Normal a = delta v3Normal	: 상대 v3Normal 값
		sdelta.v2SecondUV	= v2SecondUV[idx_1] - v2SecondUV[idx_0];	// second b - second a = delta sconduv
		sdelta.v2FirstUV	= v2FirstUV[idx_1] - v2FirstUV[idx_0];	// parsing UV b - parsing UV a	: UV Texture상에 b의 a를 0기준으로 상대적인 y값

														// index a-c
		edelta.v3Pos		= v3Pos[idx_2] - v3Pos[idx_0];			// worldposition상에 c의 a를 0으로 a 에 대한 상대 위치값
		edelta.v3Normal		= v3Normal[idx_2] - v3Normal[idx_0];	// c의 a에 대한 v3Normal 차감 edelta v3Normal : 상대 v3Normal 값
		edelta.v2SecondUV	= v2SecondUV[idx_2] - v2SecondUV[idx_0];	// second UV의 a를 0으로 a에 대한 상대 위치값
		edelta.v2FirstUV	= v2FirstUV[idx_2] - v2FirstUV[idx_0];	// parsing UV c - a



		int loop_counter = 0;


		// texture_height 접근 // 세로로 그림
		while(true)
		{
			loop_counter++;

			// 현재 triangle의 linked group 상에 상대 위치값으로 계산된 텍스쳐상에 a, c 간 거리가 0 인경우 
			if (ecur.v_delta == 0)
			{
				break;
			}

			//if (scur.v > scur.end_v + DEFAULT_OVEREDGE)
			if (scur.v > scur.end_v + m_pGlobal->g_pLMOption->_fOverEdge)
			{
				break;
			}

#if CHECKVALID_TRIRASTERIZED_SIZE
			if (loop_counter > tex_height *2 )
			{
				char buf[1024];
				sprintf(buf,"invalid loop A, face index = %d",m_pGlobal->g_invalid__face_index);
				MessageBox(NULL,buf,"...",MB_OK);
				exit(0);
			}	
#endif

			// edge 의 위치 상에 진행율
			float sr = (scur.v - scur.start_v)		/	scur.v_delta;	// a-b간 : current v의 texture상에 위치를 비율로	
			float er = (ecur.v - ecur.start_v)		/	ecur.v_delta;	// a-c간 : 				"

			// first UV a + (a->b)v2FirstUV 상대 길이값 * current v의 상대 위치값
			// texture 의 current linear 된 UV 위치값, position, v3Normal 값

			// 1. 초기 값a->b를 향한 현재의 어느 지점, v3Normal, first UV, second UV a-b간
			StartTexel.v2FirstUV	= poll.v2FirstUV	+ (sdelta.v2FirstUV) * sr;	
			StartTexel.v2SecondUV	= poll.v2SecondUV	+ (sdelta.v2SecondUV) * sr;
			StartTexel.v3Pos		= poll.v3Pos		+ (sdelta.v3Pos) * sr;
			StartTexel.v3Normal		= poll.v3Normal		+ (sdelta.v3Normal) * sr;

			// 2. 초기 값a->c를 향한 현재의 어느 지점, v3Normal, first UV, second UV a-c간
			EndTexel.v2FirstUV		= poll.v2FirstUV	+ (edelta.v2FirstUV) * er;		// 초기 정의 된 edge 길이 상에 현재 비율적용
			EndTexel.v2SecondUV		= poll.v2SecondUV	+ (edelta.v2SecondUV) * er;
			EndTexel.v3Pos			= poll.v3Pos		+ (edelta.v3Pos) * er;
			EndTexel.v3Normal		= poll.v3Normal		+ (edelta.v3Normal) * er;

			// 가로로 그려라 즉 rasterilzetion line 으로 그려라
			// arg1:linkGroupInfo, 
			// arg2:CurAlphaTexture:alpha만 갱신된 texture buffer
			// arg3:StartTexel:a->b간 현재 지점, 
			// arg4:EndTexel:a->c간 현재 지점, 비율상에 현재 지점 : 비율0 이면 false
			// tessel triangle의 가로로 그려나가며 
			// CurLinkedPolygon.m_tex->m_tex_data[i].color 구성
			RenderTriangleTexel_HorizontalLine(vecLights,
												linfo,
												CurAlphaTexture,
												&StartTexel,
												&EndTexel,
												false);

			// 몇번에 나누어 그릴건지
			scur.v += (1.0f);	
			ecur.v += (1.0f);
		}
	}



	// 상단으로부터 두번째 빗변  그릴것이 있는 경우 : 빗변 b-c
	if (do_B_phase_flag)
	{
		polla.v2FirstUV = v2FirstUV[idx_1];		
		polla.v2SecondUV = v2SecondUV[idx_1];
		polla.v3Pos = v3Pos[idx_1];
		polla.v3Normal = v3Normal[idx_1];


		// cursor 계산
		scur.start_v = v2SecondUV[idx_1].y * tex_height;
		scur.end_v = v2SecondUV[idx_2].y * tex_height;
		scur.v_delta = scur.end_v - scur.start_v;
		scur.v = scur.start_v;

		// delta 계산
		sdelta.v3Pos = v3Pos[idx_2] - v3Pos[idx_1];
		sdelta.v3Normal = v3Normal[idx_2] - v3Normal[idx_1];
		sdelta.v2SecondUV = v2SecondUV[idx_2] - v2SecondUV[idx_1];
		sdelta.v2FirstUV = v2FirstUV[idx_2] - v2FirstUV[idx_1];


		if (!do_A_phase_flag)
		{
			poll.v2FirstUV = v2FirstUV[idx_0];
			poll.v2SecondUV = v2SecondUV[idx_0];
			poll.v3Pos = v3Pos[idx_0];
			poll.v3Normal = v3Normal[idx_0];

			// delta 계산
			edelta.v3Pos = v3Pos[idx_2] - v3Pos[idx_0];
			edelta.v3Normal = v3Normal[idx_2] - v3Normal[idx_0];
			edelta.v2SecondUV = v2SecondUV[idx_2] - v2SecondUV[idx_0];
			edelta.v2FirstUV = v2FirstUV[idx_2] - v2FirstUV[idx_0];

		}


		// cursor 계산
		ecur.start_v = v2SecondUV[idx_0].y * tex_height;
		ecur.end_v = v2SecondUV[idx_2].y * tex_height;
		ecur.v_delta = ecur.end_v - ecur.start_v;
		ecur.v = scur.start_v;




		int loop_counter = 0;

		// 각 Edge 길이 * 2 만큼 Looping 된다.
		while(true)
		{
			loop_counter++;

			if (ecur.v_delta == 0)
			{
				break;
			}

			//if (scur.v > scur.end_v + DEFAULT_OVEREDGE) // 줄 끝에 도착?
			if (scur.v > scur.end_v + m_pGlobal->g_pLMOption->_fOverEdge)
			{
				break;
			}

#if CHECKVALID_TRIRASTERIZED_SIZE
			if (loop_counter > tex_height *2)
			{
				char buf[1024];
				sprintf(buf,"invalid loop B, face index = %d",m_pGlobal->g_invalid__face_index);
				MessageBox(NULL,buf,"...",MB_OK);
				exit(0);
			}	
#endif

			float sr = (scur.v - scur.start_v)	/	scur.v_delta;
			float er = (ecur.v - ecur.start_v)	/	ecur.v_delta;

			StartTexel.v2FirstUV	= polla.v2FirstUV	+ (sdelta.v2FirstUV) * sr;
			StartTexel.v2SecondUV	= polla.v2SecondUV	+ (sdelta.v2SecondUV) * sr;
			StartTexel.v3Pos		= polla.v3Pos		+ (sdelta.v3Pos) * sr;
			StartTexel.v3Normal		= polla.v3Normal	+ (sdelta.v3Normal) * sr;

			EndTexel.v2FirstUV		= poll.v2FirstUV	+ (edelta.v2FirstUV) * er;
			EndTexel.v2SecondUV		= poll.v2SecondUV	+ (edelta.v2SecondUV) * er;
			EndTexel.v3Pos			= poll.v3Pos		+ (edelta.v3Pos) * er;
			EndTexel.v3Normal		= poll.v3Normal		+ (edelta.v3Normal) * er;

			// 줄 raster 호출

			RenderTriangleTexel_HorizontalLine(vecLights,
												linfo,
												CurAlphaTexture,
												&StartTexel,
												&EndTexel,
												false);

			scur.v += (1.0f);
			ecur.v += (1.0f);
		}
	}




	if (verbose)
		SetWindowText(m_pGlobal->g_hwnd,buf);
}

// tessel triangle의 가로로 그려나가며 CurLinkedPolygon.m_tex->m_tex_data[i].color 구성
// arg1:linkGroupInfo, 
// arg2:CurAlphaTexture:alpha만 갱신된 texture buffer
// arg3:StartTexel:a->b간 현재 지점, 
// arg4:EndTexel:a->c간 현재 지점, 
// arg5:tesseltriangle height 값이 0이 아니면 true : 0번은 mix할게 없다.
// 1. 비율상에 현재 지점 : 비율0 이면 false
//					sep와 EndTexel:triangle의 a->b 또는 a->c를 향한 어느 지점상에 계산되어진 
//					position, v3Normal, v2FirstUV, v2SecondUV 존재
// 2. tessel 상의 light type 별, anti alias 별 접근으로 light color 값을 연산하고 
// 3. CurLinkedPolygon.m_tex->m_tex_data[i].color 로 light color 적용

// linkinfo.tex->m_tex_data[idx] (color) -> 최종 구성
void CTriangleRasterizer::RenderTriangleTexel_HorizontalLine( vector<CLightComp>& vecLights,
																CLinkedPolygon &linfo,
																CAlphaTexture &otex,
																tTexelState *StartTexel,
																tTexelState *EndTexel,
																bool mix_flag)
{
	// 미리 rgba(255,255,255,0)으로 allocate 된 TrgTexInfo
	CTextureSegment &rtex = linfo.m_tex;	

	tTexelState CurPointTexel, EdgeDeltaTexel;

	// a->b v2SecondUV.x > a->c v2SecondUV.x  swap : tessel은 왼쪽에서 오른쪽으로 그려나가야 한다.
	if (StartTexel->v2SecondUV.x > EndTexel->v2SecondUV.x)
	{
		tTexelState *a = StartTexel;StartTexel = EndTexel;EndTexel = a;
	}

	// current s->v2SecondUV.x(0.0-1.0)에 보정된 width값을 곱함
	//		rtex._width : 보정된 width값은 linkedGroup의 min max 간 각각의 축에 길이값에 보정 연산됨
	//		v2SecondUV.d : linkedGroup의 min max간 현재 정점의 0.0-1.0 사이의 위치값
	float start_secondU	= StartTexel->v2SecondUV.x * rtex._width;		// current s->v2SecondUV.x에 tessel width값을 곱함?
	float end_secondU	= EndTexel->v2SecondUV.x * rtex._width;		// current e->v2SecondUV.x에 		"
	float delta_secondu	= fabs(end_secondU - start_secondU);				// current tessel 지점의 edge의 현재점에 작은쪽에서 큰쪽으로 edge
	float cursor_u		= start_secondU;									// tessel render 시작점

	const float TOOSMALL_HORIZONTALLEN = 0.5;

	if (delta_secondu < TOOSMALL_HORIZONTALLEN)						// 그릴려는 지점사이길이 가 0.5 이상은 되어야 한다.
		return;


	if (cursor_u < 0)
		cursor_u = 0;

	// 각 요소간 start-end 간 길이방향 EdgeDeltaTexel 벡터 
	EdgeDeltaTexel.v2SecondUV		= EndTexel->v2SecondUV - StartTexel->v2SecondUV;				// second UV
	EdgeDeltaTexel.v2FirstUV		= EndTexel->v2FirstUV	- StartTexel->v2FirstUV;				// first UV
	EdgeDeltaTexel.v3Pos			= EndTexel->v3Pos		- StartTexel->v3Pos;				// position
	EdgeDeltaTexel.v3Normal			= EndTexel->v3Normal	- StartTexel->v3Normal;	// v3Normal

	// anti_factor 만큼 triangle의 각 지점이 나누어 현재 함수호출되며, 현재 1로 설정 
	//DWORD texel[ANTI_FACTOR];						
	DWORD co;

	int loop_counter = 0;

	// 가로의 각 점 하나씩 그려나감
	while(1)
	{
		loop_counter++;

		// 현재 cursor가 텍셀 width 에 대한 validate
		if (cursor_u > rtex._width +1)
		{
			break;
		}
		//if (cursor_u > end_secondU + DEFAULT_OVEREDGE)
		if (cursor_u > end_secondU + m_pGlobal->g_pLMOption->_fOverEdge)
		{
			break;
		}


#if CHECKVALID_TRIRASTERIZED_SIZE			// CHECKVALID_TRIRASTERIZED_SIZE = 0
		if (loop_counter>rtex._width * 2 +5)
		{
			char buf[1024];
			sprintf(buf,"invalid loop here, face index = %d",m_pGlobal->g_invalid__face_index);
			MessageBox(NULL,buf,"...",MB_OK);
			exit(0);
		}	
#endif

		float rr;

		rr = (cursor_u - start_secondU) / delta_secondu;			// tessel 가로 상에 실제 현재 지점 비율

		// 요소별 현재 비율로 실제 지점을 갱신
		CurPointTexel.v2FirstUV 	= StartTexel->v2FirstUV		+ EdgeDeltaTexel.v2FirstUV	* rr;			// v2FirstUV 
		CurPointTexel.v2SecondUV	= StartTexel->v2SecondUV	+ EdgeDeltaTexel.v2SecondUV	* rr;	// v2SecondUV
		CurPointTexel.v3Pos 		= StartTexel->v3Pos			+ EdgeDeltaTexel.v3Pos		* rr;				// position 
		CurPointTexel.v3Normal 		= StartTexel->v3Normal		+ EdgeDeltaTexel.v3Normal	* rr;			// v3Normal
		CurPointTexel.v3Normal.SafeNormalize();

		// arg1:linkedGroupInfo,
		// arg2:CurAlphaTexture:alpha만 갱신된 texture buffer
		// arg3:현재 지점의 요소정보
		// texel 지점의 light type별 최종 color 연산
		// directional light color = tessel color + (light.color * visibility factor * light intensity)
		// point light color = tessel color + (light.color * visibility factor * light intensity)
		//texel[i] = ComputeLight_Render_TexelPoint(vecLights, linfo, &CurPointTexel);
		co = ComputeLight_Render_TexelPoint(vecLights, linfo, &CurPointTexel);
		cursor_u += (1.0f);

		// tessel color에 light color를 적용
		//		1. linkedGroupInfo가 보유한 texture의 v2SecondUV의 현재 지점 color 
		//		2. 최종 light color
		// texel 상에 현재 지점 + 0.5
		// CurLinkedPolygon.m_tex->m_tex_data[i].color 로 light color 갱신
		// linkinfo.tex->m_tex_data[idx] = co;
		if (mix_flag)
			DrawPoint_Blend(&linfo.m_tex, CurPointTexel.v2SecondUV.x, CurPointTexel.v2SecondUV.y, co);
		else
			DrawPoint(&linfo.m_tex, CurPointTexel.v2SecondUV.x, CurPointTexel.v2SecondUV.y, co);

	}

}

// triangle tesselation position 실제 어떤 지점을 그림
// arg1:linkedGroupInfo,
// arg2:CurAlphaTexture:alpha만 갱신된 texture buffer
// arg3:현재 지점의 요소정보
// light를 적용한 tessel 상에 현재 지점의 색깔은 
//		1. shodow texture에서 가져온 visibility 속성과
//		2. light 와 tessel position 간에 
// return value : tessel point rgb * 최종 light color rgb
//	
//		. directional light color = tessel color + (light.color * visibility factor * light intensity)
//		. point light color = tessel color + (light.color * visibility factor * light intensity)
//	. light color 에 ambient color 적용
DWORD CTriangleRasterizer::ComputeLight_Render_TexelPoint( vector<CLightComp>& vecLights,
															CLinkedPolygon &linfo,
															tTexelState *CurTexel )
{

	int r,g,b,a;

	r=g=b=0;
	a = 255;
	
	// 각 라이트 별로 접근하여 지점이 받는 최종 COlor를 계산해낸다.
	// accumulate lights
	//for( int i = 0; i < (int)m_pGlobal->g_light_array.GetSize(); ++i )
	for( int i = 0; i < (int)vecLights.size(); ++i )
	{
		// 현재 라이트의 유효성 여부 검증 
		//CLightComp &light = m_pGlobal->g_light_array.Get_(i);
		//CLightComp &light = m_pGlobal->g_light_array[i];
		CLightComp &light = vecLights[i];

		if (light.enabled == false || light.bDoesAffectionLighting == FALSE)
			continue;

		DWORD rv = 0;

		// type directional, point간 처리
		switch(light.eLightType)
		{
		case LIGHTTYPE_DIRECTIONAL:
			{
				// directional light 
				// 현재 tessel position 지점에 최종 light 지수 = visibility + lux(p.v3Normal dot l.dir)
				// 1. light.color 
				// 2. visibility factor :  라이트가 보유한 모든 가시성 triangle들에 접근하여 
				//							directional light간 충돌이 난 지점이 있으면
				//							texture 상에 해당 지점의 alpha 값을 visiblity 255 지수에서 차감 시켜 
				//							visibility 수치를 설정, 즉 shadow factor
				// 3. light intensity :  (- (dot(light v3Normal, tesselpositionnormal) /2 + 0.5) * light.fLightIntensity_ )
				// light color = light.color * visibility factor * light intensity
				// arg1:current light,						// 현재 라이트
				// arg2:linkedGroupInfo,					// 현재 triangle 그룹과 
				// arg3:texture정보							// alpha만 갱신된 texture : UpdateTextureOnlyAlpha()함수갱신
				// arg4:current position parameter 정보		// 현재 tessel 지점의 정보(v3Pos,v2SecondUV,v2FirstUV,v3Normal)
				rv = ComputeDirectionalLight_TexelPoint_(light, linfo, CurTexel);
				//if (rv&0xffffff)			// 조명계산결과 완전 차폐가 아닌경우라면 색을 기존색에 더한다.
				if( rv > m_pGlobal->g_shadow_color )
				{
					// ray 상에 intersect 된 visibility 가산연산된 최종 visibility alpha 수치를 모두 더한다.
					BlendColor_RGB(rv, r,g,b);			// color 더함 ---> 
				}
			}
			break;
		case LIGHTTYPE_POINT:
			{
				// point light 지수 : attenuation 공식 적용 
				// arg1:current light
				// arg2:linked Group Infor
				// arg3:current texture info
				// arg4:tessel info(v3Pos,v3Normal,v2SecondUV, v2FirstUV)
				// tessel light color = light.color * light intensity * attenuation * visibility factor
				//		, attenuation = (light.fAttenuation_Far - current distance) / light.fAttenuation_Delta
				//		, light intensity = (- (dot(light v3Normal, tesselpositionnormal) /2 + 0.5) * light.fLightIntensity_ )
				//if (rv&0xffffff)		// 더이상 빛의 가산이 없어도 된다면..
				if( TRUE == ComputePointLight_TexelPoint_(light, linfo, CurTexel, rv) )
				{
					BlendColor_RGB(rv, r,g,b);
				}
			}
			break;
		}

	}
	
	// add ambient color 있으면(0 아니면) 적용 
	if (m_pGlobal->g_ambient_color&0xffffff)
	{
		BlendColor_RGB(m_pGlobal->g_ambient_color, r, g, b);
	}	

#if 2==SHOW_DEBUG_MODE
	sprintf( g_szDebug, "R=%d, G=%d ,B=%d ,A=%d\n", r,g,b,a );

	DEBUGOUTPUT( g_szDebug );
#endif

	return COLOR_RGBA(r,g,b,a);
}


// 실 Ray 충돌되는 Tri들의 Alpha Visibility 갱신
// arg1:light, 
// arg2:CurLinkedPolygon, 
// arg3:현재점 한개에 해당하는 edgeparameter, 
// 해당 light가 node검색을 통해 보유한 visibility triangle set을 통해서
// iResultVisibility -= aa; // 해당 지점의 alpha 값만큼 초기 255 에서 감산연산 으로 최종 visibility 수치 지정후 반환
BOOL CTriangleRasterizer::CheckVisibility_( set<int>& vtris,
					  const Vec3& v3TexelPos,
					  const Vec3& v3LightPos,
					  const Vec3& v3LightDir,
					  float fEpsilon_Offset_IntersectTri )
{
	int nvtris = vtris.size();

	Vec3 v3Distance = v3TexelPos - v3LightPos;

	float fDistance_Square = v3Distance.LengthSquare();

	Vec3 v3IntersectOffset(0.0f, 0.0f, 0.0f);
	Vec3 v3IntersectLength(0.0f, 0.0f, 0.0f);

	set<int>::iterator it;
	it = vtris.begin();	

	for( ; it != vtris.end(); ++it )

		// light가 보유한 모든 triangle index 에 접근 : 
		// 현재 점에 대한 가시성지수 는 라이트가 보유한 모든 triangle들의 라이트의 충돌이난 지점에
		// 텍스쳐 상에 지점의 alpha 지수에 차감된 결과
	{
		// 현재 점 tri 
		int index = (*it);												// Light -> Destination 과의 근사치로 충돌 날것같은 Tri들을 Detail한 충돌 검출
		CRasterTri &tri = m_pGlobal->g_whole_triangle_array[index];

		BOOL rv = tri.LineIntersect_Ex( const_cast<Vec3&>(v3TexelPos), 
										const_cast<Vec3&>(v3LightPos) );

		// 충돌 나는 Tri가 하나라도 있으면 무조건 그림자 드리움
		if (rv)	
		{
			/*if( fDistance_Square <= v3IntersectLength.LengthSquare() )
			{
				continue;
			}*/

			return TRUE;
		}
	}

	// 해당 라이트가 보유한 모든 삼각형을 가져와서 각 지점에 해당하는 texture alpha buffer에 접근
	// 각 수치를 255에서 차감 시켜 나가 visibility 수치를 반환
	return FALSE;
}



// 실 Ray 충돌되는 Tri들의 Alpha Visibility 갱신
// arg1:light, 
// arg2:CurLinkedPolygon, 
// arg3:현재점 한개에 해당하는 edgeparameter, 
// 해당 light가 node검색을 통해 보유한 visibility triangle set을 통해서
// iResultVisibility -= aa; // 해당 지점의 alpha 값만큼 초기 255 에서 감산연산 으로 최종 visibility 수치 지정후 반환
int CTriangleRasterizer::CheckVisibility_ProcessAlpha( set<int>& vtris,
											  Vec3& v3TexelPos,
											  Vec3& v3LightPos,
											  Vec3& v3LightDir,
											  float fLightIntensity,
											  float fEpsilon_Tri_IntersectOffset )
{
	int nvtris = vtris.size();

	BOOL rv = FALSE;

	int iResultVisibility = 255;

	//int iLimitVisibility = 255 / (2*fLightIntensity);
	int iLimitVisibility = 0;

	float fDistance_Square = (v3TexelPos - v3LightPos).LengthSquare();

	set<int>::iterator it;
	it = vtris.begin();	

	Vec3 v3Intersect_Weight(0.0f, 0.0f, 0.0f);
	Vec3 v3Intersect_Length(0.0f, 0.0f, 0.0f);

	for( ; it != vtris.end(); ++it )
	{
		// 현재 점 tri 
		int index = (*it);												// Light -> Destination 과의 근사치로 충돌 날것같은 Tri들을 Detail한 충돌 검출
		CRasterTri &tri = m_pGlobal->g_whole_triangle_array[index];

		//BOOL rv = tri.LineIntersect(v3LightPos, v3TexelPos, fEpsilon_Tri_IntersectOffset, &Intersect );

		BOOL rv = tri.LineIntersect_Pos_Ex( const_cast<Vec3&>(v3TexelPos), 
												const_cast<Vec3&>(v3LightPos), 
												v3Intersect_Weight,
												v3Intersect_Length );

		// 충돌 나는 Tri가 하나라도 있으면 무조건 그림자 드리움
		if (rv) 	
		{
			// 만약 충돌 지점이 라이트와 정점 지점의 길이 보다 더 길다면 계산에 반영 하지 않는다.
			if( fDistance_Square <= v3Intersect_Length.LengthSquare() )
			{
				continue;
			}

			// 만약 Alpha Texture 이면 Alpha 연산을 하여 감산 시켜준다.
			if( true == tri.tinfo->m_alpha_flag )
			{
				// triangle 정점 3개 에 위치 비율
				Vec2 re_uv = tri.UV[0] * v3Intersect_Weight[0] + tri.UV[1] * v3Intersect_Weight[1] + tri.UV[2] * v3Intersect_Weight[2];

				int x = tri.tinfo->_width * re_uv.x;			// 충돌이 난 지점의 텍스쳐 상 x 지점
				int y = tri.tinfo->_height * re_uv.y;			//         "        텍스쳐 상 y 지점

				if (x<0) x=0;if (x>=tri.tinfo->_width) x = tri.tinfo->_width-1;
				if (y<0) y=0;if (y>=tri.tinfo->_height) y = tri.tinfo->_height-1;

				int idx = x + y * tri.tinfo->_width;	// alpha buffer 접근 index
				DWORD aa = tri.tinfo->m_alpha_data[idx];	// 텍스쳐 상에 현재점 alpha 값

				iResultVisibility -= aa; // 해당 지점의 alpha 값만큼 초기 255 에서 감산연산

				if (iResultVisibility<= iLimitVisibility )
				{
					return (iLimitVisibility);
				}
			}
			else
			{
				return (iLimitVisibility);
			}
		}
	}

	return (iResultVisibility);
}

DWORD CTriangleRasterizer::ComputeDirectionalLight_TexelPoint_(CLightComp &light,
										  CLinkedPolygon &linfo,
										  tTexelState *CurTexel )
{
	float lux = 0;
	int			iAlphaVisibility = 255;

	int		iR,iG,iB;
	iR = iG = iB = 0;

	lux = DotProduct_(CurTexel->v3Normal,light.v3LightDir);

	if( 0 < lux ) 
	{
		return m_pGlobal->g_shadow_color;
		//return DEFAULT_SHADOWCOLOR3;
	}

	// 정리사항:ComputeDirectionalLight_TexelPoint_() directional_100, 1000
	//			triangle상의 지점에서 시작을 바로 하게 되면 자기자신의 그림자가 먹히는경우가 있고,
	//			작은값만큼 light 방향으로 나아간 상태값에서 시작을 하게 되면 해당 문제가 해결되지만
	//			만약, 작은값과 평면사이에 어떤 가시성 collision 물체가 존재하게 되면 
	//			제대로 차폐선별이 이루어지지 않는다.
	//Vec3 v3TexelPos_ = CurTexel->v3Pos;// - light.v3LightDir; // - light.v3Dir_Near;
	Vec3 v3TexelPos_ = CurTexel->v3Pos - ( light.v3LightDir * m_pGlobal->g_pLMOption->_fEpsilon_Offset_IntersectTri );
	//Vec3 v3TexelPos_ = CurTexel->v3Pos;
	Vec3 v3LightPos_ = CurTexel->v3Pos - light.v3Dir_Far;	

#if RAYTRACE_

	// 알파 검사
	if( TRUE == light.bComputeAlphaTexture )
	{
		int iAlphaVisibility = CheckVisibility_ProcessAlpha(light.vtris, 
														v3TexelPos_, 
														v3LightPos_, 
														light.v3LightDir, 
														light.fLightIntensity_,
														light.fEpsilon_Offset_IntersectTri );

		
		if (iAlphaVisibility <= 0)
		{
			return m_pGlobal->g_shadow_color;
			//return DEFAULT_SHADOWCOLOR3_1;
		}
	}
	// 알파 검사 않함
	else
	{
		BOOL		bIntersect = FALSE;

		bIntersect = CheckVisibility_( light.vtris, 
			v3TexelPos_, 
			v3LightPos_, 
			light.v3LightDir,
			light.fEpsilon_Offset_IntersectTri );



		if( TRUE == bIntersect )
		{
			return m_pGlobal->g_shadow_color;
			//return DEFAULT_SHADOWCOLOR5;
		}
	}

#else
	iAlphaVisibility = 255;
#endif

	// visibility 수치를 0.0f ~ 1.0f 사이 값으로 치환 -> fAlphaVisibility
	float fAlphaVisibility = float(iAlphaVisibility) / 255.0f;



	// light intensity 지수 / 2.0 - 0.5 ??
#if (FULL_SHADING)
	lux = (lux / 2.0 - 0.5);
#endif

	DWORD co = 0;

	// lux 방향을 제대로 설정(* -1)
	lux = -lux * light.fLightIntensity_;

	if( lux >= 1.0f ) lux = 1.0f;

	// light color * visibility 지수 * 빛 지수 = reality color
	if (lux <= 0)
	{
		return m_pGlobal->g_shadow_color;
		//return DEFAULT_SHADOWCOLOR5_1;
	}
	else
	{
		iR = lux * PickR(light.color) * fAlphaVisibility;
		iG = lux * PickG(light.color) * fAlphaVisibility;
		iB = lux * PickB(light.color) * fAlphaVisibility;
	}

	co = COLOR_RGBA(iR,iG,iB,255);


	return co;

}



// 결과 lum -> lightmap[x,y] 적용 
// : 이미 라이트가 미치는 범위 여부연산은 한 상태로 값이 넘어온다.
// point light R,G,B color 지수 : attenuation 공식 적용
// tessel point color = original color * light intensity * attenuation * visibility factor
//		attenuation : attenuation = (light.fAttenuation_Far - current fDistance_Square) / light.fAttenuation_Delta
// arg1:current light
// arg2:linked Group Infor
// arg3:current texture info
// arg4:tessel info(v3Pos,v3Normal,v2SecondUV, v2FirstUV)
BOOL CTriangleRasterizer::ComputePointLight_TexelPoint_	(CLightComp &light
																			,CLinkedPolygon &linfo
																			,tTexelState *CurTexel
																			,DWORD &dwResultVisibility )
{
	float lux = 0;

	int iAlphaVisibility = 255;

	// light->position
	Vec3 v3LightDirToTexel =  CurTexel->v3Pos - light.position;
	Vec3 dir = v3LightDirToTexel;  dir.SafeNormalize();
	Vec3 v3ReplacementLight;

	int		iR,iG,iB;
	iR = iG = iB = 0;

	float fDistance = 0.0f, fAttenuationNum = 0.0f;

	lux = DotProduct_(CurTexel->v3Normal,dir);

	if( 0 < lux )
	{
		return FALSE;
		//return DEFAULT_SHADOWCOLOR3;
	}

	fDistance = v3LightDirToTexel.Length();

	if (fDistance > light.fAttenuation_Far)
	{
		return FALSE;
		//return DEFAULT_SHADOWCOLOR3_1;
	}
	else if (fDistance < light.fAttenuation_Near)
	{
		fAttenuationNum = 1.0f;
	}
	else
	{
		// 감쇠지수 = (라이트적용범위_y - 거리값) / attenuation_ 전체감쇠거리값 에 대한 비율
		fAttenuationNum = (light.fAttenuation_Far - fDistance)/(light.fAttenuation_Delta);
	}

	// shadow가 정의된 경우만 visibility 수치 계산
#if RAYTRACE_

	// 라이트와 계산된 지점을 미세하게 빛 방향으로 약간 띄워서 계산
	// : 이유는 매우 근소한 차이로 각 Texture 가 Mapping 되는 것과 관련되어 
	//   제대로 라이트가 안먹히는 경우를 방지 하기 위함
	Vec3 v3TexelPos_		= CurTexel->v3Pos - (dir * m_pGlobal->g_pLMOption->_fEpsilon_Offset_IntersectTri);
	//Vec3 v3TexelPos_		= CurTexel->v3Pos;	
	Vec3 v3LightPos_		= light.position;

	if( TRUE == light.bComputeAlphaTexture )
	{
		iAlphaVisibility = CheckVisibility_ProcessAlpha( light.vtris, 
			v3TexelPos_, 
			v3LightPos_, 
			dir, 
			light.fLightIntensity_,
			light.fEpsilon_Offset_IntersectTri );

		if (iAlphaVisibility <= 0)
		{
			return FALSE;
			//return DEFAULT_SHADOWCOLOR3_1;
		}
	}
	else
	{
		BOOL		bIntersect = FALSE;

		bIntersect = CheckVisibility_( light.vtris, 
			v3TexelPos_, 
			v3LightPos_, 
			light.v3LightDir,
			light.fEpsilon_Offset_IntersectTri );



		if( TRUE == bIntersect )
		{
			return FALSE;
			//return DEFAULT_SHADOWCOLOR5;
		}
	}

#else
	iAlphaVisibility = 255;
#endif		

	// 가시성 지수
	float fAlphaVisibility = float(iAlphaVisibility) / 255.0f;

	lux = -lux  * light.fLightIntensity_;

	if (lux>=1.0f) lux = 1.0f;

	// visibility 수치가 0보다 작거나, 감쇠지수가 0보다 작으면 rgb = 0
	if (lux <= 0 || fAttenuationNum <= 0)
	{
		return FALSE;
		//return DEFAULT_SHADOWCOLOR5_1;
	}
	else
	{
		iR = fAttenuationNum * lux * PickR(light.color) * fAlphaVisibility;	// 감쇠지수 * 가시성크기 * color * light itensity + Alpha Visibility
		iG = fAttenuationNum * lux * PickG(light.color) * fAlphaVisibility;
		iB = fAttenuationNum * lux * PickB(light.color) * fAlphaVisibility;

		if(iR<m_pGlobal->g_iShadow_Color_R)
		{
			iR=m_pGlobal->g_iShadow_Color_R;
		}

		if(iG<m_pGlobal->g_iShadow_Color_G)
		{
			iG=m_pGlobal->g_iShadow_Color_G;
		}
		
		if(iB<m_pGlobal->g_iShadow_Color_B)
		{
			iB=m_pGlobal->g_iShadow_Color_B;
		}
	}

	dwResultVisibility = COLOR_RGBA( iR, iG, iB, 255 );

	return TRUE;

}



CTriangleRasterizer::CTriangleRasterizer(void)
{
	m_pGlobal = m_pGlobal->GetThis();
}

CTriangleRasterizer::~CTriangleRasterizer(void)
{
}
