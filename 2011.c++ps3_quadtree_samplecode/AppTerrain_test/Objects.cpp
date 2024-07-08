#include "Objects.h"

#ifdef _DEBUG_OBJECTS
using namespace OBJ;

#ifdef _DEBUG_NAVIGATION
bool CObjects::SetStatus_ObjDyn_switching()
{
	E_STATE_OBJECT eStatusObjCur = (E_STATE_OBJECT)(((int)m_eStateObj+1)%(int)(OBJ_STATE_END));

#ifdef _DEBUG
	if(false==m_pProcessorObj->m_pNavi->didBuildupGoal())
	{
		return false;
	}
#else // _DEBUG
	if( OBJ_STATE_WALK_NAVI==eStatusObjCur
		&& false==m_pProcessorObj->m_pNavi->didBuildupGoal())
	{
		eStatusObjCur = (E_STATE_OBJECT)(((int)m_eStateObj+1)%(int)(OBJ_STATE_END));
	}
#endif // _DEBUG

	SetStatus_ObjDyn( eStatusObjCur );

	return true;
}

void CObjects::SetStatus_ObjDyn(	E_STATE_OBJECT eStateObj	)
{
	m_eStateObj = eStateObj;

	switch (eStateObj)
	{
	case OBJ_STATE_WALK_SPCTREE:
		{
			resetObjDynamics_forSpcTree(	m_arrObjRenderInstance[OBJTYPE_DYNAMIC], 
											m_pProcessorObj, 
											m_vecTrisTargetMove_forSpcTree,
											m_pTerrainSpaceTree->m_pTriContainer );
		}
		break;
	case OBJ_STATE_WALK_NAVI:
		{
			resetObjDynamics_forNAVI(	m_arrObjRenderInstance[OBJTYPE_DYNAMIC],
										m_pProcessorObj );
		}
		break;
	default:
		{

		}
		break;
	}
}
#endif // _DEBUG_NAVIGATION

void CObjects::resetObjDynamics_forSpcTree(	VEC_OBJECT &vecObjDynamic,
										   CProcessorObjects *pProcessorObj,
										   vector<S_TARGET_AREA> &vecTrisTargetMove_forSpcTree,
										   CTriContainer *triCollected )
{
	int iSizeofTris = triCollected->getCountTries();

	if(vecTrisTargetMove_forSpcTree.size()>0)
	{
		vecTrisTargetMove_forSpcTree.clear();
	}

	vecTrisTargetMove_forSpcTree.resize( vecObjDynamic.size() );

	VEC_OBJECT::iterator	iter_, iterBegin_, iterEnd_;
	int iSeq = 0;
	iterBegin_ = vecObjDynamic.begin();	iterEnd_ = vecObjDynamic.end();
	for( iter_=iterBegin_; iter_!=iterEnd_; ++iter_, ++iSeq)
	{
		CObjectDynamic *pobjDynamic = (CObjectDynamic *)(*iter_);

		//		if( pobjDynamic->getLandingTri()==NULL_TRI )
		{
			bool bLanding = pProcessorObj->setLandingDirectly( pobjDynamic );
#ifdef _DEBUG
			if(false==bLanding)
			{
				PSSG_PRINTF("Object has not currently positioning on land!");
			}
#endif // _DEBUG
			if(true==bLanding)
			{
				int iTriTargetRand = getRandSingle( iSizeofTris, pobjDynamic->getLandingTri() );
				TRIPART::CTri &triCurr=triCollected->getTri( iTriTargetRand);
				setTargettoOBJ(	triCurr,
					iTriTargetRand,
					m_vecTrisTargetMove_forSpcTree[iSeq],
					pobjDynamic	);
			}
		} // if( pobjDynamic->getLandingTri()==NULL_TRI )
	} // for( iter_=iterBegin_; iter_!=iterEnd_; ++iter_, ++iSeq)
}

