#include ".\meshconverter.h"

CMeshConverter::CMeshConverter(void)
{
}

CMeshConverter::~CMeshConverter(void)
{
}

// �켱 �ӽ������� �Ѱ��� Parent �޽��� ���� .
void	CMeshConverter::ConvertToLMMeshes_WzMapTerrain( _CParserTerrainMesh& _TerrainMesh,
														Array_<CParentLMMesh*>&	ArParentMeshes, 
														Array_<CLMMesh_*>&		ArLMMeshes,
														vector<CRasterTri>&		ArWholeTriangles, 
														UINT&					uiCntRealityOutputLMObject,
														UINT&					uiGlobalyLMCnt,
														CLMOption&				LMOption )
{
	UINT		uiLMMeshesCnt = 0, 
			iCurParentMeshIdx = 0,
			uiTriesCnt_ = 0,
			uiIndexVertex = 0,
			uiCurLMSeq = 0,
			uiCurRealityLMIdx = 0;

	DWORD		dwParentExID;

	int			iTextureIdx;
	int			iRasterTriCount	= ArWholeTriangles.size();

	Vec3		v3Pos, v3Normal;
	Vec2		v2UV;

	// ���� Terrain Mesh�� Alpha Texture Shadowing ������ �ȵǾ��⿡ Alpha Texture ���� Flag�� FALSE ����.
	// ���� ���� Terrain Mesh�� Alpha Texture Shadowing ������ �Ϸ���, �� Terrain�� ���� LMMEsh ���� Texture HANDLE ������ �ϸ� �ȴ�.
	//BOOL		bComputeAlphaTexture = LMOption._bComputeAlpha;
	BOOL		bComputeAlphaTexture = FALSE;		

	(uiCntRealityOutputLMObject)++; // ���� Output �� LightMap Object �� �� : WzMap�� ��� Terrain��ü�� �ϳ��� Object �̹Ƿ� �ѹ��� ����

	
	CParentLMMesh *pCurParentLMMesh = new CParentLMMesh();
	ArParentMeshes.Set__( iCurParentMeshIdx, pCurParentLMMesh );
	CParentLMMesh&	CurParentLMMesh = *pCurParentLMMesh;


	CurParentLMMesh.Initialize( iCurParentMeshIdx, 
								WZPARSETERRAIN_IDX,
								WZPARSETERRAIN_IDX,
								TRUE, 
								LMOption._bComputeShadow, 
								FALSE );

	dwParentExID = CurParentLMMesh.GetExID();

	uiLMMeshesCnt = _TerrainMesh.m_dwNumTextures;

	ArLMMeshes.NewAllocateBuffer( uiLMMeshesCnt );

	for( UINT iCurLMCnt = 0; iCurLMCnt < uiLMMeshesCnt; ++iCurLMCnt )
	{
		uiTriesCnt_ = _TerrainMesh.m_arCntFacesPerTexture.Get_( iCurLMCnt );

		// ���� Parser Mesh �� Texture �� �����ϴ� Tri�� ���� 0 �ΰ�� LMMesh �� ���� �� �ʿ䰡 ����.
		if( 1 > uiTriesCnt_ )
		{
			continue;
		}

		uiCurLMSeq = (uiGlobalyLMCnt)++;

		//iTextureIdx = _TerrainMesh.m_arListTextureIdx.Get_(uiCurLMSeq);
		iTextureIdx = _TerrainMesh.m_arListTextureIdx.Get_(iCurLMCnt);	

		// iTextureIdx �� �������� LMMeshSeq�� �������� Array �ʿ�
		_TerrainMesh.m_arConvertToLMSeqFromParseTextureIdx.Get_( iTextureIdx ) = uiCurLMSeq;

		CLMMesh_*	pCurLMMesh = new CLMMesh_();

		ArLMMeshes.Set__( uiCurLMSeq, pCurLMMesh );

		CLMMesh_&	CurLMMesh = *pCurLMMesh;

		CurParentLMMesh.InsertSubMeshIdx( uiCurLMSeq );

		CurLMMesh.Initialize(	uiCurLMSeq,					// Seq ID
								iTextureIdx,				// Ex ID
								dwParentExID,				// Parent Ex ID
								uiTriesCnt_,				// Tries Number
								uiTriesCnt_ * 3,			// Vertices Number
								0,							// World Matrix44
								TRUE,						// Cast Shadow
								LMOption._bComputeShadow,	// ReceiveShadow
								bComputeAlphaTexture,		// BOOL ComputeAlpha
								TRUE,						// Already Compute World
								WZPARSETERRAIN_IDX,			// ParentSeq ID
								0 );
	}

	uiTriesCnt_ = _TerrainMesh.m_dwNumTries;

	for( UINT iTri = 0; iTri < uiTriesCnt_; ++iTri )
	{
		WzFace&		WzTri = _TerrainMesh.m_arList_Tries.Get_( iTri );

		iTextureIdx = _TerrainMesh.m_arListTri_TextureIdx.Get_(iTri);

		uiCurLMSeq = _TerrainMesh.m_arConvertToLMSeqFromParseTextureIdx.Get_( iTextureIdx );

		CLMMesh_&	CurLMMesh = *ArLMMeshes.Get_( uiCurLMSeq );

		CTri		CurLMMeshTri;

		Vec3		pos[3];
		Vec2		AlphaUV[3];
		Vec3		normal[3];

		CurLMMeshTri._iTextureIndex = iTextureIdx;

		for( int iVert = 0; iVert < 3; ++iVert )
		{
			CurLMMeshTri._iIndexVertices[iVert] 
			= uiIndexVertex
				= (int)WzTri.m_dwVertPack[iVert];

			WzMeshVertPack& CurVP = _TerrainMesh.m_arList_VertexPack.Get_(uiIndexVertex);

			DWORD		dwIndexVert = CurVP.m_dwVertexIndex;

			// m_List_Vertex[m_List_VertexPack[m_List_Face[j].m_dwVertPack[0]].m_dwVertexIndex].m_wvPos;
			pos[iVert]				= _TerrainMesh.m_arList_Vertex.Get_(dwIndexVert);	// Position
			normal[iVert].x			= CurVP.m_wvNormal.x;						// Normal
			normal[iVert].y			= CurVP.m_wvNormal.y;						// Normal
			normal[iVert].z			= CurVP.m_wvNormal.z;						// Normal

			AlphaUV[iVert].x			= CurVP.m_wuvVert.u;					// UV Vertex
			AlphaUV[iVert].y			= CurVP.m_wuvVert.v;					// UV Vertex

			CurLMMeshTri._uiVertexSeqs[iVert] = CurLMMesh.InsertVertex_( 
				pos[iVert].x, pos[iVert].y, pos[iVert].z, 
				normal[iVert].x, normal[iVert].y, normal[iVert].z,
				AlphaUV[iVert].x, AlphaUV[iVert].y,
				dwIndexVert );
		}

		UINT uiTri = CurLMMesh.InsertTri( CurLMMeshTri );

		Vec3&		v3Face_normal = CurLMMesh.GetTriNormal( uiTri );

		CRasterTri	RasterTriangle;
		RasterTriangle.InitilizeTriangle( pos, 
										normal, 
										v3Face_normal, 
										AlphaUV,
										CurLMMesh.m_AlphaTexture, 
										iRasterTriCount );

		if( RasterTriangle.CheckValidate() )		// �ﰢ���� �κ��� �Ÿ� �� ������ ��ȿ�� �˻�
		{
			ArWholeTriangles.push_back(RasterTriangle);

			UINT		uiCur = ArWholeTriangles.size() - 1;
			CRasterTri&		CurRTri = ArWholeTriangles[uiCur];

			CurLMMesh.InsertRasterTri(iRasterTriCount, uiTri);

			++iRasterTriCount;
		}
	}

	// ���� �۾� ����
	//// ���� �ؽ��� ����
	//// ============================================================================================
	//CAlphaTexture &tCurAlphaTexture = CurLMMesh.m_AlphaTexture;	// texture info�� mesh group ���� �������� ����

	//// CAlphaTexture�� ���� alpha ����  : Image�� ���� Loading �Ͽ� Alpha Texture ���� �Ǵ�
	//tCurAlphaTexture.RealizeTextureOnlyAlphaChannel( m_lpszFileName );

	//// /���� ���� �ؽ��� ����
	//// ============================================================================================
}


