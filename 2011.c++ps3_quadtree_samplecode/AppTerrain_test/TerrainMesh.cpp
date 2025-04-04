#include "TerrainMesh.h"

#ifdef _DEBUG_TERRAINTILE
CTerrainTile::CTerrainTile()
{

}

CTerrainTile::~CTerrainTile()
{

}

float CTerrainTile::interpolateValue(int i, int maxInt, float maxFloat)
{
	return (i * maxFloat) / maxInt - (maxFloat*0.5f);
}


float CTerrainTile::interpolateValueDistribute(int i, int maxInt, float maxFloat)
{
	float value = (i * maxFloat * 2) / maxInt - maxFloat;

	return value * 2;
}

#ifdef _DEBUG_SPACETREE
bool CTerrainTile::getIntersectTri(const BV::_Ray &_ray, 
								   Vector4 *pPickedTriangle_out, 
								   Vector3 &v3PosIntersect_out )
{
	CTri tripicked;
	bool bPicked = m_pQuadTree->rayIntersectGetPos( _ray, tripicked, v3PosIntersect_out );
	if(true==bPicked)
	{
		for(int iP=0; iP<3; ++iP)
		{
			vectorCopy3to4(pPickedTriangle_out[iP],tripicked.v3PT[iP],1.0f);
		}
	}

	return bPicked;
}

bool CTerrainTile::getIntersectTri(	const BV::_Ray &_ray, 
									int &_iTriIntersect )
{
	return m_pQuadTree->rayIntersectGetIdxTri( _ray, _iTriIntersect );
}
#endif // _DEBUG_SPACETREE


#if defined(_DEBUG_SPACETREE) && defined(_DEBUGFOR_RENDER)
void CTerrainTile::renderspacetree_fordebug(	const Vector3	&v3colorLine,
												const Vectormath::Aos::Matrix4 *matViewPrjCamera )
{
	m_pQuadTree->renderAllNodes_debug( v3colorLine, matViewPrjCamera );
}
#endif // defined(_DEBUG_SPACETREE) && defined(_DEBUGFOR_RENDER)

PShaderInstance *CTerrainTile::createTestTexturedShaderInstance(PDatabase *database,
																unsigned int width, 
																unsigned int height, 
																unsigned int checkerWidth, 
																unsigned int checkerHeight )
{
	PResult result;
	// 텍스쳐및 조명을 포함한 텍스쳐 쉐이더 인스턴스 생성
	PShaderInstance *texturedShaderInstance = 
		PShaderGroup::createDefaultShaderInstance<PShaderInstance>(
				PShaderGroup::PE_DEFAULT_SHADER_GROUP_PRELIT_TEXTURED, *database, PDatabaseUniqueNameHelper(*database, "sampleMaterial"), &result);
	if(result != PE_RESULT_NO_ERROR)
		return PSSG_SET_LAST_ERROR((PShaderInstance*)NULL, ("Unable to create the shader instance"));

	// 텍스처를 추가하여 만든 질감 인스턴스 설정
	// Extra 도우미 함수를 사용하여 텍스처 만들기
	PTexture *checkerTexture = PSSG::Extra::createCheckerTexture(*database, width, height, checkerWidth, checkerHeight);
	if(!checkerTexture)
		return PSSG_SET_LAST_ERROR((PShaderInstance*)NULL, ("Unable to create the checkerTexture texture"));

	// 쉐이더에 텍스쳐 매개 변수를 설정
	// 쉐이더 인스턴스 0번에 텍스쳐를 연결
	texturedShaderInstance->setTextureParameter(0U, checkerTexture);

	return texturedShaderInstance;
}


