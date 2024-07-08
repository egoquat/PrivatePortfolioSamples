#include "ProcessorObjects.h"

#ifdef _DEBUG_OBJECTS
using namespace OBJ;

bool CProcessorObjects::setLandingDirectly(	CObjectDynamic *&pObj, bool bDirAdjust_forNAVI )
{
	BV::_Ray		rayObj;
	const Vector3	v3DirDown(0.0f,-1.0f,0.0f), v3DirUp(0.0f,1.0f,0.0f);
	Vector3			v3PosCurrent ,v3PosNextEst, v3PosTarget;
	bool			bCollisionVertical = false;
	const float		fAdjust = ADJUST_OBJRAY_POS;
	float			fDistIntersected = 0.0f;

	pObj->getPosObj(v3PosCurrent);

	v3PosNextEst = v3PosCurrent + (v3DirUp*fAdjust);

	rayObj.set( v3PosNextEst, v3DirDown );

	int iTriIntersected = -1;
	bCollisionVertical = m_pSpcTree->rayIntersectGetIdxTri( rayObj, iTriIntersected );

	if(true==bCollisionVertical)
	{
		CTri	*pTriIntersected = m_pSpcTree->getTri(iTriIntersected);

		Vector3	*arv3TriPnt = pTriIntersected->v3PT;

		BV::IntersectRayTri(	rayObj,
								arv3TriPnt[0],arv3TriPnt[1],arv3TriPnt[2], 
								&fDistIntersected );
		BV::getPosIntersectedRay(	rayObj._v3Origin,
									rayObj._v3Direction,
									fDistIntersected,
									v3PosTarget );

		pObj->setLandingTri(iTriIntersected);
		pObj->setPosObj(v3PosTarget);

#ifdef _DEBUG_NAVIGATION

		if(false==m_pNavi->getCell(iTriIntersected).IsBlockCell())
		{
			if(true==bDirAdjust_forNAVI)
			{
				if(m_pNavi->isGoalCell(iTriIntersected)==true)
				{
					return false;
				}

				if(m_pNavi->didBuildupGoal()==false)
				{
					return false;
				}

				Vector3 v3PosBestWay, v3DirObj;
				m_pNavi->getPosBestWayOnCell( iTriIntersected, v3PosBestWay );

				v3DirObj = normalize(v3PosBestWay-v3PosTarget);
				pObj->setDirection(v3DirObj);
			}
		}
#endif // _DEBUG_NAVIGATION
	}
	else
	{
		return false;
	}

	return true;
}

void CProcessorObjects::updateAllObjDyn_useSpcTree(const double &dblElapsedTime,
										vector<CObject*> &vecObjects )
{
	for(unsigned int iModel=0; iModel<vecObjects.size();++iModel)
	{
		CObjectDynamic	*pObjD = (CObjectDynamic *)(vecObjects[iModel]);

		updateObjDyn_useSpcTree(dblElapsedTime, pObjD);
	}
}

void CProcessorObjects::updateObjDyn_useSpcTree(const double
												
												&dblElapsedTime,
												CObjectDynamic *pObj )
{
	BV::_Ray		rayObj;
	const Vector3	v3DirDown(0.0f,-1.0f,0.0f), v3DirUp(0.0f,1.0f,0.0f);
	Vector3			v3PosCurrent ,v3PosNextEst;
	bool			bOnLand=true;
	const float		fAdjust = ADJUST_OBJRAY_POS;

	pObj->getPosObj(v3PosCurrent);

	if(NULL_TRI==pObj->getLandingTri())
	{
		bOnLand=false;
	}
	else
	{
 		int iTriIntersected = -1;
		bool bIntersectedTri=false;
 		v3PosNextEst = v3PosCurrent + (v3DirUp*fAdjust);
 		rayObj.set( v3PosNextEst, v3DirDown );
 
 		bIntersectedTri=m_pSpcTree->rayIntersectGetIdxTri( rayObj,iTriIntersected );
		if( true==bIntersectedTri )
		{
			if( iTriIntersected!=pObj->getLandingTri() )
			{
				Vector3		v3PosRevise;
				Vector3		*parTriPos;
				float		fDistIntersect;

				pObj->setLandingTri(iTriIntersected);
				parTriPos=m_pSpcTree->getTri(iTriIntersected)->v3PT;

				bIntersectedTri=BV::IntersectRayTri( rayObj, parTriPos[0], parTriPos[1], parTriPos[2], &fDistIntersect );
				if(true==bIntersectedTri)
				{
					BV::getPosIntersectedRay( rayObj._v3Origin, rayObj._v3Direction, fDistIntersect, v3PosRevise );
					pObj->setPosObj(v3PosRevise);

					v3PosNextEst = v3PosRevise + 
						(pObj->getDirBasement()*(pObj->getSpeedPerSec()*((float)dblElapsedTime/1.0f)));

					v3PosNextEst = v3PosNextEst + (v3DirUp*fAdjust);
					rayObj.set( v3PosNextEst, v3DirDown );

					Vector3 v3PosNext, v3DirObj;
					bOnLand=m_pSpcTree->rayIntersectGetPos( rayObj, v3PosNext );
					v3DirObj=normalize(v3PosNext-v3PosRevise);
					pObj->setDirection( v3DirObj );
				}
				else
				{
					bOnLand=false;
				}
			} // if( iTriIntersected!=pObj->getLandingTri() )
		} // if( true==bIntersectedTri )
		else
		{
			bOnLand=false;
		} // if( true==bIntersectedTri )

	}

	if(false==bOnLand)
	{
		pObj->UpdateObjectFalling(dblElapsedTime);
	}
	else
	{
		pObj->UpdateObject(dblElapsedTime);
	}
}

