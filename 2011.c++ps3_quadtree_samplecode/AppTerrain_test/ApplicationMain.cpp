
#include "ApplicationMain.h"

// サンプルアプリケーションのスタティックなインスタンス
static CApplicationMain s_application;
const char *CApplicationMain::s_databaseName = "test.test";

#ifdef _DEBUG_COLLECTPICKING_FORNAVI
bool CApplicationMain::IntersectRayCollectTris(const bool			bRayStartasMousePos,
											   PCameraNode			*cameraActive, 
											   BV::_Ray				&ray_out,
											   vector<int>			&vecTrisPicked,
											   E_MODE_COLLECTNAVI	eModeCollectNavi )
{
	if(!cameraActive)
	{
		return false;
	}


	bool bPickedTriInTree = false;

	// construct ray
	int view[4] = {0, 0, m_width, m_height};
	int iXViewPort_RayStartPos,iYViewPort_RayStartPos;
	Vector4 v4lineEnd, v4lineStart;
	Vector3 v3lineEnd, v3lineStart;
	if( true==bRayStartasMousePos )
	{
		iXViewPort_RayStartPos = m_mouseX;	
		iYViewPort_RayStartPos = m_mouseY;
	}
	else
	{
		iXViewPort_RayStartPos = (int)(m_width*0.5f);
		iYViewPort_RayStartPos = (int)(m_height*0.5f);
	}

	PSSG::Extra::screenToWorld(iXViewPort_RayStartPos, m_height - iYViewPort_RayStartPos, 1.0f, cameraActive->m_inverseGlobalMatrix, cameraActive->m_projection, view, v4lineEnd);
	PSSG::Extra::screenToWorld(iXViewPort_RayStartPos, m_height - iYViewPort_RayStartPos, -1.0f, cameraActive->m_inverseGlobalMatrix, cameraActive->m_projection, view, v4lineStart);

	vectorCopy4to3(v3lineEnd,v4lineEnd);
	vectorCopy4to3(v3lineStart,v4lineStart);

	Vector3	v3dirRay = v3lineEnd-v3lineStart;
	v3dirRay = normalize(v3dirRay);
	ray_out.set( v3lineStart, v3dirRay, true );

	// object ray tracing
	Vector3	v3PosIntersect;
	int iIdxTri = 0;

	bPickedTriInTree = m_pQuadTree->rayIntersectGetIdxTri( ray_out, iIdxTri );

	if(true==bPickedTriInTree)
	{
		if(E_MODE_COLLECTNAVY_INSERT==eModeCollectNavi)
		{
			if(vecTrisPicked.size()>0)
			{
				vector<int>::iterator	iter_;
				sort(vecTrisPicked.begin(), vecTrisPicked.end());

				iter_ = find(vecTrisPicked.begin(), vecTrisPicked.end(), iIdxTri);
				bool bFound = iter_!=vecTrisPicked.end();

				if(true!=bFound)
				{
					vecTrisPicked.push_back(iIdxTri);
				}
			}
			else
			{
				vecTrisPicked.push_back(iIdxTri);
			}
		}
		else	// E_MODE_COLLECTNAVY_DELETE
		{
			if(vecTrisPicked.size()>0)
			{
				vector<int>::iterator	iter_;
				sort(vecTrisPicked.begin(), vecTrisPicked.end());

				iter_ = find(vecTrisPicked.begin(), vecTrisPicked.end(), iIdxTri);
				bool bFound = iter_!=vecTrisPicked.end();
				if(true==bFound)
				{
					vecTrisPicked.erase(iter_);
				}
			}
		}
	}

	return bPickedTriInTree;
}
#endif // _DEBUG_COLLECTPICKING_FORNAVI

#ifdef _DEBUG_SPACETREE
bool CApplicationMain::IntersectCameraRay_ST(		const bool			bRayStartasMousePos,
															PCameraNode			*cameraActive, 
															BV::_Ray			&ray_out,
															Vector4				*v4PickedTri_out )
{
	if(!cameraActive)
	{
#ifdef APPTARGET_IS_PS3
		char	szOutputDebug[512];
		unsigned int uiCntTris = m_pQuadTree->m_pTriContainer->getCountTries();

		sprintf( szOutputDebug, "\t IntersectCameraRay_ST Camera Node is NULL. " );
		PSSG_PRINTF( szOutputDebug );
#endif // APPTARGET_IS_PS3
		return false;
	}

	bool bPickedTriInTree = false;

	// construct ray
	int view[4] = {0, 0, m_width, m_height};
	int iXViewPort_RayStartPos,iYViewPort_RayStartPos;
	Vector4 v4lineEnd, v4lineStart;
	Vector3 v3lineEnd, v3lineStart;
	if( true==bRayStartasMousePos )
	{
		iXViewPort_RayStartPos = m_mouseX;	
		iYViewPort_RayStartPos = m_mouseY;
	}
	else
	{
		iXViewPort_RayStartPos = (int)(m_width*0.5f);
		iYViewPort_RayStartPos = (int)(m_height*0.5f);
	}

	PSSG::Extra::screenToWorld(iXViewPort_RayStartPos, m_height - iYViewPort_RayStartPos, 1.0f, cameraActive->m_inverseGlobalMatrix, cameraActive->m_projection, view, v4lineEnd);
	PSSG::Extra::screenToWorld(iXViewPort_RayStartPos, m_height - iYViewPort_RayStartPos, -1.0f, cameraActive->m_inverseGlobalMatrix, cameraActive->m_projection, view, v4lineStart);

	vectorCopy4to3(v3lineEnd,v4lineEnd);
	vectorCopy4to3(v3lineStart,v4lineStart);

	Vector3	v3dirRay = v3lineEnd-v3lineStart;
	v3dirRay = normalize(v3dirRay);
	ray_out.set( v3lineStart, v3dirRay, true );

	// object ray tracing
	Vector3	v3PosIntersect;

	CTri tripicked;
	bPickedTriInTree = m_pQuadTree->rayIntersectGetPos( ray_out, tripicked, v3PosIntersect );

	if(true==bPickedTriInTree)
	{
		for(int iP=0; iP<3; ++iP)
		{
			vectorCopy3to4(v4PickedTri_out[iP],tripicked.v3PT[iP],1.0f);
		}
	}

	return bPickedTriInTree;
}
#endif // _DEBUG_SPACETREE