void CObjects::resetObjDynamics_forNAVI(	VEC_OBJECT &vecObjDynamic,
										CProcessorObjects *pProcessorObj )
{
	VEC_OBJECT::iterator	iter_, iterBegin_, iterEnd_;
	int iSeq = 0;
	iterBegin_ = vecObjDynamic.begin();	iterEnd_ = vecObjDynamic.end();
	for( iter_=iterBegin_; iter_!=iterEnd_; ++iter_, ++iSeq)
	{
		CObjectDynamic *pobjDynamic = (CObjectDynamic *)(*iter_);

		//		if( pobjDynamic->getLandingTri()==NULL_TRI )
		{
#ifdef _DEBUG
			bool bLanding = pProcessorObj->setLandingDirectly( pobjDynamic, true );

			if(false==bLanding)
			{
				PSSG_PRINTF("Object has not currently positioning on land!");
			}
#else // _DEBUG
			pProcessorObj->setLandingDirectly( pobjDynamic, true );
#endif // _DEBUG
		} // if( pobjDynamic->getLandingTri()==NULL_TRI )
	} // for( iter_=iterBegin_; iter_!=iterEnd_; ++iter_, ++iSeq)
}


void CObjects::InsertNewObjDyn(	const E_TYPE_OBJDYNAMIC eTypeObj_in,
								Vector3 &v3Position_in,
								const Vector3 &v3Dir_in,
								const float fSpeedAnimation_in,
								const float fSpeedPerSec_in )
{
	PResult result;
	PNode* pBNode_rsc=m_arObjResource[eTypeObj_in]._pBNode_RenderRsc;
 	PNode *pmodelNode_clone = (PNode*)(pBNode_rsc->clone(m_pDatabaseScene, &result));

	// Object Property 설정
	OBJ::CObjectDynamic	*pObjDyn = new CObjectDynamic();
	pObjDyn->setPropertyDyn(	OBJ::OBJTYPE_DYNAMIC,
								eTypeObj_in,
								pmodelNode_clone,
								v3Position_in,
								v3Dir_in,
								fSpeedAnimation_in,
								fSpeedPerSec_in );

	m_arrObjRenderInstance[OBJTYPE_DYNAMIC].push_back(pObjDyn);
	m_pnodeRoot->addChild(*pmodelNode_clone);
}

void CObjects::deleteObjDynAll()
{

}

void CObjects::deleteObjDynTypeAll(	const E_TYPE_OBJDYNAMIC eTypeObj_in )
{

}

void CObjects::batchNewObjsDynType_test(	const E_TYPE_OBJDYNAMIC eTypeObj_in,
										   const int iCntObj_in,
										   const Vector3 &v3Pos_in,
										   const Vector3 &v3Dir_in,
										   const float fSpeedAnimation_in,
										   const float fSpeedPerSec_in )
{
	if(iCntObj_in<1)
		return;

	Vector3	v3MinArea, v3MaxArea;
	float	fOffset=0.0f, fSizeOffsetX=0.0f, fSizeOffsetY=0.0f;
	int		iCntEdge=0, iSeqObj_=0;
	bool	bLoopingDone=false;
	
	iCntEdge=(int)(sqrt((float)iCntObj_in));
	(iCntEdge==0)?1:iCntEdge;

	S_OBJ_RESOURCE	objRsc=m_arObjResource[eTypeObj_in];
	Vector3	v3SizeObj=objRsc._v3SizeObj, v3Pos=v3Pos_in;

	fOffset=((v3SizeObj.getX()+v3SizeObj.getY())*0.5f)*0.2f;
	fSizeOffsetX=v3SizeObj.getX()+fOffset;
	fSizeOffsetY=v3SizeObj.getY()+fOffset;

	for(int iUnit=0;false==bLoopingDone ;iUnit=iUnit+2)
	{
		int iRowcurr, iColcurr;
		for(int iRow=0;iRow<=iUnit&&false==bLoopingDone;++iRow)
		{
			iRowcurr=iRow-(iUnit/2);
			for(int iCol=0;iCol<=iUnit&&false==bLoopingDone;++iCol)
			{
				iColcurr=iCol-(iUnit/2);

				v3Pos.setX(iRowcurr*fSizeOffsetX);
				v3Pos.setY(iColcurr*fSizeOffsetY);

				InsertNewObjDyn( eTypeObj_in, v3Pos, v3Dir_in, fSpeedAnimation_in, fSpeedPerSec_in );
				if(++iSeqObj_>=iCntObj_in)
				{
					bLoopingDone=true;
				}

			} // for(int iCol=0;iCol<=iUnit;++iCol)

		} // for(int iRow=0;iRow<=iUnit;++iRow)

	} // for(int iUnit=0; ;iUnit=iUnit+2)

}