bool CTerrainTile::createTileMesh_Test(	PRootNode *pnodeRoot,
										PDatabase *pDatabase,
										float fdistHorizontal,
										float fdistVertical,
										float fMeshScale,
										float fCntWave,
										float fIntensityWave,
										unsigned int uiSubdivisions,		// 가로, 세로를 몇개를 나눌지.
										const Vector3 &v3Translation )
{
	PResult internalResult;

	unsigned int i, j;
	unsigned int subdivisions = uiSubdivisions;
	unsigned int indexCount = subdivisions * subdivisions * 6;
	unsigned int vertexCount = (subdivisions + 1) * (subdivisions + 1);

	// インデックスソースを追加
	PRenderIndexSource *indexSource = 
		pDatabase->createObject<PRenderIndexSource>(
		PDatabaseUniqueNameHelper(
		*pDatabase, "indexSource"), &internalResult);
	if(!indexSource)
	{
		internalResult = PE_RESULT_OUT_OF_MEMORY;
		return PSSG_SET_LAST_ERROR(false, ("Unable to create index source"));
	}

	// インデックスタイプは「unsigned int」となり、トライアングルプリミティブが描画される
	internalResult = indexSource->create(*PDataType::getType(PE_TYPE_UINT), indexCount, PRenderInterface::PE_PRIMITIVE_TRIANGLES);
	if(internalResult != PE_RESULT_NO_ERROR)
	{
		delete indexSource;
		return PSSG_SET_LAST_ERROR(false, ("Unable to allocate space in index source"));
	}

	// バッファを設定したら、データを設定する		// 인덱스 버퍼를 설정하면, 데이터 설정
	unsigned int *indexData = (unsigned int *)indexSource->getData();
	for(i = 0 ; i < subdivisions ; i++)
	{
		for( j = 0 ; j < subdivisions ; j++ )
		{
			*indexData++ = i * (subdivisions + 1) + j;
			*indexData++ = i * (subdivisions + 1) + j + 1;
			*indexData++ = (i + 1) * (subdivisions + 1) + j;

			*indexData++ = i * (subdivisions + 1) + j + 1;
			*indexData++ = (i + 1) * (subdivisions + 1) + j + 1;
			*indexData++ = (i + 1) * (subdivisions + 1) + j;
		}
	}

	// レンダーデータソースをセグメントセットに追加する
	// このデータソースにはストリームが含まれる
	PRenderDataSource *renderDataSource = pDatabase->createObject<PRenderDataSource>(PDatabaseUniqueNameHelper(*pDatabase, "meshDataSrc"), &internalResult);
	if(internalResult != PE_RESULT_NO_ERROR)
	{
		delete indexSource;
		return PSSG_SET_LAST_ERROR(false, ("Unable to create data source"));
	}
	renderDataSource->setIndexSource(indexSource);


	// 頂点、法線、ST0（テクスチャ座標0）
	internalResult = renderDataSource->setStreamCountWithUniqueBlocks(3);
	if(internalResult != PE_RESULT_NO_ERROR)
	{
		delete indexSource;
		delete renderDataSource;
		return PSSG_SET_LAST_ERROR(false, ("Unable to set stream count in data source"));
	}


	// 頂点を保持するデータブロックを追加		// 정점을 유지하는 데이터 블록을 추가
	PRenderStream *vertexStream = renderDataSource->getStream(0);
	PDataBlock *vertexBlock = vertexStream->getDataBlock();
	internalResult = vertexBlock->setSingleStream(vertexCount, *PDataType::getType(PE_TYPE_FLOAT3), *PSSG_GET_RENDERTYPE(Vertex));
	if(internalResult == PE_RESULT_NO_ERROR)
	{
		// バッファを設定したら、データを設定する		// 버퍼를 설정하면, 데이터 설정
		float *vertexData = (float *)vertexBlock->getData();

		// Min Max Boundary
		float fMinX=0.0f, fMinY=0.0f, fMinZ=0.0f, fMaxX=0.0f, fMaxY=0.0f, fMaxZ=0.0f;
		Vector3 v3Min, v3Max, v3Current;

		fMinX=fMinY=fMinZ=99999.0f;
		fMaxX=fMaxY=fMaxZ=-99999.0f;

		float fDistHorizontal = (fMeshScale * fdistHorizontal) / subdivisions;			
		float fDistVertical = (fMeshScale * fdistVertical) / subdivisions;				
		for(i = 0; i < subdivisions + 1; i++)
		{
			for(j = 0; j < subdivisions + 1; j++)
			{
				float fDistributeInterpolX = interpolateValueDistribute( i,subdivisions, fCntWave );
				float fDistributeInterpolY = interpolateValueDistribute( j,subdivisions, fCntWave );

				*vertexData++ = v3Translation.getX() + 
					(float)(i * (fDistHorizontal) - ((float)subdivisions * (fDistHorizontal)/2.0f));		// X
				*vertexData++ = v3Translation.getY() + 
					(sin(fDistributeInterpolX+cos(fDistributeInterpolY))*fIntensityWave);	// Y
				*vertexData++ = v3Translation.getZ() + 
						(float)(j * (fDistVertical) - ((float)subdivisions * (fDistVertical)/2.0f));		// Z

				// Min Max 설정
				(fMinX > *(vertexData-3))?fMinX=*(vertexData-3):1;
				(fMinY > *(vertexData-2))?fMinY=*(vertexData-2):1;
				(fMinZ > *(vertexData-1))?fMinZ=*(vertexData-1):1;

				(fMaxX < *(vertexData-3))?fMaxX=*(vertexData-3):1;
				(fMaxY < *(vertexData-3))?fMaxY=*(vertexData-2):1;
				(fMaxZ < *(vertexData-3))?fMaxZ=*(vertexData-1):1;	
			}
		}

		m_v3BoundaryMin.setX(fMinX);	m_v3BoundaryMin.setY(fMinY);	m_v3BoundaryMin.setZ(fMinZ);
		m_v3BoundaryMax.setX(fMaxX);	m_v3BoundaryMax.setY(fMaxY);	m_v3BoundaryMax.setZ(fMaxZ);
	}

#ifdef _DEBUG_SPACETREE
	// Construct All Triangle and Quad Tree based on IndexBuffer, VertexBuffer 
	m_pContainerTri	= m_pContainerTri->getthis();
	m_pContainerTri->constructAllTriangles( renderDataSource );

	m_pQuadTree = m_pQuadTree->getthis();
	m_pQuadTree->constructTree( m_pContainerTri );
#endif // _DEBUG_SPACETREE

	// 法線を保持するデータブロックを追加		// 법선을 포함하는 데이터 블록을 추가
	PRenderStream *normalStream = renderDataSource->getStream(1);
	PDataBlock *normalBlock = normalStream->getDataBlock();
	internalResult = normalBlock->setSingleStream(vertexCount, *PDataType::getType(PE_TYPE_FLOAT3), *PSSG_GET_RENDERTYPE(Normal));
	if(internalResult == PE_RESULT_NO_ERROR)
	{
		// バッファを設定したら、データを設定する
		float *normalData = (float *)normalBlock->getData();
		for(i = 0; i < vertexCount ; i++)
		{
			// 法線
			*normalData++ = 0.0f;
			*normalData++ = 1.0f;
			*normalData++ = 0.0f;
		}
	}

	// STを格納するデータブロックを追加する		// ST Texture 좌표를 포함하는 데이터 블록을 추가
	PRenderStream *texcoordStream = renderDataSource->getStream(2);
	PDataBlock *texcoordBlock = texcoordStream->getDataBlock();
	internalResult = texcoordBlock->setSingleStream(vertexCount, *PDataType::getType(PE_TYPE_FLOAT2), *PSSG_GET_RENDERTYPE(ST));
	if(internalResult == PE_RESULT_NO_ERROR)
	{
		// バッファを設定したら、データを設定する
		float *texcoordData = (float *)texcoordBlock->getData();
		for(i = 0; i < subdivisions + 1; i++)
		{
			for(j = 0; j < subdivisions + 1; j++)
			{
				// テクスチャ座標	// 텍스쳐 좌표
				*texcoordData++ = (float)i / (float)subdivisions;
				*texcoordData++ = (float)j / (float)subdivisions;
			}
		}
	}


	// 모디파이어 네트워크 설정
	PModifierNetwork *pModifierNetwork = PModifierNetwork::getStandardNetwork(PModifierNetwork::PE_NETWORK_TRANSFORM_LIGHT);
	if(!pModifierNetwork)
		return PSSG_SET_LAST_ERROR(false, ("Unable to create the modifier pModifierNetwork"));



	// 렌더 노드에 Boundary Min, Max 설정
	PRenderNode *pRenderNodeMesh = pnodeRoot->createChildNode<PRenderNode>(PDatabaseUniqueNameHelper(
		*pDatabase, "meshRenderNode"), &internalResult);
	if(internalResult != PE_RESULT_NO_ERROR)
		return PSSG_SET_LAST_ERROR(false, ("Unable to create the render node"));
	pRenderNodeMesh->m_boundsMax = m_v3BoundaryMax;
	pRenderNodeMesh->m_boundsMin = m_v3BoundaryMin;

	// SampleTexture 설정
	PShaderInstance *texturedShaderInstance = createTestTexturedShaderInstance( pDatabase, 160, 160, 8, 8 );

	// 세그먼트 및 렌더 노드를 이용하여 메쉬를 인스턴스 화
	PSSG::Extra::instanceSegment(	*pRenderNodeMesh, 
									*renderDataSource, 
									*texturedShaderInstance, 
									*pModifierNetwork );

	return true;
}