bool CApplicationMain::IntersectCameraRay_Traversal(	const PSSG::PDatabaseID	dbaseID,
															const bool			bRayStartasMousePos,
															PCameraNode			*cameraActive,
															BV::_Ray			&ray_out,
															Vector4				*v4PickedTri_out )
{
	if(!cameraActive)
	{
#ifdef APPTARGET_IS_PS3
		char	szOutputDebug[512];
		unsigned int uiCntTris = m_pQuadTree->m_pTriContainer->getCountTries();

		sprintf( szOutputDebug, "\t IntersectCameraRay_ST Camera Node is NULL. " );
		PSSG_PRINTF( szOutputDebug );
#endif // APPTARGET_IS_PS3
		return false;
	}

	bool bPickedTriInPhyre = false;
	
	PDatabaseReadLock readLock(dbaseID);
	PDatabase *database = readLock.getDatabase();
	if(!database)
		return PSSG_SET_LAST_ERROR(false, ("Unable to lock sample database"));

	int view[4] = {0, 0, m_width, m_height};
	int iXViewPort_RayStartPos,iYViewPort_RayStartPos;
	if( true==bRayStartasMousePos )
	{
		iXViewPort_RayStartPos = m_mouseX;
		iYViewPort_RayStartPos = m_mouseY;
	}
	else
	{
		iXViewPort_RayStartPos = (int)(m_width*0.5f);
		iYViewPort_RayStartPos = (int)(m_height*0.5f);
	}

	for(PListIterator scenes(database->getSceneList()) ; scenes ; scenes.next())
	{
		PNode *root = (PNode *)scenes.data();
		if(!root)
			continue;

		cameraActive->setAspect((float)(m_width)/(float)(m_height));
		cameraActive->updateInverseGlobalMatrix();

		Vector4 v4lineEnd, v4lineStart;
		PSSG::Extra::screenToWorld(iXViewPort_RayStartPos, m_height - iYViewPort_RayStartPos, 1.0f, cameraActive->m_inverseGlobalMatrix, cameraActive->m_projection, view, v4lineEnd);
		PSSG::Extra::screenToWorld(iXViewPort_RayStartPos, m_height - iYViewPort_RayStartPos, -1.0f, cameraActive->m_inverseGlobalMatrix, cameraActive->m_projection, view, v4lineStart);

		Vector3 lineDir = Vector3((v4lineEnd - v4lineStart).getXYZ());
		lineDir = normalize(lineDir);

		Vector3 v3lineStart, v3lineEnd;

		vectorCopy4to3(v3lineEnd,v4lineEnd);
		vectorCopy4to3(v3lineStart,v4lineStart);

		Vector3	v3dirRay = v3lineEnd-v3lineStart;
		v3dirRay = normalize(v3dirRay);
		ray_out.set( v3lineStart, v3dirRay, true );

		// トラバースを設定
		PTriangleIntersectLineTraversal lineTraversal;
		Point3 plineEnd = Point3(v4lineEnd.getXYZ());
		lineTraversal.setLine(Point3(v4lineStart.getXYZ()), lineDir, plineEnd);
		lineTraversal.traverseDepthFirst(*root);

		m_intersectedNode = lineTraversal.getNearest();

		if(m_intersectedNode)
		{
			// 三角形が交差しているかどうかチェック
			PMesh *mesh = (PMesh *)m_intersectedNode->getUserData(PSSG_TYPE(PMesh));
			if(mesh && lineTraversal.getTriangleID() < mesh->getTriangleCount())
			{
				const PTriangle *triangles = mesh->getTriangleVertices();
				const PTriangle &triangle = triangles[lineTraversal.getTriangleID()];

				// 頂点はfloat3であると仮定
				PRenderStream *vertexStream = mesh->getStream(mesh->getVertexStreamID());
				if(vertexStream)
				{
					// 頂点データにアクセス - この時点で、メッシュの構造体から任意の頂点データを読み込み可能
					const float *vertices = (const float *)vertexStream->getData();
					if(vertices)
					{
						for (int i = 0; i < 3 ; i++)
						{
							const float *vert = &vertices[triangle[i] * 3];
							v4PickedTri_out[i].setX(vert[0]);
							v4PickedTri_out[i].setY(vert[1]);
							v4PickedTri_out[i].setZ(vert[2]);
							v4PickedTri_out[i].setW(1.0f);
						}
						// 適切な三角形
						bPickedTriInPhyre = true;
						m_intersectedNode->generateGlobalTransform();
					}
				}
			}

		}
		else
		{
			// ユーザはモデル以外の何かをクリックしなければならない
			bPickedTriInPhyre = false;
		}
	} // for(PListIterator scenes(database->getSceneList()) ; scenes ; scenes.next())

	return bPickedTriInPhyre;
}

bool CApplicationMain::InitApplication(char ** argv, int argc)
{
#ifdef APPTARGET_IS_PS3
	m_enableJoyPad = true;
#endif // APPTARGET_IS_PS3

#ifdef MEMORYLEAK_FIND_CRTDBG_WIN32
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	//_CrtSetBreakAlloc(151);		// Output window LeakNumber
#endif // MEMORYLEAK_FIND_CRTDBG_WIN32

	return PApplication::InitApplication(argv, argc);
}