bool CObjects::InitializeObjDynamics(	PDatabase *databaseScene,
										PSSG::PRootNode *pnodeRoot, 
										CSpaceTree *pTerrainSpaceTree,
#ifdef _DEBUG_NAVIGATION
										NAVIPART::CNaviCells *pNavi,
#endif // _DEBUG_NAVIGATIONs
										const Vector3 &v3TransObj,
										PString szFileName_path,
										E_STATE_OBJECT eStateObj,
										unsigned int uiMutantCntPerType,
										float fSpeedAnimation,
										float fSpeedobjMovePerSec )
{
	PResult result;

	Vector3		v3DirBasement(1.0f, 0.0f, 0.f),
				v3ZeroVector(0.f, 0.f, 0.f),
				v3TargetPosition(0.f,0.f,0.f),
				v3DirResult(0.f,0.f,0.f);

	m_eStateObj=eStateObj;
	m_pDatabaseScene=databaseScene;
	m_pnodeRoot=pnodeRoot;

	VEC_OBJECT &vecObjDynamic = m_arrObjRenderInstance[OBJTYPE_DYNAMIC];
	vecObjDynamic.reserve(TYPEOBJ_DYN_END*uiMutantCntPerType);

	for( int iModel=0; iModel<TYPEOBJ_DYN_END; ++iModel )
	{
		PSSG::PDatabaseID		databaseIDObj_;

		if(PLinkResolver::getDatabase(databaseIDObj_, szFileName_path)!=PE_RESULT_NO_ERROR)
		{
			return PSSG_SET_LAST_ERROR(false,("Unable to get the default database for this sample."));
		}

		if(PSSG::Extra::resolveAllLinks()!=PE_RESULT_NO_ERROR)
		{
			return PSSG_SET_LAST_ERROR(false,("Unable to resolve all outstanding links"));
		}

		E_TYPE_OBJDYNAMIC	eTypeObjDyn=(E_TYPE_OBJDYNAMIC)iModel;

		// Database File을 읽어온다.
		PDatabaseWriteLock modelWriteLock(databaseIDObj_);
		PDatabase *pmodelDatabase = modelWriteLock.getDatabase();
		if(!pmodelDatabase)
		{
			return PSSG_SET_LAST_ERROR(false,("Unable to lock model database"));
		}

		PListIterator modelScenes(pmodelDatabase->getSceneList());
		PNode *pmodelRoot = (PNode*)modelScenes.data();

		// Bundle Node : 즉 Model Node를 뽑아낸다.
		PTraversalFindNodeByType findBUndleNode(PSSG_TYPE(PBundleNode));
		findBUndleNode.traverseDepthFirst(*pmodelRoot);
		PNode *pBundleNodeObj = (PNode*)findBUndleNode.getFoundNode();
		if(!pBundleNodeObj)
		{
			return PSSG_SET_LAST_ERROR(false, ("Unable to clone the model node bacause model has not bundle node."));
		}

		PTraversalFindNodeByType findRenderNode(PSSG_TYPE(PRenderNode));
		findRenderNode.traverseDepthFirst(*pmodelRoot);
		PNode *prenderNode = (PNode*)findRenderNode.getFoundNode();
		if(!prenderNode)
		{
			return PSSG_SET_LAST_ERROR(false, ("Unable to clone the model node because model has not rendernode."));
		}
		Vector3 objectSize(0.0f);
		objectSize = prenderNode->m_boundsMax - prenderNode->m_boundsMin;

		m_arObjResource[(int)eTypeObjDyn].setObjRsc(databaseIDObj_,pBundleNodeObj,objectSize );
	} // for( int iModel=0; iModel<TYPEOBJ_DYN_END; ++iModel )


	m_pProcessorObj = m_pProcessorObj->getthis();

#if defined(_DEBUG_SPACETREE) && defined(_DEBUG_NAVIGATION)
	m_pProcessorObj->Initialize(	pTerrainSpaceTree, pNavi );
#else
#ifdef _DEBUG_SPACETREE
	m_pProcessorObj->Initialize(	pTerrainSpaceTree );
#endif // _DEBUG_SPACETREE
#ifdef _DEBUG_NAVIGATION
	m_pProcessorObj->Initialize(	pNavi );
#endif // _DEBUG_NAVIGATION
#endif // defined(_DEBUG_SPACETREE) && defined()_DEBUG_NAVIGATION)

	m_pTerrainSpaceTree = pTerrainSpaceTree;

	batchNewObjsDynType_test( TYPEOBJ_DYN_A, 
								16, 
								v3TransObj, 
								v3DirBasement, 
								fSpeedAnimation, 
								fSpeedobjMovePerSec );

	SetStatus_ObjDyn(	eStateObj );

	resetObjDynamics_forSpcTree(	vecObjDynamic, 
									m_pProcessorObj,
									m_vecTrisTargetMove_forSpcTree, 
									m_pTerrainSpaceTree->m_pTriContainer );

	return true;
}