void CTerrainTile::Release()
{
#ifdef _DEBUG_SPACETREE
	m_pQuadTree->Release();
	m_pQuadTree->destroyInstance();

	m_pContainerTri->Release();
	m_pContainerTri->destroyInstance();
#endif // _DEBUG_SPACETREE
}
#endif //_DEBUG_TERRAINTILE





#ifdef _DEBUG_TERRAINSHAPE_TEST

bool CTerrainShapeForSpaceTree::constructSpaceTree( PDatabase *pDatabase )
{
	PSegmentSet *segmentSet;
	PDatabaseListableIterator<PSegmentSet>		it1(*pDatabase);

	m_pQuadTree = m_pQuadTree->getthis();

	while (it1)
	{
		segmentSet = &(*it1);

		for(unsigned int i = 0 ; i < segmentSet->getSegmentCount() ; i++)
		{
			PRenderDataSource *source = segmentSet->getSegment(i);

			m_pContainerTri->constructAllTriangles( source );	
		}
		++it1;
	}

	m_pQuadTree->constructTree( m_pContainerTri );

	return true;
}


#if defined(_DEBUG_SPACETREE) && defined(_DEBUGFOR_RENDER)
void CTerrainShapeForSpaceTree::renderspacetree_fordebug(	const Vector3	&v3colorLine,
															const Vectormath::Aos::Matrix4 *matViewPrjCamera )
{
	m_pQuadTree->renderAllNodes_debug( v3colorLine,matViewPrjCamera );
}
#endif // defined(_DEBUG_SPACETREE) && defined(_DEBUGFOR_RENDER)