bool CApplicationMain::InitScene()
{
	// 새로운 데이터 베이스 생성 
	m_dbaseID = PSSG::Extra::createExampleDatabase(s_databaseName);

	PDatabaseWriteLock writeLock(m_dbaseID);
	PDatabase *database = writeLock.getDatabase();
	if(!database)
		return PSSG_SET_LAST_ERROR(false, ("Unable to lock sample database"));
	PResult result;

	Vector3		v3Translation( 0.0f, -20.0f, 0.0f );		// Tarrain Mesh들의 위치 이동

	// ルートノードを作成
	PRootNode *nodeRoot = database->createSceneRoot<PRootNode>(PDatabaseUniqueNameHelper(*database, "nodeRoot"), &result);
	if(result != PE_RESULT_NO_ERROR)
		return PSSG_SET_LAST_ERROR(false, ("Unable to create the screen graph nodeRoot node"));
	

	// 게임용 카메라
	m_arCamera[CAMERALIST_GAME] = PSSG::Extra::simpleAddCamera(*nodeRoot, 0.0f, 50.0f, 100.0f);
	if(!m_arCamera[CAMERALIST_GAME])
	{
		return PSSG_SET_LAST_ERROR(false, ("Unable to create the game camera"));
	}
	
	// 디버그 카메라
	m_arCamera[CAMERALIST_DEBUG] = PSSG::Extra::simpleAddCamera(*nodeRoot, 0.0f, 50.0f, 100.0f);
	if(!m_arCamera[CAMERALIST_DEBUG])
	{
		return PSSG_SET_LAST_ERROR(false, ("Unable to create the debug camera"));	
	}

	bool bResult = false;
#ifdef _DEBUG_TERRAINTILE
	// 타일 메쉬 만들기 - 테스트 텍스쳐를 0번 메쉬로 PipeLine에 보낸다.
	bResult = m_terrainTile_test.createTileMesh_Test(	nodeRoot, 
														database, 
														500.0f, 
														500.0f,
														1.0f, 6.f, 10.f,
														48,
														v3Translation );
	if(!bResult)
	{
		return PSSG_SET_LAST_ERROR(false, ("Unable to create TileMesh."));	
	}

	#ifdef _DEBUG_SPACETREE
	m_pQuadTree = m_terrainTile_test.getSpaceTree();
	#endif // _DEBUG_SPACETREE


	#ifdef _DEBUG_NAVIGATION
		m_pnaviManager = m_pnaviManager->getthis();
		m_pnaviManager->Initialize(		m_terrainTile_test.getTriContainer(), 
			m_terrainTile_test.getSpaceTree() );
	#endif // _DEBUG_NAVIGATION

#endif // _DEBUG_TERRAINTILE

#ifdef _DEBUG_TERRAINSHAPE_TEST
	PString	szDirectoryData, szFileNameTerrain;
	szDirectoryData = m_dirWithFilePrefix;
	szDirectoryData += "/SampleData/TerrainTestData/";
	//szDirectoryData += "/SampleData/TerrainTestData/";
	szFileNameTerrain = "pan2.PSSG";

	PLinkResolver::addDirectory(szDirectoryData);

	bResult = m_terrainShape_test.createTerrainShape_test(	nodeRoot, 
															database,
															szDirectoryData+szFileNameTerrain,
															20.0f );
	if(!bResult)
	{
		return PSSG_SET_LAST_ERROR(false, ("Unable to create TileMesh."));
	}

	m_pQuadTree = m_terrainShape_test.getSpaceTree();

	#ifdef _DEBUG_NAVIGATION
		m_pnaviManager = m_pnaviManager->getthis();
		m_pnaviManager->Initialize(	m_terrainShape_test.getTriContainer(), 
									m_terrainShape_test.getSpaceTree() );
	#endif // _DEBUG_NAVIGATION
#endif // _DEBUG_TERRAINSHAPE_TEST
	

#ifdef _DEBUG_TERRAINTEST
	// TerrainMesh 생성
	PString	szDirectoryData;
	szDirectoryData = m_dirWithFilePrefix;
	szDirectoryData += "/SampleData/TerrainTestData_pan/";
	//szDirectoryData += "/SampleData/TerrainTestData/";
	PLinkResolver::addDirectory(szDirectoryData);

	bResult = m_terrainMesh_test.createTerrain_test(	nodeRoot, database,
															m_arCamera,
															(int)CAMERALIST_CNT,
															m_renderInterface, 
															szDirectoryData, 
															"Terrain.PSSG",//"TerrainTest_101227.PSSG",//"pan1_02.PSSG",
															"TerrainShader",
															"TerrainDiffuse.dds",
															"TerrainNormal.dds",
															"TerrainLit.dds" );

	m_pQuadTree = m_terrainMesh_test.getSpaceTree();

	#ifdef _DEBUG_NAVIGATION
		m_pnaviManager = m_pnaviManager->getthis();
		m_pnaviManager->Initialize(	m_terrainMesh_test.getTriContainer(), 
									m_terrainMesh_test.getSpaceTree() );
	#endif // _DEBUG_NAVIGATION
#endif // _DEBUG_TERRAINTEST


	PString	szDirData, szNameData;
	szDirData = m_dirWithFilePrefix;
	szDirData += "/SampleData/TerrainTestData/";
	szNameData = "mutant_test.PSSG";


#if defined(_DEBUG_TERRAINTILE) && defined(_DEBUG_OBJECTS)
	// 모델 오브젝트 생성
	m_Objects.InitializeObjDynamics(	database, 
										nodeRoot, 
										m_terrainTile_test.getSpaceTree(), 
	#ifdef _DEBUG_NAVIGATION
										&(m_pnaviManager->getNaviCells()),
	#endif // _DEBUG_NAVIGATION
										v3Translation,
										szDirData+szNameData,
										OBJ::OBJ_STATE_WALK_SPCTREE,
#ifdef APPTARGET_IS_WIN		// Number of Instance Objects
	#ifdef _DEBUG
										4,
	#else // _DEBUG
										8,
	#endif // _DEBUG
#else // APPTARGET_IS_WIN
	#ifdef _DEBUG
										8,
	#else // _DEBUG
										16,
	#endif // _DEBUG
#endif // APPTARGET_IS_WIN
										30.0f );
#endif // defined(_DEBUG_TERRAINTILE) && defined(_DEBUG_OBJECTS)

#if defined(_DEBUG_TERRAINSHAPE_TEST) && defined(_DEBUG_OBJECTS)
	// 모델 오브젝트 생성
	m_Objects.InitializeObjDynamics(database, 
									nodeRoot, 
									m_terrainShape_test.getSpaceTree(), 
	#ifdef _DEBUG_NAVIGATION
									&(m_pnaviManager->getNaviCells()),
	#endif // _DEBUG_NAVIGATION
									v3Translation, 
									szDirData+szNameData,
									OBJ::OBJ_STATE_WALK_SPCTREE,
#ifdef APPTARGET_IS_WIN		// Number of Instance Objects
									4,
#else // APPTARGET_IS_WIN
									16, 
#endif // APPTARGET_IS_WIN
									10.0f );
#endif // defined(_DEBUG_TERRAINSHAPE_TEST) && defined(_DEBUG_OBJECTS)

#if defined(_DEBUG_TERRAINTEST) && defined(_DEBUG_OBJECTS)
	// 모델 오브젝트 생성
	m_Objects.InitializeObjDynamics(	database, 
									nodeRoot, 
									m_terrainMesh_test.getSpaceTree(), 
#ifdef _DEBUG_NAVIGATION
									&(m_pnaviManager->getNaviCells()),
#endif // _DEBUG_NAVIGATION
									v3Translation, 
									szDirData+szNameData,
									OBJ::OBJ_STATE_WALK_SPCTREE,
#ifdef APPTARGET_IS_WIN		// Number of Instance Objects
									4,
#else // APPTARGET_IS_WIN
									16, 
#endif // APPTARGET_IS_WIN
									10.0f );
#endif // defined(_DEBUG_TERRAINTEST) && defined(_DEBUG_OBJECTS)


// 	// 포인트 광원
// 	PSSG::Extra::simpleAddLight(*nodeRoot, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

	// 방향성 광원
	Vector3 lightPos(0.0f, 500.0f, 0.0f);
	PSSG::PLightNode  *directionalLight = PSSG::Extra::simpleAddLight(
						*nodeRoot, lightPos.getX(), lightPos.getY(), lightPos.getZ(), 1.0f, 1.0f, 1.0f);
	directionalLight->setType(PLightNode::PE_LIGHTTYPE_DIRECTIONAL);
	directionalLight->setLookat(lightPos,Vector3(0),Vector3(0,1,0));


	// For IntersectRay Phyre Traversal line
#ifdef _DEBUG_TRAVERSALINTERSECT_PH
	PNode *root = NULL;
	PTraversalAddIntersectionData addIntersectionData;
	for(PListIterator scenes(database->getSceneList()); scenes ; scenes.next())
	{
		PNode *scene = (PNode *)scenes.data();
		if(!scene)
			continue;

		root = scene;

		// 交差データを追加
		addIntersectionData.traverseDepthFirst(*scene);

 		// 個々のシーンに光源を追加
 		PSSG::Extra::simpleAddLight(*scene, 0.0f, 0.0f, 10.0f, 0.8f, 0.8f, 0.8f);
	}
	
	if (!root)
	{
		return PSSG_SET_LAST_ERROR(false, ("Unable to find root node"));
	}
#endif // _DEBUG_TRAVERSALINTERSECT_PH

	// Debug Texture 초기화
	m_debugingTexture.Init();

#ifdef _DEBUGFOR_RENDER
	// Debug Render 초기화
	m_pdrawForDebug = m_pdrawForDebug->getthis();
	m_pdrawForDebug->Intialize( m_renderInterface );
#endif // _DEBUGFOR_RENDER

	// Output Text Format
	S_PTextFormat		TextFormat;
	TextFormat.Init( "TestTEXT", "Tuffy.ttf", 
		15,
		255, 255, 255, 200, 240, 235,
		-0.95f, 0.94f, -1.0f, 0.05f, 0.0f, 0.0f );

	m_OutputText.Init( database, nodeRoot, TextFormat);

	// @ For TESTDEBUG
#if defined(_DEBUG_SPACETREE) && defined(_DEBUGFOR_RENDER)
	m_bRenderAllTreeNodes_fordebug = false;
#endif // defined(_DEBUG_SPACETREE) && defined(_DEBUGFOR_RENDER)

	m_bpickedTriInTree = false;
	m_bpickedPhyre = false;

#ifdef _DEBUG_CUSTOMTIME
	for( int i=0; i<E_TIME_COUNT; ++i )
	{
		m_arrTimedebug[i].InitTime();
	}
#endif // _DEBUG_CUSTOMTIME
	// @ For TESTDEBUG

	// 데이터 로드 된 상태로 표시.
//	database->setLoaded();
	

	// @ For Control
	m_invertedY				= true;

	return true;
}