#ifdef _DEBUG_NAVIGATION
void CProcessorObjects::updateAllObjDyn_useNAVI(const double &dblElapsedTime,
												vector<CObject*> &vecObjects )
{
	for(unsigned int iModel=0; iModel<vecObjects.size();++iModel)
	{
		CObjectDynamic	*pObjD = (CObjectDynamic *)(vecObjects[iModel]);

		updateObjDyn_useNAVI(dblElapsedTime, pObjD);
	}
}

void CProcessorObjects::updateObjDyn_useNAVI(	const double &dblElapsedTime,
												CObjectDynamic *pObj )
{
	BV::_Ray		rayObj;
	const Vector3	v3DirDown(0.0f,-1.0f,0.0f), v3DirUp(0.0f,1.0f,0.0f);
	Vector3			v3PosCurrent ,v3PosNextEst;
	bool			bOnLand=true;
	const float		fAdjust = ADJUST_OBJRAY_POS;

	pObj->getPosObj(v3PosCurrent);

	if(NULL_TRI==pObj->getLandingTri())
	{
		bOnLand=false;
	}
	else
	{
		int iTriIntersected = -1;
		bool bIntersectedTri=false;
		v3PosNextEst = v3PosCurrent + (v3DirUp*fAdjust);
		rayObj.set( v3PosNextEst, v3DirDown );

		bIntersectedTri=m_pSpcTree->rayIntersectGetIdxTri( rayObj,iTriIntersected );

		if( true==bIntersectedTri )
		{
			if(true==m_pNavi->getCell(iTriIntersected).IsGoalCell())
			{
				pObj->setReachtheGoal(true);
			}
			else
			{
				if( iTriIntersected!=pObj->getLandingTri() )
				{
					Vector3		v3PosRevise, v3PosBestWay, v3DirObj;
					Vector3		*parTriPos;
					float		fDistIntersect;

					pObj->setLandingTri(iTriIntersected);
					parTriPos=m_pSpcTree->getTri(iTriIntersected)->v3PT;

					bIntersectedTri=BV::IntersectRayTri( rayObj, parTriPos[0], parTriPos[1], parTriPos[2], &fDistIntersect );
					if(true==bIntersectedTri)
					{
						BV::getPosIntersectedRay( rayObj._v3Origin, rayObj._v3Direction, fDistIntersect, v3PosRevise );
						pObj->setPosObj(v3PosRevise);

						m_pNavi->getPosBestWayOnCell( iTriIntersected, v3PosBestWay );

						v3DirObj=normalize(v3PosBestWay-v3PosRevise);
						pObj->setDirection( v3DirObj );
					}
					else
					{
						bOnLand=false;
					}
				} // if( iTriIntersected!=pObj->getLandingTri() )
			} // if(true==m_pNavi->getCell(iTriIntersected).IsGoalCell())

		} // if( true==bIntersectedTri )
		else
		{
			bOnLand=false;
		} // if( true==bIntersectedTri )
	}

	if(false==bOnLand)
	{
		pObj->UpdateObjectFalling(dblElapsedTime);
	}
	else
	{
		pObj->UpdateObject(dblElapsedTime);
	}
}
#endif // _DEBUG_NAVIGATION

void CProcessorObjects::updateObjDyn_test(	const double &dblElapsedTime,
											CObjectDynamic *pObj )
{
	Vector3			v3PosCurrent ,v3PosCurrentEst, v3PositionNext;

	BV::_Ray		rayObj;
	Vector3			v3PosLand, v3DirDown(0.0f,-1.0f,0.0f), v3DirUp(0.0f,1.0f,0.0f);
	Vector3			v3PosNextCal ,v3PosNextEst;
	bool			bCollisionVertical = false;
	const float		fAdjust = ADJUST_OBJRAY_POS;

	pObj->getPosObj(v3PosCurrent);

	v3PosNextEst	= v3PosCurrent + 
		( pObj->getDirection()*(pObj->getSpeedPerSec()*((float)dblElapsedTime/1.0f)) );
	v3PosNextEst = v3PosNextEst + (v3DirUp*fAdjust);

	rayObj.set( v3PosNextEst, v3DirDown );
	bCollisionVertical = m_pSpcTree->rayIntersectGetPos( rayObj, v3PosNextCal );

	if(false==bCollisionVertical)
	{
		const float		fG = ACCELERATOR_G*10.0f;
	
		v3PosNextCal	= v3PosCurrent + 
			( v3DirDown*(fG*((float)dblElapsedTime/1.0f)) );
	}
	
	pObj->setPosObj(v3PosNextCal);
}

void CProcessorObjects::updateAllObjStatic(	const double &dblElapsedTime,
											VEC_OBJECT &vecObjects )
{
	for(unsigned int iModel=0; iModel<vecObjects.size();++iModel)
	{
		CObjectDynamic	*pObjD = (CObjectDynamic *)(vecObjects[iModel]);
		updateObjStatic(dblElapsedTime, pObjD);
	}
}


void CProcessorObjects::updateObjStatic(	const double &dblElapsedTime,
											CObjectDynamic *pObj )
{
	pObj->UpdateObject(dblElapsedTime);
}


void CProcessorObjects::Release()
{

}


CProcessorObjects::CProcessorObjects(void)
{
#ifdef _DEBUG_NAVIGATION
	m_pNavi = NULL;
#endif // _DEBUG_NAVIGATION
	m_pSpcTree = NULL;
}

CProcessorObjects::~CProcessorObjects(void)
{
}

#endif // _DEBUG_OBJECTS