bool CTerrainShapeForSpaceTree::getIntersectTri(const BV::_Ray &_ray, 
								   Vector4 *pPickedTriangle_out, 
								   Vector3 &v3PosIntersect_out )
{
	CTri tripicked;
	bool bPicked = m_pQuadTree->rayIntersectGetPos( _ray, tripicked, v3PosIntersect_out );
	if(true==bPicked)
	{
		for(int iP=0; iP<3; ++iP)
		{
			vectorCopy3to4( pPickedTriangle_out[iP] , tripicked.v3PT[iP],1.0f );
		}
	}

	return bPicked;
}

bool CTerrainShapeForSpaceTree::createTerrainShape_test(	PRootNode			*pnodeRoot,
															PDatabase			*pDatabase,
															PString				szNameFileTerrain,
															float				fScaleTerrain )
{

// 	// 1. TEST
// 	PResult result;
// 
// 	m_pNoderootTerr = pnodeRoot->createChildNode<PVisibleRenderNode>(PDatabaseUniqueNameHelper(*pDatabase, "terrainShapeForSpaceTree"), &result);
// 	if(result != PE_RESULT_NO_ERROR)
// 		return PSSG_SET_LAST_ERROR(false, ("Unable to create the screen graph root node"));	
// 
// 
// 	PDatabaseID terrainDatabaseID;
// 	if(PLinkResolver::getDatabase(terrainDatabaseID, szNameFileTerrain) != PE_RESULT_NO_ERROR)
// 		return PSSG_SET_LAST_ERROR(false, ("Unable to find terrain database"));
// 	if(PSSG::Extra::resolveAllLinks() != PE_RESULT_NO_ERROR)
// 		return PSSG_SET_LAST_ERROR(false, ("Unable to resolve terrain database"));
// 
// 	PDatabaseReadLock readLock(terrainDatabaseID);
// 	PDatabase *databaseShapeTerr = readLock.getDatabase();
// 	if(!databaseShapeTerr)
// 		return PSSG_SET_LAST_ERROR(false, ("Unable to lock terrain database"));
// 
// 	PSegmentSet *segmentSet

// 	PDatabaseListableIterator<PSegmentSet>		it1(*databaseShapeTerr);
// 	while (it1)
// 	{
// 		segmentSet = &(*it1);
// 		for(unsigned int i = 0 ; i < segmentSet->getSegmentCount() ; i++)
// 		{
// 			PRenderDataSource *renderdatasource = segmentSet->getSegment(i);
// 			m_pContainerTri->constructAllTriangles( renderdatasource );
// 		} 
// 		++it1;
// 	}
// 
// 	//m_pQuadTree->constructTree( m_pContainerTri );

	// 2. TEST

	PResult result;
	if(PLinkResolver::getDatabase(m_terrainDBID, szNameFileTerrain)!=PE_RESULT_NO_ERROR)
	{
		return PSSG_SET_LAST_ERROR(false,("Unable to get the default database for this sample."));
	}

	if(PSSG::Extra::resolveAllLinks()!=PE_RESULT_NO_ERROR)
	{
		return PSSG_SET_LAST_ERROR(false,("Unable to resolve all outstanding links"));
	}

	// Database File을 읽어온다.
	PDatabaseWriteLock modelWriteLock(m_terrainDBID);
	PDatabase *databaseShapeTerr = modelWriteLock.getDatabase();
	if(!databaseShapeTerr)
	{
		return PSSG_SET_LAST_ERROR(false,("Unable to lock model database"));
	}

	// Bundle Node : 즉 Model Node를 뽑아낸다.
	PListIterator modelScenes(databaseShapeTerr->getSceneList());
	PNode *pnodeRootShapeTerr = (PNode*)modelScenes.data();

	PTraversalFindNodeByType findBUndleNode(PSSG_TYPE(PBundleNode));
	findBUndleNode.traverseDepthFirst(*pnodeRootShapeTerr);
	PNode *pnodeRenderBundle = (PNode*)findBUndleNode.getFoundNode();

	if(!pnodeRenderBundle)
	{
		PTraversalFindNodeByType findRenderNode(PSSG_TYPE(PRenderNode));
		findRenderNode.traverseDepthFirst(*pnodeRootShapeTerr);
		pnodeRenderBundle = (PNode*)findRenderNode.getFoundNode();
		if(!pnodeRenderBundle)
		{
			return PSSG_SET_LAST_ERROR(false, ("Unable to find the model node because model has not rendernode."));
		}
	}

	PNode *pmodelNode_clone = (PNode*)pnodeRenderBundle->clone(pDatabase, &result);


	bool bNeedTransform = !APPROX_EQ(1.0f,fScaleTerrain);
	Matrix4	matTMforTree;
	
	if( true==bNeedTransform )
	{
		Matrix4	&mat = pmodelNode_clone->m_matrix;
		matTMforTree = matTMforTree.identity();
		float	fscaleMul = 0.0f;

		for(int i=0; i<3; ++i)
		{
			fscaleMul = mat.getElem(i,i)*fScaleTerrain;
			mat.setElem( i,i, fscaleMul );
			matTMforTree.setElem( i,i,fscaleMul );
		}
	}
 
	pnodeRoot->addChild( *pmodelNode_clone );

	PSegmentSet *segmentSet;
	PDatabaseListableIterator<PSegmentSet>		it1(*databaseShapeTerr);
	while (it1)
	{
		segmentSet = &(*it1);
		for(unsigned int i = 0 ; i < segmentSet->getSegmentCount() ; i++)
		{
			PRenderDataSource *renderdatasource = segmentSet->getSegment(i);

			if(true==bNeedTransform)
			{
				m_pContainerTri->constructAllTriangles( renderdatasource, &matTMforTree );
			}
			else
			{
				m_pContainerTri->constructAllTriangles( renderdatasource );
			}
		} 
		++it1;
	}

	m_pQuadTree->constructTree( m_pContainerTri );

	return true;
}

// void CTerrainShapeForSpaceTree::Render(	PSSG::PRenderInterface *renderInterface, 
// 										int iwidthViewPort, 
// 										int iheightViewPort, 
// 										PSSG::PCameraNode *pnodeCamera )
// {
// 	PSSG::Extra::renderDatabaseWithPortals(
// 									m_terrainDBID,	// ID of the database to render
// 									*renderInterface,		// Render interface to be used for rendering
// 									iwidthViewPort, 
// 									iheightViewPort,		// Dimensions of the viewport
// 									~0U,					// Render all passes
// 									pnodeCamera,			// camera to render from.
// 									NULL);					// Use the default context
// }

void CTerrainShapeForSpaceTree::Release()
{
	m_pQuadTree->Release();
	m_pQuadTree->destroyInstance();

	m_pContainerTri->Release();
	m_pContainerTri->destroyInstance();
}


CTerrainShapeForSpaceTree::CTerrainShapeForSpaceTree() 
{

	m_pContainerTri	= m_pContainerTri->getthis();
	m_pQuadTree = m_pQuadTree->getthis();
}

CTerrainShapeForSpaceTree::~CTerrainShapeForSpaceTree()
{

}

#endif // _DEBUG_TERRAINSHAPE_TEST