bool CApplicationMain::HandleInputs()
{
//	if(isJoypadButtonDown(FWInput::Channel_Button_Circle))
//	{
//	}	

#ifdef APPTARGET_IS_PS3
	if(isJoypadButtonDown(FWInput::Channel_Button_Right))
#else // APPTARGET_IS_PS3
	if( checkAndClearKey(']') )
#endif // APPTARGET_IS_PS3
	{
		m_eCurrentCamera = static_cast<E_CAMERALIST>((m_eCurrentCamera + 1) % static_cast<int>(CAMERALIST_CNT));
	}

#ifdef APPTARGET_IS_PS3
	if(isJoypadButtonDown(FWInput::Channel_Button_Left))
#else // APPTARGET_IS_PS3
	if( checkAndClearKey('[') )
#endif // APPTARGET_IS_PS3
	{
		m_eCurrentCamera = static_cast<E_CAMERALIST>((m_eCurrentCamera - 1)<0?static_cast<int>(CAMERALIST_CNT)-1:(m_eCurrentCamera - 1));
	}

#ifdef APPTARGET_IS_PS3
	if(isJoypadButtonDown(FWInput::Channel_Button_Up))
#else // APPTARGET_IS_PS3
	if( checkAndClearKey(';') )
#endif // APPTARGET_IS_PS3
	{
		m_bOutputDebugInfo = !m_bOutputDebugInfo;
	}

#ifdef APPTARGET_IS_PS3
	if(isJoypadButtonDown(FWInput::Channel_Button_Down))
#else // APPTARGET_IS_PS3
	if(checkAndClearKey('P'))
#endif // APPTARGET_IS_PS3
	{
		m_wireframe = !m_wireframe;
	}

#if defined(_DEBUG_SPACETREE) && defined(_DEBUGFOR_RENDER)
	
#ifdef APPTARGET_IS_PS3
	if(isJoypadButtonDown(FWInput::Channel_Button_Square))
#else // APPTARGET_IS_PS3
	if(checkAndClearKey('G') || checkAndClearKey('g'))
#endif // APPTARGET_IS_PS3
	{
		m_bRenderAllTreeNodes_fordebug = !m_bRenderAllTreeNodes_fordebug;
	}

#ifdef _DEBUG_OBJECTS
	
#endif // _DEBUG_OBJECTS
#endif // defined(_DEBUG_SPACETREE) && defined(_DEBUGFOR_RENDER)


#ifdef _DEBUG_SPACETREE
#ifdef APPTARGET_IS_PS3
	if(isJoypadButtonDown(FWInput::Channel_Button_Cross))
#else // APPTARGET_IS_PS3
	if(checkAndClearKey('1'))
#endif // APPTARGET_IS_PS3
	{
		m_arrTimedebug[E_TIME_CHKPERFORM_ST].MarkThisTime();

#ifdef APPTARGET_IS_WIN
		bool bRayStartReferMouse = true;
#else // APPTARGET_IS_WIN
		bool bRayStartReferMouse = false;
#endif // APPTARGET_IS_WIN

		PCameraNode *cameraActive = m_arCamera[m_eCurrentCamera];
		m_bpickedTriInTree = IntersectCameraRay_ST(	bRayStartReferMouse,
													cameraActive,
													m_rayfordebugTree,
													m_triPickedSpceTree._v4Tri3Points );

		double dblElapsedTime = m_arrTimedebug[E_TIME_CHKPERFORM_ST].GetElapsedTime();
		char	szOutput[512];
		sprintf( szOutput, "SPCTree IntersectTri\t// Picked(%s) // Elapsed Time (%f)\n", m_bpickedTriInTree==true?"TRUE":"FALSE",dblElapsedTime );
#ifdef APPTARGET_IS_WIN
		OutputDebugString( szOutput );
#endif // APPTARGET_IS_WIN
		m_debugingTexture.printText( szOutput );

	} // if(checkAndClearKey('1'))
#endif // _DEBUG_SPACETREE



#ifdef _DEBUG_TRAVERSALINTERSECT_PH

#ifdef APPTARGET_IS_PS3
	if(isJoypadButtonDown(FWInput::Channel_Button_Triangle))
#else // APPTARGET_IS_PS3
	if(checkAndClearKey('2'))
#endif // APPTARGET_IS_PS3
	{
		m_arrTimedebug[E_TIME_CHKPERFORM_ST].MarkThisTime();

#ifdef APPTARGET_IS_WIN
		bool bRayStartReferMouse = true;
#else // APPTARGET_IS_WIN
		bool bRayStartReferMouse = false;
#endif // APPTARGET_IS_WIN
		PCameraNode *cameraActive = m_arCamera[m_eCurrentCamera];
		m_bpickedPhyre = IntersectCameraRay_Traversal(	m_dbaseID, 
														bRayStartReferMouse,
														cameraActive,
														m_rayfordebugPhyre, 
														m_triPickedPhyre._v4Tri3Points );


		double	dblElapsedTime = m_arrTimedebug[E_TIME_CHKPERFORM_ST].GetElapsedTime();
		char	szOutput[512];
		sprintf( szOutput, "Phyre IntersectTri\t// Picked(%s) // Elapsed Time (%f)\n", m_bpickedPhyre==true?"TRUE":"FALSE",dblElapsedTime );
#ifdef APPTARGET_IS_WIN
		OutputDebugString( szOutput );
#endif // APPTARGET_IS_WIN
		m_debugingTexture.printText( szOutput );


	} // if(checkAndClearKey('2'))
#endif // _DEBUG_TRAVERSALINTERSECT_PH


#ifdef _DEBUG_COLLECTPICKING_FORNAVI

#ifdef APPTARGET_IS_PS3
	if(isJoypadButtonDown(FWInput::Channel_Button_Select))
#else // APPTARGET_IS_PS3
	if(checkAndClearKey('M'))
#endif // APPTARGET_IS_PS3
	{
		m_bModeNaviFlipBlocks =! m_bModeNaviFlipBlocks;
	}

#ifdef APPTARGET_IS_PS3
	if(isJoypadButtonDown(FWInput::Channel_Button_L1))
#else // APPTARGET_IS_PS3
	if(KEY_DOWN__('Z'))
#endif // APPTARGET_IS_PS3
	{
		if(true==m_bModeNaviFlipBlocks)
		{
#ifdef APPTARGET_IS_PS3
			bool bRayStartReferMouse = false;
#else // APPTARGET_IS_PS3
			bool bRayStartReferMouse = true;
#endif // APPTARGET_IS_PS3
			PCameraNode *cameraActive = m_arCamera[m_eCurrentCamera];

			m_bFlipNaviBlocks = IntersectRayCollectTris(bRayStartReferMouse,
				cameraActive,
				m_rayfordebugNaviBlocks,
				m_vecTrisPicked_naviBlocks,
				E_MODE_COLLECTNAVY_INSERT );
		}
		else	// if(true==m_bModeNaviFlipBlocks)
		{
#ifdef APPTARGET_IS_PS3
			bool bRayStartReferMouse = false;
#else // APPTARGET_IS_PS3
			bool bRayStartReferMouse = true;
#endif // APPTARGET_IS_PS3
			PCameraNode *cameraActive = m_arCamera[m_eCurrentCamera];

			m_bFlipNaviBlocks = IntersectRayCollectTris(bRayStartReferMouse,
				cameraActive,
				m_rayfordebugNaviGoals,
				m_vecTrisPicked_naviGoals,
				E_MODE_COLLECTNAVY_INSERT );
		}	// if(true==m_bModeNaviFlipBlocks)
	} // if(KEY_DOWN__('Z'))

#ifdef APPTARGET_IS_PS3
	if(isJoypadButtonDown(FWInput::Channel_Button_R1))
#else // APPTARGET_IS_PS3
	if(KEY_DOWN__('X'))
#endif // APPTARGET_IS_PS3
	{
		if(true==m_bModeNaviFlipBlocks)
		{
#ifdef APPTARGET_IS_PS3
			bool bRayStartReferMouse = false;
#else // APPTARGET_IS_PS3
			bool bRayStartReferMouse = true;
#endif // APPTARGET_IS_PS3
			PCameraNode *cameraActive = m_arCamera[m_eCurrentCamera];

			m_bFlipNaviBlocks = IntersectRayCollectTris(bRayStartReferMouse,
				cameraActive,
				m_rayfordebugNaviBlocks,
				m_vecTrisPicked_naviBlocks,
				E_MODE_COLLECTNAVY_DELETE );
		}
		else // if(true==m_bModeNaviFlipBlocks)
		{
#ifdef APPTARGET_IS_PS3
			bool bRayStartReferMouse = false;
#else // APPTARGET_IS_PS3
			bool bRayStartReferMouse = true;
#endif // APPTARGET_IS_PS3
			PCameraNode *cameraActive = m_arCamera[m_eCurrentCamera];

			m_bFlipNaviBlocks = IntersectRayCollectTris(bRayStartReferMouse,
				cameraActive,
				m_rayfordebugNaviGoals,
				m_vecTrisPicked_naviGoals,
				E_MODE_COLLECTNAVY_DELETE );
		} // if(true==m_bModeNaviFlipBlocks)
	} // if(GetKeyState(VK_SHIFT)& 0x80000000)

#ifdef APPTARGET_IS_PS3
	if(isJoypadButtonDown(FWInput::Channel_Button_Circle))
#else // APPTARGET_IS_PS3
	if(checkAndClearKey(VK_RETURN))
#endif // APPTARGET_IS_PS3
	{
#ifdef APPTARGET_IS_WIN
		if(IDYES==MessageBox( NULL, "Do construct Navigation mesh?", "Select", MB_YESNO ))
#endif // APPTARGET_IS_WIN
		{
			PSSG_PRINTF("Construct & Mapping Navigation mesh");
			m_arrTimedebug[E_TIME_CHKPERFORM_ST].MarkThisTime();

			set<int> slistCellGoals, slistCellBlocks;

			vector<int>::iterator iter_, iterBegin, iterEnd;
			iterBegin = m_vecTrisPicked_naviGoals.begin();
			iterEnd = m_vecTrisPicked_naviGoals.end();
			for( iter_=iterBegin; iter_!=iterEnd; ++iter_ )
			{
				slistCellGoals.insert((*iter_));
			}

			iterBegin = m_vecTrisPicked_naviBlocks.begin();
			iterEnd = m_vecTrisPicked_naviBlocks.end();
			for( iter_=iterBegin; iter_!=iterEnd; ++iter_ )
			{
				slistCellBlocks.insert((*iter_));
			}

			m_pnaviManager->BuildNewNavi(  slistCellGoals, slistCellBlocks );

			double	dblElapsedTime = m_arrTimedebug[E_TIME_CHKPERFORM_ST].GetElapsedTime();
			char	szOutput[512];
			sprintf( szOutput, "\tComplete construct Navi // ElapsedTime(%f)//LevelofCells(%d)\n", 
								dblElapsedTime, 
								m_pnaviManager->getLevelofCells() );
			PSSG_PRINTF(szOutput);
		}
	}

#ifdef APPTARGET_IS_PS3
	if(isJoypadButtonDown(FWInput::Channel_Button_L2))
#else // APPTARGET_IS_PS3
	if(checkAndClearKey('4'))
#endif // APPTARGET_IS_PS3
	{
#ifdef _DEBUG_OBJECTS
		m_Objects.SetStatus_ObjDyn(OBJ::OBJ_STATE_WALK_SPCTREE);
#endif // _DEBUG_OBJECTS
		m_pnaviManager->setClearNavi();

		m_vecTrisPicked_naviBlocks.clear();
		m_vecTrisPicked_naviGoals.clear();
	}

#ifdef _DEBUG_OBJECTS
#ifdef APPTARGET_IS_PS3
	if(isJoypadButtonDown(FWInput::Channel_Button_R2))
#else // APPTARGET_IS_PS3
	if(checkAndClearKey('5'))
#endif // APPTARGET_IS_PS3
	{
		if(false==m_Objects.SetStatus_ObjDyn_switching())
		{
			char	szOutput[512];
			
			sprintf( szOutput, "Before switching Status Object, do the Set GOAL First!" );
#ifdef APPTARGET_IS_WIN
			MessageBox(NULL, szOutput, "Confirm", MB_OK);
#else // APPTARGET_IS_WIN
			PSSG_PRINTF( szOutput );
#endif // APPTARGET_IS_WIN
		}
	}
#endif // _DEBUG_OBJECTS

#endif // _DEBUG_COLLECTPICKING_FORNAVI


	if( m_arCamera[m_eCurrentCamera] )
	{
		float fMovementVel = 0.0f;
#ifdef _DEBUG_TERRAINTILE
		fMovementVel = 50.0f;
#endif // _DEBUG_TERRAINTILE

#ifdef _DEBUG_TERRAINSHAPE_TEST
		fMovementVel = 10.0f;
#endif // _DEBUG_TERRAINSHAPE_TEST

#ifdef _DEBUG_TERRAINTEST
		fMovementVel = 5.0f;
#endif // _DEBUG_TERRAINTEST
		HandleInputs_CurrentCamera(*m_arCamera[m_eCurrentCamera], fMovementVel, 3.0f, 50.0f);
	}

	return PApplication::HandleInputs();
}