// Alpha Texture ������ �� ���Ŀ� ȣ�� �Ǿ�� �Ѵ�.
// Alpha Texture ���� ��� ��� 
//		1. �ܺο��� AlphaBuffer�� ���� ���ڷ� �޾Ƽ�, ���� Array�� ��ȯ
//				ex) manager->SetAlphaBuffer(MeshID, AlphaBuffer);		ok
//		2. �ܺο��� ���� Array�� ������ �� �ִ� ���ڸ� ����
//				 ex) manager->GetMesh(n)->SetAlphaArray(nIndex, ALphaVIsibility);
//		3. �ܺο��� AlphaTextureFileName�� �޾Ƽ�, ���� ��ȯ			ok
BOOL	CMeshConverter::ConvertToLMMeshes( const mapParserMeshes& ObjectInstances, 
								  Array_<CLMMesh_*>& ArrLMMeshes, 
								  Array_<CParentLMMesh*>& ArrParentMeshes,
								  vector<CRasterTri>& ArrWholeTriangles, 
								  CCommonWz& _Wz,
								  UINT& uiCntRealityOutputLMObject,
								  UINT& uiGlobalyLMCnt,
								  CLMOption& LMOption,
								  vector<int> *pvExceptionCastShadowObjectsList,
								  vector<int> *pvExceptionReceiveShadowObjectsList )
{
	mapParserMeshes::iterator it;
	DWORD		dwParentExID = 0;

	it = (*(const_cast<mapParserMeshes*>(&ObjectInstances))).begin();

	UINT			iRasterTriCount	= ArrWholeTriangles.size();
	UINT			uiLMMeshCnt = 0, uiParentMeshCnt = 0, uiCurMeshID = 0, uiCurParentMeshID = 0;

	uiParentMeshCnt = ArrParentMeshes.GetSize();

	BOOL			bComputeAlphaTexture = FALSE;
	BOOL			bExceptionCurObjectCastingShadow = FALSE;
	BOOL			bExceptionCurObjectReceivingShadow = FALSE;

	vector<int>::iterator	itC_S, itC_E, itC_;
	vector<int>::iterator	itR_S, itR_E, itR_;

	//char			szIdx[8], *szCur = 0;
	//memset( szIdx, 0, sizeof(char) * 8 );

	int				iCurIdx = 0;

	itC_S = pvExceptionCastShadowObjectsList->begin();
	itC_E = pvExceptionCastShadowObjectsList->end();

	itR_S = pvExceptionReceiveShadowObjectsList->begin();
	itR_E = pvExceptionReceiveShadowObjectsList->end();

	// object instance ����
	for( ;it !=  ObjectInstances.end(); ++it )
	{
		uiCurParentMeshID = uiParentMeshCnt++;

		_CParserLightmapMeshInstance &CurBatchMesh_		= *((*it).first);
		_CParserLightMapMesh &CurMeshComponent_	= (*it).second;

		CParentLMMesh* pCurParentMesh = new CParentLMMesh();
		ArrParentMeshes.Set__( uiCurParentMeshID, pCurParentMesh );
		CParentLMMesh& CurParentMesh = *pCurParentMesh;

		bComputeAlphaTexture = CurMeshComponent_.m_bComputeAlphaTexture;

		bExceptionCurObjectCastingShadow = FALSE;
		bExceptionCurObjectReceivingShadow = FALSE;

		//itC_ = find( itC_S, itC_E, szIdx );
		for( itC_ = itC_S; itC_ != itC_E; ++itC_ )
		{
			iCurIdx = (*itC_);

			if( iCurIdx == CurBatchMesh_._iIdentitySeq )
			{
				bExceptionCurObjectCastingShadow = TRUE;
				break;
			}
		}

		//bExceptionCurObjectCastingShadow = ( (itC_E == itC_)?FALSE:TRUE );

		//itR_ = find( itR_S, itR_E, szIdx );
		for( itR_ = itR_S; itR_ != itR_E; ++itR_ )
		{
			iCurIdx = (*itR_);

			if( iCurIdx == CurBatchMesh_._iIdentitySeq )
			{
				bExceptionCurObjectReceivingShadow = TRUE;
				break;
			}
		}
		//bExceptionCurObjectReceivingShadow = ( (itC_E == itC_)?FALSE:TRUE );

		// LM Parent Mesh : 
		CurParentMesh.Initialize( uiCurParentMeshID, 
			CurBatchMesh_.entityindex,						// Parent Ex ID
			CurBatchMesh_.WzID,
			!bExceptionCurObjectCastingShadow,
			!bExceptionCurObjectReceivingShadow & LMOption._bComputeShadow );

		dwParentExID = CurParentMesh.GetExID();

		if( TRUE == CurBatchMesh_.receive_shadows )
		{
			++(uiCntRealityOutputLMObject);
		}

		WzMeshVertex* vertexlist = CurMeshComponent_.m_wzd->GetVertexList();
		int nmg = CurMeshComponent_.m_wzd->GetNumberOfMeshGroup();

		CurParentMesh.SetSubMeshCnt( nmg );		// New Allocate

		// pWzMeshGroup ���� /  
		int iGM=0,ni = nmg;
		for( iGM = 0 ; iGM < ni ; ++iGM )
		{

			uiCurMeshID = uiGlobalyLMCnt++;

			CLMMesh_* pCurMesh = new CLMMesh_();
			ArrLMMeshes.Set__( uiCurMeshID, pCurMesh );

			CLMMesh_& CurLMMesh = *pCurMesh;


			UINT	uiSeq = CurParentMesh.InsertSubMeshIdx( uiCurMeshID );

			WzMeshGroup *pWzMeshGroup = CurMeshComponent_.m_wzd->GetMeshGroupInfo(iGM);

			int numface = pWzMeshGroup->GetNumFace();

			// LMMesh : Mesh ID, WorldTransform, Environments, Parent Mesh ID
			CurLMMesh.Initialize(	uiCurMeshID,
				iGM,
				dwParentExID,
				numface,
				numface*3,
				&CurMeshComponent_.m_matrix, 
				!bExceptionCurObjectCastingShadow,
				!bExceptionCurObjectReceivingShadow & LMOption._bComputeShadow, 
				bComputeAlphaTexture,
				FALSE,
				uiCurParentMeshID );


			// ���� �ؽ��� ����
			// ============================================================================================
			//if( LMOption. )
			CAlphaTexture &tCurAlphaTexture = CurLMMesh.m_AlphaTexture;	// texture info�� mesh group ���� �������� ����

			// �޽� �׷��� Diffuse �ؽ��� �ڵ� �� ���� ���ο� ���� Alpha Flag ����
			if (pWzMeshGroup->m_ahTexture[0] == HANDLE(-1) || pWzMeshGroup->m_ahTexture[0] == HANDLE(0))
			{
				tCurAlphaTexture.m_alpha_flag = false;
				continue;
			}
			else
			{
				HANDLE hTexture = pWzMeshGroup->m_ahTexture[0];

				// tCurAlphaTexture�� Image�� ���� Alpha�� Buffer�� ó�� -> 
				// �� ������ �������� tCurAlphaTexture.m_alpha_data[idx] = buf[idx*bpp + 3];
				// objectinstance->m_AlphaTexture[meshgroup��]->m_alpha_data[tessel��]
				St_TextureContainer *tc = _Wz.GetWzDraw()->x_pManagerTexture->GetTextureInfo(hTexture);

				// CAlphaTexture�� ���� alpha ����  : Image�� ���� Loading �Ͽ� Alpha Texture ���� �Ǵ�
				tCurAlphaTexture.RealizeTextureOnlyAlphaChannel( tc->m_lpszFileName, tc->m_nWidth, tc->m_nHeight );

			}

			CurLMMesh.SetComputeAlpha( (BOOL)tCurAlphaTexture.m_alpha_flag );		// Alpha ��� ���� ����

			// /���� ���� �ؽ��� ����
			// ============================================================================================



			UINT	uiTri = 0;
			// triangle ���� 
			// ���� triangle vertex�� vertexList�� position�� normal���� 
			// instance�� world position���� normal�� ���� ���� ����
			for(int iFace = 0 ; iFace < numface ; ++iFace )
			{
				WzFace &face = pWzMeshGroup->m_pwfFaces[iFace];
				DWORD *vidx = face.m_dwVertPack;		// Group VertexIndex ����

				// getting triangle normal
				Vec3 pos[3];
				Vec2 UV[3];
				Vec3 normal[3];

				UINT arrSeqVertex[3];

				int arrIndexVerticies[3];

				// �� Index Vertices �� -> World Vertices ��ȯ�� �Ͽ� Geometry �� �����Ѵ�.
				for( int k = 0; k < 3 ; ++k )
				{
					arrIndexVerticies[k] = pWzMeshGroup->m_pwMVPs[vidx[k]].m_dwVertexIndex;

					Vec3&	v3Pos = *(Vec3*)&vertexlist[arrIndexVerticies[k]].m_wvPos;
					Vec2&	v2UV = *(Vec2*)&pWzMeshGroup->m_pwMVPs[vidx[k]].m_wuvVert;
					Vec3&	v3Normal = *(Vec3*)&pWzMeshGroup->m_pwMVPs[vidx[k]].m_wvNormal;

					arrSeqVertex[k] = 
						CurLMMesh.InsertVertex_( v3Pos, v3Normal, v2UV, arrIndexVerticies[k] );

					CLMVertex& LMver = CurLMMesh.GetLMVertex_(arrSeqVertex[k]);

					UV[k].x = v2UV.x;
					UV[k].y = v2UV.y;

					pos[k] = LMver._v3Pos;
					normal[k] = LMver._v3Normal;
				}

				// Geometry Tri ����	// 
				uiTri = CurLMMesh.InsertTri( arrSeqVertex, arrIndexVerticies );

				// shadow�� �޴� ������Ʈ�� �ƴϸ� rasterTri ������ ���� �ʴ´�.
				if( FALSE == CurLMMesh.GetCastShadow() ) continue;

				// 2. RasterTriangle ���� : 
				//		���� pos, FaceNormal, VertexNormal -> WorldTM ���� �Ǿ��־�� �Ѵ�.
				// ============================================================================================
				// world ���� vertex�� �� triangle ���� ���� 

				// Geometry �� �����Ǿ� ���� ��� �Ǿ��� TriNormal�� �����ͼ� RasterTriNormal�� �����Ѵ�.
				Vec3&	v3Face_normal = CurLMMesh.GetTriNormal( uiTri );

				CRasterTri RasterTriangle;
				RasterTriangle.InitilizeTriangle( pos, 
					normal, 
					v3Face_normal,
					UV,
					CurLMMesh.m_AlphaTexture,
					iRasterTriCount );

				if( RasterTriangle.CheckValidate() )		// �ﰢ���� �κ��� �Ÿ� �� ������ ��ȿ�� �˻�
				{
					ArrWholeTriangles.push_back(RasterTriangle);

					CurLMMesh.InsertRasterTri((DWORD)iRasterTriCount, uiTri);

					++iRasterTriCount;
				} 

				//		���� pos, FaceNormal, VertexNormal -> WorldTM ���� �� AlphaTexture �� ���� �Ǿ��־�� �Ѵ�.
				// ============================================================================================
			} // for ( numfaces )
		}	

		// for( iGM = 0 ; iGM < ni ; ++iGM ) pWzMeshGroup ���� / ����
	}

	return TRUE;
}