void CObjects::UpdateObj(double &dElapsedTime)
{
	UpdateTest(dElapsedTime);
}


void CObjects::UpdateTest(double &dElapsedTime)
{
	UpdateObjsDynamic( m_arrObjRenderInstance[OBJTYPE_DYNAMIC], dElapsedTime );
	UpdateObjsStatic( m_arrObjRenderInstance[OBJTYPE_STATIC], dElapsedTime);
}


void CObjects::UpdateObjsDynamic(	VEC_OBJECT &vecObjects, const double &dElapsedTime)
{
	if(vecObjects.size()<1)
	{
		return;
	}

	Vector3 v3Position(0.0f), v3DirToTarget(0.0f), v3Dir(0.0f);
	switch(m_eStateObj)
	{
	case OBJ_STATE_IDLE:
		{
			
		}
		break;	
	case OBJ_STATE_WALK_SPCTREE:
		{
			int			iCntTris = m_pTerrainSpaceTree->m_pTriContainer->getCountTries();
			bool		bPositioningInTarget = false;
			float		fdistTarget=0.0f;
			Vector3		v3PosObj, v3PosTarget;
			CObjectDynamic *pObjDynamic = NULL;

			for(unsigned int i=0;i<m_vecTrisTargetMove_forSpcTree.size();++i)
			{
				pObjDynamic = static_cast<CObjectDynamic*>(vecObjects[i]);

				pObjDynamic->getPosObj( v3PosObj );
				v3PosTarget = m_vecTrisTargetMove_forSpcTree[i]._v3PosTarget;

				fdistTarget=length(v3PosTarget-v3PosObj);
#ifdef _DEBUG
				if(0.0f>fdistTarget)
				{
					assert__(false, __FILE__, __LINE__, __FUNCTION__, "");
				}
#endif // _DEBUG

				bPositioningInTarget = fdistTarget < DEFAULT_DIST_TARGET;
				if(true==bPositioningInTarget)
				{
					int iTriTargetNext = getRandSingle( iCntTris, pObjDynamic->getLandingTri() );
					TRIPART::CTri	&triTarget = m_pTerrainSpaceTree->m_pTriContainer->getTri(iTriTargetNext);

					setTargettoOBJ(	triTarget, 
									iTriTargetNext, 
									m_vecTrisTargetMove_forSpcTree[i],
									pObjDynamic);
				}
			}

			m_pProcessorObj->updateAllObjDyn_useSpcTree( dElapsedTime, vecObjects );
		}
		break;
	case OBJ_STATE_WALK_NAVI:
		{
#ifdef _DEBUG_NAVIGATION
			m_pProcessorObj->updateAllObjDyn_useNAVI( dElapsedTime, vecObjects );
#endif // _DEBUG_NAVIGATION
		}
		break;
	default:
		{

		}
		break;
	} // switch(m_eStateObj)
}


void CObjects::UpdateObjsStatic(	VEC_OBJECT &vecObjects, const double &dElapsedTime )
{
	if(vecObjects.size()<1)
	{
		return;
	}

	for(unsigned int i=0;i<vecObjects.size();++i)
	{
		
	}

	m_pProcessorObj->updateAllObjStatic( dElapsedTime, vecObjects );
}

void CObjects::Release()
{
	for(unsigned int ivecObj=0;ivecObj<OBJTYPE_END;++ivecObj)
	{
		VEC_OBJECT	&vecObj = m_arrObjRenderInstance[ivecObj];

		for( int iObj=0; iObj<(int)vecObj.size(); ++iObj )
		{
			CObject *pcurrObj = vecObj[iObj];
			SAFEDELETE(pcurrObj);
		}
	}

	if(m_vecTrisTargetMove_forSpcTree.size()>0)
	{
		m_vecTrisTargetMove_forSpcTree.clear();
	}

	m_pProcessorObj->destroyInstance();
}

#endif // _DEBUG_OBJECTS