void CApplicationMain::HandleInputs_CurrentCamera(PSSG::PNode &node, float moveScale /* = 1.0f */, float rotateScale /* = 1.0f */, float perpendicularScale /* = 1.0f */)
{

	if(	(m_leftMouse && (m_mouseOffsetX || m_mouseOffsetY)))
	{
		PSSG::Extra::rotateNode(node, 
			rotateScale * .02f * static_cast<float>(m_elapsedTime) * m_mouseOffsetX, 
			rotateScale * .02f * static_cast<float>(m_elapsedTime) * m_mouseOffsetY);
	}
	
	const float fVelocityCameraMovement = 10.0f;

	if(isKeyDown('W') || isKeyDown('w'))
	{
		PSSG::Extra::moveNodeForward(node, (static_cast<float>(m_elapsedTime)*fVelocityCameraMovement*moveScale) );
	}

	if(isKeyDown('S') || isKeyDown('s'))
	{
		PSSG::Extra::moveNodeForward(node, -(static_cast<float>(m_elapsedTime)*fVelocityCameraMovement*moveScale) );
	}

	if(isKeyDown('A') || isKeyDown('a'))
	{
		PSSG::Extra::moveNodePerpendicular(node, -(static_cast<float>(m_elapsedTime)*fVelocityCameraMovement*moveScale),0);
	}

	if(isKeyDown('D') || isKeyDown('d'))
	{
		PSSG::Extra::moveNodePerpendicular(node, (static_cast<float>(m_elapsedTime)*fVelocityCameraMovement*moveScale),0);
	}

	if(m_middleMouse && (m_mouseOffsetX || m_mouseOffsetY))
	{
		PSSG::Extra::moveNodePerpendicular(node, m_mouseOffsetX * perpendicularScale * 0.01f, m_mouseOffsetY * perpendicularScale * -0.01f);
	}

	float leftX, leftY, rightX, rightY;

	if(!m_enableJoyPad)
		return;

	// Left handed flag exchanges left and right sticks.
	if (m_leftHanded)
	{
		leftX = getJoypadStickPosition(JOYPAD_STICKAXIS_RIGHT_X);
		leftY = getJoypadStickPosition(JOYPAD_STICKAXIS_RIGHT_Y);
		rightX = getJoypadStickPosition(JOYPAD_STICKAXIS_LEFT_X);
		rightY = getJoypadStickPosition(JOYPAD_STICKAXIS_LEFT_Y);
	}
	else
	{
		leftX = getJoypadStickPosition(JOYPAD_STICKAXIS_LEFT_X);
		leftY = getJoypadStickPosition(JOYPAD_STICKAXIS_LEFT_Y);
		rightX = getJoypadStickPosition(JOYPAD_STICKAXIS_RIGHT_X);
		rightY = getJoypadStickPosition(JOYPAD_STICKAXIS_RIGHT_Y);
	}

	float felapsedTime = (float)m_elapsedTime;

	if((rightX != 0) || (rightY != 0))
	{
		float verticalScale = m_invertedY ? (-1.5f*felapsedTime) : (1.5f*felapsedTime);
		PSSG::Extra::rotateNode(node, rotateScale * (-1.5f*felapsedTime) * rightX, rotateScale * verticalScale * rightY);
	}

	if(leftY != 0)
		PSSG::Extra::moveNodeForward(node, -10.f * leftY * moveScale * felapsedTime);

	if(leftX != 0)
		PSSG::Extra::moveNodePerpendicular(node, leftX * 10.f * moveScale * felapsedTime, 0.0f);
}

