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

	// co�� texture�� ��û������ color�� + ����
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


// light�� �����Ǵ� visible tringle���� gathering �Ѵ�.
// triangle tesselation position �� ��� �� light���� 
// ��� ���� ��带 �˻��ؼ� �ش� tesselation position �� light position ���� �ϳ���
// �����ϴ� ��忡 ���� �ִ� ��� triangle index���� �ش� light�� visible triangle set collect
// �ǹ�����:light�� vertex���̰� intersection ������ �� ���ϴ���?
void CTriangleRasterizer::CollectApproximateTri_RayTrace_DirectionalLight( 
													const CKDTree	*pKDTreeRoot, 
													const Vec3		*arv3PolyPos,
													const UINT		uiRasterTriSeq,
													CLightComp *pCurLight )
{	
	pCurLight->bHasApproximatedCastShadowTri = true;
	{
		// to do : 1 ���� ���� ū �ﰢ����
		// �ɰ��� üũ�ϵ��� �ٲ����.
		int ntpos;

		// ���� triangle�� �� ������ tesselation���� ����
		Vec3 *tpos = TesselateRasterizationPos(arv3PolyPos,&ntpos);

		if( 0 < pCurLight->vtris.size() )
		{
			pCurLight->vtris.clear();		// visible triangle set�� blank
		}
		Vec3 v3Pos, v3Light;
		int i=0,ni = ntpos;

		// Shadow ����ȭ ��� : Texel �ΰ��� �ϳ��� �ǳʶ�
		if( TRUE == m_pGlobal->g_pLMOption->_bUsingShadowOptimization)
		{
			for( i = 0 ; i < ni ; ++i )
			{
				if( 0 != i % 2  )
				{
					// light_1000 �ִ� ���̰��� �������� �� ��ŭ�� �ӽ������� ����Ʈ �������� ���
					// ��� ����� point pCurLight�� ����
					v3Pos = tpos[i];
					v3Light = v3Pos - pCurLight->v3Dir_Far;

					// �����Ǵ� triangle�鸸 �����Ѵ�.
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
				// light_1000 �ִ� ���̰��� �������� �� ��ŭ�� �ӽ������� ����Ʈ �������� ���
				// ��� ����� point pCurLight�� ����
				v3Pos = tpos[i];
				v3Light = v3Pos - pCurLight->v3Dir_Far;

				// �����Ǵ� triangle�鸸 �����Ѵ�.
				LineSegment		LSeg;
				LSeg.CalSegment( v3Light, v3Pos );

				const_cast<CKDTree*>(pKDTreeRoot)->CollectTriangleFromNode_Slab( LSeg,pCurLight->vtris );
				//const_cast<CKDTree*>(pKDTreeRoot)->CollectTriangleFromNode(v3Pos,v3Light,pCurLight->vtris);
				//const_cast<CKDTree*>(pKDTreeRoot)->CollectVisibleNodesChecker( v3Pos, v3Light, pCurLight->vtris);	
				//const_cast<CKDTree*>(pKDTreeRoot)->CollectVisibleNodes_Temp(v3Pos,v3Light,pCurLight->vtris);
			}
		}
	}

	//// �ڱ� �ڽ��� Raster Tri�� ���� ��Ų��.
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

		uiBuf = uiBuf + sprintf( szDebug + uiBuf, " CollectVisibleTriFromNodes �˻��� Triangle ���� : %d", uiCount );

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
	// position �� �������� tesselatePosition �� �����´�?
	// edge�� ���� �亯���� ������ ��ŭ ���簢�� �������� �� position ��ġ��
	// position0 ���� �������� 1���� �迭�� g_pos[] �� ���� �ϴµ�
	// g_pos �� index�� �ؼ� ���� width�� height���� ���� ������ ���� ������ ���� ������ �ݿ��ǰ�,
	// ���� vector ��ġ ������ �� edge�� ���� * current 0.0f ~ 1.0f ��������� ���� ���� 
	// arv3PolyPos[0] + a * ar + b * br;�� ���
	// ntpos = tesselateposition ����
	Vec3 *tpos = TesselateRasterizationPos(arv3PolyPos,&ntpos);	// ntpos : tri�� ��ü tessel ��	// Tesselate Rasterization Pos�� ��ȯ�Ͽ� ���� 

	if( 0 < pCurLight->vtris.size() )
	{
		pCurLight->vtris.clear();
	}

	Vec3 v3Pos, v3Light;

	// triangle�� ���� ��� 1���� tesselation position ���� 
	for( int i = 0 ; i < ntpos ; ++i )
	{
		v3Pos = tpos[i];
		v3Light = pCurLight->position;
		
		// ��庰 ���� triangle�鸸 ����
		LineSegment		LSeg;
		LSeg.CalSegment( v3Light, v3Pos );

		const_cast<CKDTree*>(pKDTreeRoot)->CollectTriangleFromNode_Slab( LSeg, pCurLight->vtris);
		//const_cast<CKDTree*>(pKDTreeRoot)->CollectTriangleFromNode( v3Pos, v3Light, pCurLight->vtris);
		//const_cast<CKDTree*>(pKDTreeRoot)->CollectVisibleNodesChecker( v3Pos, v3Light, pCurLight->vtris);	
		//const_cast<CKDTree*>(pKDTreeRoot)->CollectVisibleNodes_Temp( v3Pos, v3Light, pCurLight->vtris);
	}

	// �ڱ� �ڽ��� Raster Tri�� ���� ��Ų��.
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

		uiBuf = uiBuf + sprintf( szDebug + uiBuf, " CollectVisibleTriFromNodes �˻��� Triangle ���� : %d", uiCount );

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


// ���� �ﰢ�� pos1,2,3 �� ������ ���� ��� Tessel ������ ����
// arg1 : triangle �� world position 
Vec3* CTriangleRasterizer::TesselateRasterizationPos(const Vec3 *v3Pos,int *ntpos)
{
	// triangle �� 1�� edge�� ����
	float delta = (v3Pos[0]-v3Pos[1]).Length();
	float delta_max;

	delta_max = delta;

	// triangle �� 2�� edge�� ����
	delta = (v3Pos[1]-v3Pos[2]).Length();

	if (delta > delta_max)
		delta_max = delta;

	// triangle �� 3�� edge�� ����
	delta = (v3Pos[2]-v3Pos[0]).Length();

	// ���� �� ������ edge�� deltamax�� ����
	if (delta > delta_max)
		delta_max = delta;

	//int ntess = 5 * delta_max / (DEFAULT_KDTREERES);
	int ntess = delta_max;

	*ntpos = 0;

	// edge 1, 2 directional vector
	Vec3 vEdgeA = v3Pos[1] - v3Pos[0];		// 
	Vec3 vEdgeB = v3Pos[2] - v3Pos[1];

	// �� tesselation ������ 0.0 ~ 1.0 ���� ���� 
	int i=0,ni = ntess + 1, iTemp = 0;		// Edge���� ���� 
	UINT	uiNi = ni * ni;

	if( uiNi > MAX_A_TRI_TESSEL_CNT )
	{
		char		szCriticalMsg[256];
		sprintf( szCriticalMsg, "������ Texel Count�� %d�̰�, �ִ� texel ���� Count %d�� �ִ� Texel ���� Count�� �÷��� �մϴ�. ������ ���� ���� �մϴ�.", 
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
			int j = 0 ,nj = i+1;			// Edge���̷� ���ٵǴ� ���� �� + 1��ŭ
			for( j = 0 ; j < nj ; ++j )
			{
				float ar,br;
				ar = (float(i)/float(ni-1));		// 0.0f ~ 1.0f ����
				if (nj ==1)							 
					br = 0;
				else
					br = (float(j)/float(nj-1));	// 0.0f ~ 1.0f ����

				// ���� ���� ������ ��ġ ������ ���� 
				// 3d ��ǥ ���̸�ŭ �� ������ ���� ��ġ������ ��ȯ
				// triangle�� 0�� position + (edge 1�� * (current 0.0f ~ 1.0f ����))
				//						   + (edge 2�� * (current 0.0f ~ 1.0f ����))
				m_v3TesselArr[(*ntpos)++] = v3Pos[0] + vEdgeA * ar + vEdgeB * br;	
				//(*ntpos)++;		// ���� index
			}
		}
	}
	else
	{
		m_v3TesselArr[(*ntpos)++] = v3Pos[0];
	}
	
	return &m_v3TesselArr[0];
}

// -. first UV�� Second UV :1. second UV ��ǥ�� linked Group min, max boundary �� ��ġ�� 
//							���� triangle�� �� ������ ����� ��ġ ��
//							2. first UV �� ��� parsing�� ���� �о�� texture�� UV ��ǥ
// triangle ������ tessel color�� light color ������ ���� CurLinkedPolygon.m_tex->m_tex_data[i].color ����

// 1.rasteraization 2.calculate lumel 
// 3.calculate visiblity color from gathering triangle 4.draw Texel Color
void CTriangleRasterizer::RenderRasterizeTriangle( vector<CLightComp>& vecLights,		// Light Component Container
													CLinkedPolygon &linfo,		// linked Group Info
													Vec2 *v2SecondUV,			// triangle�� ������ second UV ��ǥ (linked Group min max 1-0��ǥ)	
													Vec3 *v3Pos,				// triangle�� ������ world position
													Vec3 *v3Normal,				// triangle�� ������ v3Normal
													CAlphaTexture &CurAlphaTexture,	// alpha channel�� CurAlphaTexture.m_alpha_data[idx] ����
													Vec2 *v2FirstUV )		// parsing�� ������ UV ��ǥ
{
	const int big_tri = 100*100;
	bool verbose = false;
	char old_text[1024],buf[1024];
	int line_counter = 0;
	const int freq = 10;

	// triangle �ؽ��� ������ ������ 10000 ���� ũ�� error
	if (linfo.m_tex._width * linfo.m_tex._height > big_tri)
	{
		verbose = true;
		GetWindowText(m_pGlobal->g_hwnd,old_text,1024);
	}

	verbose = false;
	

	CTextureSegment &rtex = linfo.m_tex;
	float tex_height = float(linfo.m_tex._height);		// linkinfo Group �� texture height


	const float fTOOSMALL_EDGELENGTH = 1.0;

	DWORD tick = timeGetTime();


	int pre_index[3] = {0,1,2};
	bool do_A_phase_flag = true;
	bool do_B_phase_flag = true;

	// 1. �ϴ� y ���� ũ�� ������ ��Ʈ : y ���� ��������  0 < 1 < 2 ����
	// 2. UV ���̰��� �ʹ� ������� �÷��� = false
	//	  Edge A Validate flag, Edge B Validate flag
	if (v2SecondUV[pre_index[0]].y > v2SecondUV[pre_index[1]].y) Swapi(pre_index[0],pre_index[1]);
	if (v2SecondUV[pre_index[0]].y > v2SecondUV[pre_index[2]].y) Swapi(pre_index[0],pre_index[2]);
	if (v2SecondUV[pre_index[1]].y > v2SecondUV[pre_index[2]].y) Swapi(pre_index[1],pre_index[2]);

	// 2. UV ���̰��� �ʹ� ������� ( ���� �� 1.0f ) 0-1�� �Ǵ� 2-1�� ���̰� �ʹ� ������ ������� �ʴ´�.
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

	// y �� ���̷� ascending sorting ���� index ����
	int idx_0 = pre_index[0];	
	int idx_1 = pre_index[1];	
	int idx_2 = pre_index[2];	

	// ������κ��� ù��° ����  �׸����� �ִ� ��� : ���� a-b
	if (do_A_phase_flag)
	{
		poll.v2FirstUV		= v2FirstUV[idx_0];				// first UV a   : �ڽ��� ������ Texture�� UV
		poll.v2SecondUV		= v2SecondUV[idx_0];			// second UV a  : Light ���Ǿ� ������ UV
		poll.v3Pos			= v3Pos[idx_0];					// position a	: Triangle 0�� Vertex ��ġ
		poll.v3Normal		= v3Normal[idx_0];				// position v3Normal a

		// cursor ��� : ��ġ���� y�������� ����			// index a-b, 
		scur.start_v		= v2SecondUV[idx_0].y * tex_height;	// 
		scur.end_v			= v2SecondUV[idx_1].y * tex_height;	// UV ��ġ
		scur.v_delta		= scur.end_v - scur.start_v;			// b -> a �Ÿ����� 
		scur.v				= scur.start_v;							// UV ���۰�

														// index a-c
		ecur.start_v		= v2SecondUV[idx_0].y * tex_height;	// �ؽ��� �� index a �� ���� y ��
		ecur.end_v			= v2SecondUV[idx_2].y * tex_height;	//		"			 c			"
		ecur.v_delta		= ecur.end_v - ecur.start_v;		// c -> a �� �Ÿ� ���� ����
		ecur.v				= ecur.start_v;							// a �� �ؽ��Ļ� ���� y ���۰�

		// delta ���									// index a-b
		sdelta.v3Pos		= v3Pos[idx_1] - v3Pos[idx_0];			// world position ���� b�� a�� 0���� a�� ���� ��� ��ġ��
		sdelta.v3Normal		= v3Normal[idx_1] - v3Normal[idx_0];	// v3Normal b - v3Normal a = delta v3Normal	: ��� v3Normal ��
		sdelta.v2SecondUV	= v2SecondUV[idx_1] - v2SecondUV[idx_0];	// second b - second a = delta sconduv
		sdelta.v2FirstUV	= v2FirstUV[idx_1] - v2FirstUV[idx_0];	// parsing UV b - parsing UV a	: UV Texture�� b�� a�� 0�������� ������� y��

														// index a-c
		edelta.v3Pos		= v3Pos[idx_2] - v3Pos[idx_0];			// worldposition�� c�� a�� 0���� a �� ���� ��� ��ġ��
		edelta.v3Normal		= v3Normal[idx_2] - v3Normal[idx_0];	// c�� a�� ���� v3Normal ���� edelta v3Normal : ��� v3Normal ��
		edelta.v2SecondUV	= v2SecondUV[idx_2] - v2SecondUV[idx_0];	// second UV�� a�� 0���� a�� ���� ��� ��ġ��
		edelta.v2FirstUV	= v2FirstUV[idx_2] - v2FirstUV[idx_0];	// parsing UV c - a



		int loop_counter = 0;


		// texture_height ���� // ���η� �׸�
		while(true)
		{
			loop_counter++;

			// ���� triangle�� linked group �� ��� ��ġ������ ���� �ؽ��Ļ� a, c �� �Ÿ��� 0 �ΰ�� 
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

			// edge �� ��ġ �� ������
			float sr = (scur.v - scur.start_v)		/	scur.v_delta;	// a-b�� : current v�� texture�� ��ġ�� ������	
			float er = (ecur.v - ecur.start_v)		/	ecur.v_delta;	// a-c�� : 				"

			// first UV a + (a->b)v2FirstUV ��� ���̰� * current v�� ��� ��ġ��
			// texture �� current linear �� UV ��ġ��, position, v3Normal ��

			// 1. �ʱ� ��a->b�� ���� ������ ��� ����, v3Normal, first UV, second UV a-b��
			StartTexel.v2FirstUV	= poll.v2FirstUV	+ (sdelta.v2FirstUV) * sr;	
			StartTexel.v2SecondUV	= poll.v2SecondUV	+ (sdelta.v2SecondUV) * sr;
			StartTexel.v3Pos		= poll.v3Pos		+ (sdelta.v3Pos) * sr;
			StartTexel.v3Normal		= poll.v3Normal		+ (sdelta.v3Normal) * sr;

			// 2. �ʱ� ��a->c�� ���� ������ ��� ����, v3Normal, first UV, second UV a-c��
			EndTexel.v2FirstUV		= poll.v2FirstUV	+ (edelta.v2FirstUV) * er;		// �ʱ� ���� �� edge ���� �� ���� ��������
			EndTexel.v2SecondUV		= poll.v2SecondUV	+ (edelta.v2SecondUV) * er;
			EndTexel.v3Pos			= poll.v3Pos		+ (edelta.v3Pos) * er;
			EndTexel.v3Normal		= poll.v3Normal		+ (edelta.v3Normal) * er;

			// ���η� �׷��� �� rasterilzetion line ���� �׷���
			// arg1:linkGroupInfo, 
			// arg2:CurAlphaTexture:alpha�� ���ŵ� texture buffer
			// arg3:StartTexel:a->b�� ���� ����, 
			// arg4:EndTexel:a->c�� ���� ����, ������ ���� ���� : ����0 �̸� false
			// tessel triangle�� ���η� �׷������� 
			// CurLinkedPolygon.m_tex->m_tex_data[i].color ����
			RenderTriangleTexel_HorizontalLine(vecLights,
												linfo,
												CurAlphaTexture,
												&StartTexel,
												&EndTexel,
												false);

			// ����� ������ �׸�����
			scur.v += (1.0f);	
			ecur.v += (1.0f);
		}
	}



	// ������κ��� �ι�° ����  �׸����� �ִ� ��� : ���� b-c
	if (do_B_phase_flag)
	{
		polla.v2FirstUV = v2FirstUV[idx_1];		
		polla.v2SecondUV = v2SecondUV[idx_1];
		polla.v3Pos = v3Pos[idx_1];
		polla.v3Normal = v3Normal[idx_1];


		// cursor ���
		scur.start_v = v2SecondUV[idx_1].y * tex_height;
		scur.end_v = v2SecondUV[idx_2].y * tex_height;
		scur.v_delta = scur.end_v - scur.start_v;
		scur.v = scur.start_v;

		// delta ���
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

			// delta ���
			edelta.v3Pos = v3Pos[idx_2] - v3Pos[idx_0];
			edelta.v3Normal = v3Normal[idx_2] - v3Normal[idx_0];
			edelta.v2SecondUV = v2SecondUV[idx_2] - v2SecondUV[idx_0];
			edelta.v2FirstUV = v2FirstUV[idx_2] - v2FirstUV[idx_0];

		}


		// cursor ���
		ecur.start_v = v2SecondUV[idx_0].y * tex_height;
		ecur.end_v = v2SecondUV[idx_2].y * tex_height;
		ecur.v_delta = ecur.end_v - ecur.start_v;
		ecur.v = scur.start_v;




		int loop_counter = 0;

		// �� Edge ���� * 2 ��ŭ Looping �ȴ�.
		while(true)
		{
			loop_counter++;

			if (ecur.v_delta == 0)
			{
				break;
			}

			//if (scur.v > scur.end_v + DEFAULT_OVEREDGE) // �� ���� ����?
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

			// �� raster ȣ��

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

// tessel triangle�� ���η� �׷������� CurLinkedPolygon.m_tex->m_tex_data[i].color ����
// arg1:linkGroupInfo, 
// arg2:CurAlphaTexture:alpha�� ���ŵ� texture buffer
// arg3:StartTexel:a->b�� ���� ����, 
// arg4:EndTexel:a->c�� ���� ����, 
// arg5:tesseltriangle height ���� 0�� �ƴϸ� true : 0���� mix�Ұ� ����.
// 1. ������ ���� ���� : ����0 �̸� false
//					sep�� EndTexel:triangle�� a->b �Ǵ� a->c�� ���� ��� ������ ���Ǿ��� 
//					position, v3Normal, v2FirstUV, v2SecondUV ����
// 2. tessel ���� light type ��, anti alias �� �������� light color ���� �����ϰ� 
// 3. CurLinkedPolygon.m_tex->m_tex_data[i].color �� light color ����

// linkinfo.tex->m_tex_data[idx] (color) -> ���� ����
void CTriangleRasterizer::RenderTriangleTexel_HorizontalLine( vector<CLightComp>& vecLights,
																CLinkedPolygon &linfo,
																CAlphaTexture &otex,
																tTexelState *StartTexel,
																tTexelState *EndTexel,
																bool mix_flag)
{
	// �̸� rgba(255,255,255,0)���� allocate �� TrgTexInfo
	CTextureSegment &rtex = linfo.m_tex;	

	tTexelState CurPointTexel, EdgeDeltaTexel;

	// a->b v2SecondUV.x > a->c v2SecondUV.x  swap : tessel�� ���ʿ��� ���������� �׷������� �Ѵ�.
	if (StartTexel->v2SecondUV.x > EndTexel->v2SecondUV.x)
	{
		tTexelState *a = StartTexel;StartTexel = EndTexel;EndTexel = a;
	}

	// current s->v2SecondUV.x(0.0-1.0)�� ������ width���� ����
	//		rtex._width : ������ width���� linkedGroup�� min max �� ������ �࿡ ���̰��� ���� �����
	//		v2SecondUV.d : linkedGroup�� min max�� ���� ������ 0.0-1.0 ������ ��ġ��
	float start_secondU	= StartTexel->v2SecondUV.x * rtex._width;		// current s->v2SecondUV.x�� tessel width���� ����?
	float end_secondU	= EndTexel->v2SecondUV.x * rtex._width;		// current e->v2SecondUV.x�� 		"
	float delta_secondu	= fabs(end_secondU - start_secondU);				// current tessel ������ edge�� �������� �����ʿ��� ū������ edge
	float cursor_u		= start_secondU;									// tessel render ������

	const float TOOSMALL_HORIZONTALLEN = 0.5;

	if (delta_secondu < TOOSMALL_HORIZONTALLEN)						// �׸����� �������̱��� �� 0.5 �̻��� �Ǿ�� �Ѵ�.
		return;


	if (cursor_u < 0)
		cursor_u = 0;

	// �� ��Ұ� start-end �� ���̹��� EdgeDeltaTexel ���� 
	EdgeDeltaTexel.v2SecondUV		= EndTexel->v2SecondUV - StartTexel->v2SecondUV;				// second UV
	EdgeDeltaTexel.v2FirstUV		= EndTexel->v2FirstUV	- StartTexel->v2FirstUV;				// first UV
	EdgeDeltaTexel.v3Pos			= EndTexel->v3Pos		- StartTexel->v3Pos;				// position
	EdgeDeltaTexel.v3Normal			= EndTexel->v3Normal	- StartTexel->v3Normal;	// v3Normal

	// anti_factor ��ŭ triangle�� �� ������ ������ ���� �Լ�ȣ��Ǹ�, ���� 1�� ���� 
	//DWORD texel[ANTI_FACTOR];						
	DWORD co;

	int loop_counter = 0;

	// ������ �� �� �ϳ��� �׷�����
	while(1)
	{
		loop_counter++;

		// ���� cursor�� �ؼ� width �� ���� validate
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

		rr = (cursor_u - start_secondU) / delta_secondu;			// tessel ���� �� ���� ���� ���� ����

		// ��Һ� ���� ������ ���� ������ ����
		CurPointTexel.v2FirstUV 	= StartTexel->v2FirstUV		+ EdgeDeltaTexel.v2FirstUV	* rr;			// v2FirstUV 
		CurPointTexel.v2SecondUV	= StartTexel->v2SecondUV	+ EdgeDeltaTexel.v2SecondUV	* rr;	// v2SecondUV
		CurPointTexel.v3Pos 		= StartTexel->v3Pos			+ EdgeDeltaTexel.v3Pos		* rr;				// position 
		CurPointTexel.v3Normal 		= StartTexel->v3Normal		+ EdgeDeltaTexel.v3Normal	* rr;			// v3Normal
		CurPointTexel.v3Normal.SafeNormalize();

		// arg1:linkedGroupInfo,
		// arg2:CurAlphaTexture:alpha�� ���ŵ� texture buffer
		// arg3:���� ������ �������
		// texel ������ light type�� ���� color ����
		// directional light color = tessel color + (light.color * visibility factor * light intensity)
		// point light color = tessel color + (light.color * visibility factor * light intensity)
		//texel[i] = ComputeLight_Render_TexelPoint(vecLights, linfo, &CurPointTexel);
		co = ComputeLight_Render_TexelPoint(vecLights, linfo, &CurPointTexel);
		cursor_u += (1.0f);

		// tessel color�� light color�� ����
		//		1. linkedGroupInfo�� ������ texture�� v2SecondUV�� ���� ���� color 
		//		2. ���� light color
		// texel �� ���� ���� + 0.5
		// CurLinkedPolygon.m_tex->m_tex_data[i].color �� light color ����
		// linkinfo.tex->m_tex_data[idx] = co;
		if (mix_flag)
			DrawPoint_Blend(&linfo.m_tex, CurPointTexel.v2SecondUV.x, CurPointTexel.v2SecondUV.y, co);
		else
			DrawPoint(&linfo.m_tex, CurPointTexel.v2SecondUV.x, CurPointTexel.v2SecondUV.y, co);

	}

}

// triangle tesselation position ���� � ������ �׸�
// arg1:linkedGroupInfo,
// arg2:CurAlphaTexture:alpha�� ���ŵ� texture buffer
// arg3:���� ������ �������
// light�� ������ tessel �� ���� ������ ������ 
//		1. shodow texture���� ������ visibility �Ӽ���
//		2. light �� tessel position ���� 
// return value : tessel point rgb * ���� light color rgb
//	
//		. directional light color = tessel color + (light.color * visibility factor * light intensity)
//		. point light color = tessel color + (light.color * visibility factor * light intensity)
//	. light color �� ambient color ����
DWORD CTriangleRasterizer::ComputeLight_Render_TexelPoint( vector<CLightComp>& vecLights,
															CLinkedPolygon &linfo,
															tTexelState *CurTexel )
{

	int r,g,b,a;

	r=g=b=0;
	a = 255;
	
	// �� ����Ʈ ���� �����Ͽ� ������ �޴� ���� COlor�� ����س���.
	// accumulate lights
	//for( int i = 0; i < (int)m_pGlobal->g_light_array.GetSize(); ++i )
	for( int i = 0; i < (int)vecLights.size(); ++i )
	{
		// ���� ����Ʈ�� ��ȿ�� ���� ���� 
		//CLightComp &light = m_pGlobal->g_light_array.Get_(i);
		//CLightComp &light = m_pGlobal->g_light_array[i];
		CLightComp &light = vecLights[i];

		if (light.enabled == false || light.bDoesAffectionLighting == FALSE)
			continue;

		DWORD rv = 0;

		// type directional, point�� ó��
		switch(light.eLightType)
		{
		case LIGHTTYPE_DIRECTIONAL:
			{
				// directional light 
				// ���� tessel position ������ ���� light ���� = visibility + lux(p.v3Normal dot l.dir)
				// 1. light.color 
				// 2. visibility factor :  ����Ʈ�� ������ ��� ���ü� triangle�鿡 �����Ͽ� 
				//							directional light�� �浹�� �� ������ ������
				//							texture �� �ش� ������ alpha ���� visiblity 255 �������� ���� ���� 
				//							visibility ��ġ�� ����, �� shadow factor
				// 3. light intensity :  (- (dot(light v3Normal, tesselpositionnormal) /2 + 0.5) * light.fLightIntensity_ )
				// light color = light.color * visibility factor * light intensity
				// arg1:current light,						// ���� ����Ʈ
				// arg2:linkedGroupInfo,					// ���� triangle �׷�� 
				// arg3:texture����							// alpha�� ���ŵ� texture : UpdateTextureOnlyAlpha()�Լ�����
				// arg4:current position parameter ����		// ���� tessel ������ ����(v3Pos,v2SecondUV,v2FirstUV,v3Normal)
				rv = ComputeDirectionalLight_TexelPoint_(light, linfo, CurTexel);
				//if (rv&0xffffff)			// �������� ���� ���� �ƴѰ���� ���� �������� ���Ѵ�.
				if( rv > m_pGlobal->g_shadow_color )
				{
					// ray �� intersect �� visibility ���꿬��� ���� visibility alpha ��ġ�� ��� ���Ѵ�.
					BlendColor_RGB(rv, r,g,b);			// color ���� ---> 
				}
			}
			break;
		case LIGHTTYPE_POINT:
			{
				// point light ���� : attenuation ���� ���� 
				// arg1:current light
				// arg2:linked Group Infor
				// arg3:current texture info
				// arg4:tessel info(v3Pos,v3Normal,v2SecondUV, v2FirstUV)
				// tessel light color = light.color * light intensity * attenuation * visibility factor
				//		, attenuation = (light.fAttenuation_Far - current distance) / light.fAttenuation_Delta
				//		, light intensity = (- (dot(light v3Normal, tesselpositionnormal) /2 + 0.5) * light.fLightIntensity_ )
				//if (rv&0xffffff)		// ���̻� ���� ������ ��� �ȴٸ�..
				if( TRUE == ComputePointLight_TexelPoint_(light, linfo, CurTexel, rv) )
				{
					BlendColor_RGB(rv, r,g,b);
				}
			}
			break;
		}

	}
	
	// add ambient color ������(0 �ƴϸ�) ���� 
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


// �� Ray �浹�Ǵ� Tri���� Alpha Visibility ����
// arg1:light, 
// arg2:CurLinkedPolygon, 
// arg3:������ �Ѱ��� �ش��ϴ� edgeparameter, 
// �ش� light�� node�˻��� ���� ������ visibility triangle set�� ���ؼ�
// iResultVisibility -= aa; // �ش� ������ alpha ����ŭ �ʱ� 255 ���� ���꿬�� ���� ���� visibility ��ġ ������ ��ȯ
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

		// light�� ������ ��� triangle index �� ���� : 
		// ���� ���� ���� ���ü����� �� ����Ʈ�� ������ ��� triangle���� ����Ʈ�� �浹�̳� ������
		// �ؽ��� �� ������ alpha ������ ������ ���
	{
		// ���� �� tri 
		int index = (*it);												// Light -> Destination ���� �ٻ�ġ�� �浹 ���Ͱ��� Tri���� Detail�� �浹 ����
		CRasterTri &tri = m_pGlobal->g_whole_triangle_array[index];

		BOOL rv = tri.LineIntersect_Ex( const_cast<Vec3&>(v3TexelPos), 
										const_cast<Vec3&>(v3LightPos) );

		// �浹 ���� Tri�� �ϳ��� ������ ������ �׸��� �帮��
		if (rv)	
		{
			/*if( fDistance_Square <= v3IntersectLength.LengthSquare() )
			{
				continue;
			}*/

			return TRUE;
		}
	}

	// �ش� ����Ʈ�� ������ ��� �ﰢ���� �����ͼ� �� ������ �ش��ϴ� texture alpha buffer�� ����
	// �� ��ġ�� 255���� ���� ���� ���� visibility ��ġ�� ��ȯ
	return FALSE;
}



// �� Ray �浹�Ǵ� Tri���� Alpha Visibility ����
// arg1:light, 
// arg2:CurLinkedPolygon, 
// arg3:������ �Ѱ��� �ش��ϴ� edgeparameter, 
// �ش� light�� node�˻��� ���� ������ visibility triangle set�� ���ؼ�
// iResultVisibility -= aa; // �ش� ������ alpha ����ŭ �ʱ� 255 ���� ���꿬�� ���� ���� visibility ��ġ ������ ��ȯ
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
		// ���� �� tri 
		int index = (*it);												// Light -> Destination ���� �ٻ�ġ�� �浹 ���Ͱ��� Tri���� Detail�� �浹 ����
		CRasterTri &tri = m_pGlobal->g_whole_triangle_array[index];

		//BOOL rv = tri.LineIntersect(v3LightPos, v3TexelPos, fEpsilon_Tri_IntersectOffset, &Intersect );

		BOOL rv = tri.LineIntersect_Pos_Ex( const_cast<Vec3&>(v3TexelPos), 
												const_cast<Vec3&>(v3LightPos), 
												v3Intersect_Weight,
												v3Intersect_Length );

		// �浹 ���� Tri�� �ϳ��� ������ ������ �׸��� �帮��
		if (rv) 	
		{
			// ���� �浹 ������ ����Ʈ�� ���� ������ ���� ���� �� ��ٸ� ��꿡 �ݿ� ���� �ʴ´�.
			if( fDistance_Square <= v3Intersect_Length.LengthSquare() )
			{
				continue;
			}

			// ���� Alpha Texture �̸� Alpha ������ �Ͽ� ���� �����ش�.
			if( true == tri.tinfo->m_alpha_flag )
			{
				// triangle ���� 3�� �� ��ġ ����
				Vec2 re_uv = tri.UV[0] * v3Intersect_Weight[0] + tri.UV[1] * v3Intersect_Weight[1] + tri.UV[2] * v3Intersect_Weight[2];

				int x = tri.tinfo->_width * re_uv.x;			// �浹�� �� ������ �ؽ��� �� x ����
				int y = tri.tinfo->_height * re_uv.y;			//         "        �ؽ��� �� y ����

				if (x<0) x=0;if (x>=tri.tinfo->_width) x = tri.tinfo->_width-1;
				if (y<0) y=0;if (y>=tri.tinfo->_height) y = tri.tinfo->_height-1;

				int idx = x + y * tri.tinfo->_width;	// alpha buffer ���� index
				DWORD aa = tri.tinfo->m_alpha_data[idx];	// �ؽ��� �� ������ alpha ��

				iResultVisibility -= aa; // �ش� ������ alpha ����ŭ �ʱ� 255 ���� ���꿬��

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

	// ��������:ComputeDirectionalLight_TexelPoint_() directional_100, 1000
	//			triangle���� �������� ������ �ٷ� �ϰ� �Ǹ� �ڱ��ڽ��� �׸��ڰ� �����°�찡 �ְ�,
	//			��������ŭ light �������� ���ư� ���°����� ������ �ϰ� �Ǹ� �ش� ������ �ذ������
	//			����, �������� �����̿� � ���ü� collision ��ü�� �����ϰ� �Ǹ� 
	//			����� ���󼱺��� �̷������ �ʴ´�.
	//Vec3 v3TexelPos_ = CurTexel->v3Pos;// - light.v3LightDir; // - light.v3Dir_Near;
	Vec3 v3TexelPos_ = CurTexel->v3Pos - ( light.v3LightDir * m_pGlobal->g_pLMOption->_fEpsilon_Offset_IntersectTri );
	//Vec3 v3TexelPos_ = CurTexel->v3Pos;
	Vec3 v3LightPos_ = CurTexel->v3Pos - light.v3Dir_Far;	

#if RAYTRACE_

	// ���� �˻�
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
	// ���� �˻� ����
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

	// visibility ��ġ�� 0.0f ~ 1.0f ���� ������ ġȯ -> fAlphaVisibility
	float fAlphaVisibility = float(iAlphaVisibility) / 255.0f;



	// light intensity ���� / 2.0 - 0.5 ??
#if (FULL_SHADING)
	lux = (lux / 2.0 - 0.5);
#endif

	DWORD co = 0;

	// lux ������ ����� ����(* -1)
	lux = -lux * light.fLightIntensity_;

	if( lux >= 1.0f ) lux = 1.0f;

	// light color * visibility ���� * �� ���� = reality color
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



// ��� lum -> lightmap[x,y] ���� 
// : �̹� ����Ʈ�� ��ġ�� ���� ���ο����� �� ���·� ���� �Ѿ�´�.
// point light R,G,B color ���� : attenuation ���� ����
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
		// �������� = (����Ʈ�������_y - �Ÿ���) / attenuation_ ��ü����Ÿ��� �� ���� ����
		fAttenuationNum = (light.fAttenuation_Far - fDistance)/(light.fAttenuation_Delta);
	}

	// shadow�� ���ǵ� ��츸 visibility ��ġ ���
#if RAYTRACE_

	// ����Ʈ�� ���� ������ �̼��ϰ� �� �������� �ణ ����� ���
	// : ������ �ſ� �ټ��� ���̷� �� Texture �� Mapping �Ǵ� �Ͱ� ���õǾ� 
	//   ����� ����Ʈ�� �ȸ����� ��츦 ���� �ϱ� ����
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

	// ���ü� ����
	float fAlphaVisibility = float(iAlphaVisibility) / 255.0f;

	lux = -lux  * light.fLightIntensity_;

	if (lux>=1.0f) lux = 1.0f;

	// visibility ��ġ�� 0���� �۰ų�, ���������� 0���� ������ rgb = 0
	if (lux <= 0 || fAttenuationNum <= 0)
	{
		return FALSE;
		//return DEFAULT_SHADOWCOLOR5_1;
	}
	else
	{
		iR = fAttenuationNum * lux * PickR(light.color) * fAlphaVisibility;	// �������� * ���ü�ũ�� * color * light itensity + Alpha Visibility
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