bool CApplicationMain::Animate()
{
#ifdef _DEBUG_OBJECTS

#ifdef _DEBUG_CUSTOMTIME_UPDATEOBJECTS
	m_arrTimedebug[E_TIME_UPDATEOBJ].MarkThisTime();
#endif // _DEBUG_CUSTOMTIME_UPDATEOBJECTS

	m_Objects.UpdateObj( m_elapsedTime );

#ifdef _DEBUG_CUSTOMTIME_UPDATEOBJECTS
	double	dblElapsedTime = m_arrTimedebug[E_TIME_UPDATEOBJ].GetElapsedTime();

	if(true==m_arrTimerdebug[E_TIME_UPDATEOBJ].getIntervalTick(m_elapsedTime))
	{
		char	szOutput[512];
		sprintf( szOutput, "Update Objects\t// Elapsed Time (%f)\n",
			dblElapsedTime );

		PSSG_PRINTF(szOutput);
#ifdef APPTARGET_IS_WIN
		OutputDebugString( szOutput );
#endif // APPTARGET_IS_WIN
	}
#endif // _DEBUG_CUSTOMTIME_UPDATEOBJECTS

#endif // _DEBUG_OBJECTS
	return PApplication::Animate();
}

bool CApplicationMain::Render()
{
#ifdef _DEBUG_TERRAINTEST
 	m_terrainMesh_test.RenderPre( m_arCamera[m_eCurrentCamera] );
#endif // _DEBUG_TERRAINTEST

	if(m_renderInterface)
	{
#ifdef _DEBUG_TERRAINSHAPE_TEST
		m_renderInterface->setClearColor(0.6f,0.6f,0.6f,0);
#else // _DEBUG_TERRAINSHAPE_TEST
		m_renderInterface->setClearColor(0.3f,0.3f,0.3f,0);
#endif // _DEBUG_TERRAINSHAPE_TEST

		// Clear screen
		m_renderInterface->beginScene(PRenderInterface::PE_CLEAR_COLOR_BUFFER_BIT | PRenderInterface::PE_CLEAR_DEPTH_BUFFER_BIT);

		// Render the database. Use the more expensive renderDatabaseWithPortals() since the database may use portals.

		if(m_dbaseID)
		{
			PSSG::Extra::renderDatabaseWithPortals(
			m_dbaseID,							// ID of the database to render
			*m_renderInterface,					// Render interface to be used for rendering
			m_width, m_height,					// Dimensions of the viewport
			~0U,								// Render all passes
			m_arCamera[m_eCurrentCamera],		// camera to render from.
			NULL);								// Use the default context
		}

		// Render Debug Info
		if( true == m_bOutputDebugInfo )
		{
			m_renderInterface->setBlending(true,PShader::PE_SHADER_BLEND_ONE,PShader::PE_SHADER_BLEND_SRC_ALPHA);
			m_renderInterface->debugTexture(*(m_debugingTexture.m_debugTexture),0.0125f,0.9611f,0.8125f,0.25f);
			m_renderInterface->setBlending(false,PShader::PE_SHADER_BLEND_ONE,PShader::PE_SHADER_BLEND_ONE);
		}

		// Render Debug 3D
		if( true ==  m_bpickedTriInTree)
		{
			Vector3 v3ColorTri( 0.9f, 0.5f, 0.5f );
			RenderPickedTri(m_triPickedSpceTree._v4Tri3Points, m_arCamera[m_eCurrentCamera],v3ColorTri);
			
#ifdef _DEBUGFOR_RENDER
			Vector3	v3Start, v3End, v3ColorLine;
			v3ColorLine.setX(0.9f);		// R
			v3ColorLine.setY(0.5f);		// G
			v3ColorLine.setZ(0.0f);		// B

			v3Start = m_rayfordebugTree._v3Origin;
			v3End	= m_rayfordebugTree._v3Origin + (m_rayfordebugTree._v3Direction * 1000.0f);

			m_pdrawForDebug->renderLine( v3Start, v3End, v3ColorLine, &(m_arCamera[m_eCurrentCamera]->m_viewProjectionMatrix));
#endif // _DEBUGFOR_RENDER
		}

#ifdef _DEBUG_TRAVERSALINTERSECT_PH
		if( true == m_bpickedPhyre)
		{
			Vector3 v3ColorTri( 1.0f, 0.9f, 0.5f );
			RenderPickedTri(m_triPickedPhyre._v4Tri3Points, m_arCamera[m_eCurrentCamera],v3ColorTri);

#ifdef _DEBUGFOR_RENDER
			Vector3	v3Start, v3End, v3ColorLine;
			v3ColorLine.setX(0.9f);		// R
			v3ColorLine.setY(0.5f);		// G
			v3ColorLine.setZ(0.5f);		// B

			v3Start = m_rayfordebugPhyre._v3Origin;
			v3End	= m_rayfordebugPhyre._v3Origin + (m_rayfordebugPhyre._v3Direction * 1000.0f);

			m_pdrawForDebug->renderLine(v3Start, v3End, v3ColorLine, &(m_arCamera[m_eCurrentCamera]->m_viewProjectionMatrix));
#endif // _DEBUGFOR_RENDER
		}
#endif // _DEBUG_TRAVERSALINTERSECT_PH

#ifdef _DEBUG_COLLECTPICKING_FORNAVI
	if(m_vecTrisPicked_naviBlocks.size()>0)
	{
		Vector3 v3ColorTri( 0.1f, 0.1f, 0.95f );
		RenderSelectedTris( m_vecTrisPicked_naviBlocks, m_arCamera[m_eCurrentCamera], v3ColorTri );
	}

	if(m_vecTrisPicked_naviGoals.size()>0)
	{
		Vector3 v3ColorTri( 1.0f, 0.0f, 0.0f );
		RenderSelectedTris( m_vecTrisPicked_naviGoals, m_arCamera[m_eCurrentCamera], v3ColorTri );
	}
#endif // _DEBUG_COLLECTPICKING_FORNAVI
		
#if defined(_DEBUG_SPACETREE) && defined(_DEBUGFOR_RENDER)
		if(true==m_bRenderAllTreeNodes_fordebug)
		{
			Vector3	v3Color;

			v3Color.setX( 1.f );
			v3Color.setY( 1.0f );
			v3Color.setZ( 1.0f );
#ifdef _DEBUG_TERRAINTILE
			m_terrainTile_test.renderspacetree_fordebug( v3Color, &(m_arCamera[m_eCurrentCamera]->m_viewProjectionMatrix) );
#endif // _DEBUG_TERRAINTILE

#ifdef _DEBUG_TERRAINSHAPE_TEST
			m_terrainShape_test.renderspacetree_fordebug( v3Color, &(m_arCamera[m_eCurrentCamera]->m_viewProjectionMatrix) );
#endif // _DEBUG_TERRAINSHAPE_TEST

#ifdef _DEBUG_TERRAINTEST
			m_terrainMesh_test.renderspacetree_fordebug( v3Color, &(m_arCamera[m_eCurrentCamera]->m_viewProjectionMatrix) );
#endif // _DEBUG_TERRAINTEST

		}
#endif // defined(_DEBUG_SPACETREE) && defined(_DEBUGFOR_RENDER)

		// Polygon Fill
		m_renderInterface->setPolygonFill(m_wireframe ? PShader::PE_SHADER_FILL_LINE : PShader::PE_SHADER_FILL_SOLID);

		// Complete the scene
		m_renderInterface->endScene();
	}

	RenderUsefulValues_debug();
	return true;
}

void CApplicationMain::RenderPickedTri(	const Vector4 *arTriPicked, 
										const PCameraNode *pActiveNodeCamera, 
										const Vector3 &v3Color )
{
	const Matrix4 localToClip = pActiveNodeCamera->m_viewProjectionMatrix;// * intersectedNode->m_globalMatrix;

	Vector4 color;
	vectorCopy3to4( color, v3Color, 1.0f );
	color.setW(1.0f);
	m_renderInterface->debugDraw(PRenderInterface::PE_PRIMITIVE_TRIANGLES, arTriPicked, color, NULL, 3, &localToClip);
}

#ifdef _DEBUG_SPACETREE
void CApplicationMain::RenderSelectedTris(	const vector<int> &vecTrisPicked,
											const PCameraNode *pActiveCamera,
											const Vector3 &v3Color )
{
	if( vecTrisPicked.size()<1 ) return;

	const Matrix4 localToClip = pActiveCamera->m_viewProjectionMatrix;// * intersectedNode->m_globalMatrix;

	Vector4 color;
	vectorCopy3to4( color, v3Color, 1.0f );
	color.setW(1.0f);

	Vector4 v4Tri[3];
	int iIdxTri = 0;
	for( int i=0; i<(int)(vecTrisPicked.size()); ++i )
	{
		iIdxTri = vecTrisPicked[i];
		CTri *pTriCurr = m_pQuadTree->getTri( iIdxTri );

		for(int iP=0; iP<3; ++iP)
		{
			vectorCopy3to4(v4Tri[iP],pTriCurr->v3PT[iP],1.0f);
		}

		m_renderInterface->debugDraw(PRenderInterface::PE_PRIMITIVE_TRIANGLES, v4Tri, color, NULL, 3, &localToClip);
	}
}
#endif // _DEBUG_SPACETREE

void CApplicationMain::RenderUsefulValues_debug()
{	
	unsigned int uiBuf_=0;
	static unsigned long ulTickCnt;
	unsigned long ulTickCntCurrent;
	double dElapsedTimeSummary;
	char	szOutputText[1024];


	++ulTickCnt;

	if(true==m_arrTimerdebug[E_TIME_DEBUG_RENDER].getIntervalTick(m_elapsedTime))
	{
		ulTickCntCurrent = ulTickCnt;
		ulTickCnt = 0;
		dElapsedTimeSummary = 0.0f;	

		uiBuf_ = uiBuf_+sprintf( uiBuf_+szOutputText, "1.FPS:%2d", (int)ulTickCntCurrent );		

#ifdef _DEBUG_NAVIGATION
		if(true==m_bModeNaviFlipBlocks)
		{
			uiBuf_ = uiBuf_+sprintf( uiBuf_+szOutputText, "  /  2.MODE_NAVI:BLOCKS" );
		}
		else
		{
			uiBuf_ = uiBuf_+sprintf( uiBuf_+szOutputText, "  /  2.MODE_NAVI:GOALS" );
		}

		uiBuf_ = uiBuf_+sprintf( uiBuf_+szOutputText, "  /  3.BUILDED_NAVI:%s", 
										true==m_pnaviManager->m_NaviCells_basement.didBuildupGoal()?"TRUE ":"FALSE" );

	#ifdef _DEBUG_OBJECTS
		char	szStatusObj[32];
		{
			switch(m_Objects.GetStatus_ObjDyn())
			{
			case OBJ::OBJ_STATE_IDLE:
				{
					sprintf( szStatusObj, "IDLE" );
				}
				break;
			case OBJ::OBJ_STATE_WALK_SPCTREE:
				{
					sprintf( szStatusObj, "WALK_SPCTREE_RAND" );
				}
				break;
			case OBJ::OBJ_STATE_WALK_NAVI:
				{
					sprintf( szStatusObj, "WALK_NAVIGATION" );
				}
				break;
			default:
				{
					sprintf( szStatusObj, "NONE" );
				}
				break;
			}
		}

		uiBuf_ = uiBuf_+sprintf( uiBuf_+szOutputText, "  /  4.STATUS_OBJECT:%s", szStatusObj );		
	#endif // _DEBUG_OBJECTS

#endif // _DEBUG_NAVIGATION

#ifdef _DEBUG_OBJECTS
		unsigned int uiCntObjs=m_Objects.getCntofObjsTotal();
		uiBuf_ = uiBuf_+sprintf( uiBuf_+szOutputText, "  /  5.CNT_OBJECTS:%2d", (int)uiCntObjs );

#endif // _DEBUG_OBJECTS

		m_OutputText.RenderText( szOutputText );
	} // if( dElapsedTimeSummary >= 1.0f || 0 == ulTickCnt )
}


bool CApplicationMain::ExitScene()
{
	PSSG::PSSGResetPrintfCallback();

#ifdef _DEBUG_OBJECTS
	m_Objects.Release();
#endif // _DEBUG_OBJECTS

	if(m_meshSegmentSet)
		PSSG::Extra::deleteSegmentSet(*m_meshSegmentSet);

#ifdef _DEBUG_TERRAINTILE
	m_terrainTile_test.Release();
#endif // #ifdef _DEBUG_TERRAINTILE

#ifdef _DEBUG_TERRAINSHAPE_TEST
	m_terrainShape_test.Release();
#endif // _DEBUG_TERRAINSHAPE_TEST

#ifdef _DEBUG_TERRAINTEST
	m_terrainMesh_test.Release();
#endif // _DEBUG_TERRAINTEST

#ifdef _DEBUGFOR_RENDER
	m_pdrawForDebug->destroyInstance();
#endif // _DEBUGFOR_RENDER

#ifdef _DEBUG_COLLECTPICKING_FORNAVI
	if(m_vecTrisPicked_naviBlocks.size()>0)
	{
		m_vecTrisPicked_naviBlocks.clear();
	}

	if(m_vecTrisPicked_naviGoals.size()>0)
	{
		m_vecTrisPicked_naviGoals.clear();
	}
#endif // _DEBUG_COLLECTPICKING_FORNAVI

	// 親クラスを呼び出す
	return PApplication::ExitScene();
}


bool CApplicationMain::ExitApplication()
{
	
	return PApplication::ExitApplication();

	bool bRsltAppExit = PApplication::ExitApplication();

#ifdef MEMORYLEAK_FIND_CRTDBG_WIN32
	_CrtDumpMemoryLeaks();
#endif // MEMORYLEAK_FIND_CRTDBG_WIN32

	return bRsltAppExit;
}

