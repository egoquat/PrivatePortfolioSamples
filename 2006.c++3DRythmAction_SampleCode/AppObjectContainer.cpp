#include ".\appobjectcontainer.h"
#include "../datainclude/map/appmap.h"
#include "../datainclude/map/ProjectiveShadow.h"
#pragma comment (lib, "../datalib/appmap.lib")
#pragma comment (lib, "../appmap/mapbase/model.lib")

AppObjectContainer*			AppObjectContainer::m_pObjectContainer = 0;

AppObjectContainer*			AppObjectContainer::GetThis()
{
	if (m_pObjectContainer == 0) m_pObjectContainer = new AppObjectContainer();
	return m_pObjectContainer;
}

void						AppObjectContainer::DestroyInstance()
{
	if (m_pObjectContainer != 0) 
	{
		delete m_pObjectContainer;
	}
	m_pObjectContainer = 0;
}

void			AppObjectContainer::Add(	float fPositionX, float fPositionY, float fPositionZ, float fScaleValue,
											APP_DIRECTION_INDEX iDirection,
											APP_OBJECT_ACTION_INDEX iAniIndex,
											APP_NPCTYPE_INDEX iNpcType,
											APP_AI_INDEX iAIType,
											APP_OBJECT_INDEX iObjectIndex)
{
	AppObjectHero*						pObjectHero;
	AppObjectNpc*						pObjectNpc;

	AppCharObj*							pChar;

	if (m_iCurrent >= m_iReserveCnt) 
	{
		MessageBox(NULL, "추가되려는 오브젝트 값은 현재 예약된 오브젝트값 범위를 초과 합니다.", NULL, NULL);
		return;
	}

	switch (iObjectIndex)
	{
	case OBJECT_HERO:
	case OBJECT_HEROINE:
		{
			pChar	= m_pChar->GetChar(iObjectIndex);

			pObjectHero = new AppObjectHero(	fPositionX, fPositionY,fPositionZ, fScaleValue,
				iDirection, iAniIndex, pChar,m_iCurrent, iNpcType, iObjectIndex);

			m_Container[m_iCurrent] = (INT64)pObjectHero;
			break;
		}
	case OBJECT_NPC_GUARDRED:
	case OBJECT_NPC_GUARDBLUE:
	case OBJECT_NPC_FORK:
	case OBJECT_NPC_BAKER:
	case OBJECT_NPC_FLYPIG:
	case OBJECT_NPC_QUEEN:
	case OBJECT_NPC_CAT:
	case OBJECT_NPC_HAMPTY:
	case OBJECT_NPC_CULTBEAR:
	case OBJECT_NPC_PUMPKINBEAR:
	case OBJECT_NPC_WITCH:
	case OBJECT_NPC_ELIZABETH:
	case OBJECT_NPC_SEVASTIAN:
		{
			pObjectNpc = new AppObjectNpc(		fPositionX, fPositionY,fPositionZ, fScaleValue,
				iDirection, iAniIndex, m_pChar->GetChar(iObjectIndex),m_iCurrent, iNpcType, iAIType, iObjectIndex );
			m_Container[m_iCurrent] = (INT64)pObjectNpc;
			break;
		}
	case OBJECT_NPC_HERO:
		{
			pObjectNpc = new AppObjectNpc(		fPositionX, fPositionY,fPositionZ, fScaleValue,
				iDirection, iAniIndex, m_pChar->GetChar(OBJECT_HERO),m_iCurrent, iNpcType, iAIType, OBJECT_NPC_HERO );
			m_Container[m_iCurrent] = (INT64)pObjectNpc;
			break;
		}
	case OBJECT_NPC_HEROINE:
		{
			pObjectNpc = new AppObjectNpc(		fPositionX, fPositionY,fPositionZ, fScaleValue,
				iDirection, iAniIndex, m_pChar->GetChar(OBJECT_HEROINE),m_iCurrent, iNpcType, iAIType, OBJECT_NPC_HEROINE );
			m_Container[m_iCurrent] = (INT64)pObjectNpc;
			break;
		}
	default:
		{
			return;
		}
	}

	m_iCurrent++;
}

bool	AppObjectContainer::Init()
{	
	SET_INSTANCE(m_pChar);
	return true;
}

struct PredAlphaObjSort : binary_function<ALPHAOBJECT,ALPHAOBJECT,bool> {
	bool operator() (const ALPHAOBJECT& x, const ALPHAOBJECT& y) const
	{
		return x.fDist > y.fDist;
	}
};

bool	AppObjectContainer::Frame()
{
	if (m_iReserveCnt <= 0 || m_iCurrent < 1) return true;

	AppMap* pMap = AppMap::GetThis();
	m_vCameraPos = *pMap->GetCameraPos();
	m_vHeroPos = *pMap->GetHeroPos();

	const float h = 5.f;
	AABB		aabb;	// 캐릭터 AABB
	bool		bHit;
	D3DXVECTOR3* vPos;
	D3DXVECTOR3	vHitCoord;
	float fDist;
	D3DXVECTOR3	vDirCameraToHero = m_vHeroPos - m_vCameraPos;
	float		fRayDist = D3DXVec3Length(&vDirCameraToHero);
	D3DXVec3Normalize(&vDirCameraToHero, &vDirCameraToHero);

	m_vecAlphaObject.clear();
	for (int iObj = 0; iObj < m_iReserveCnt; iObj++)
	{
		if (m_Container[iObj]!= 0)
		{
			if (iObj==0 || ((AppObjectBasis*)m_Container[iObj])->FrustumCull(&((AppObjectBasis*)m_Container[0])->m_vPos))	// Cull Test 통과시 true
			{
				((AppObjectBasis*)m_Container[iObj])->Frame();
				if (iObj == 0)
					continue;

				// 캐릭터가 카메라-주인공 사이에서 시야를 가리는지를 판정, 알파 처리
				vPos = &((AppObjectBasis*)m_Container[iObj])->m_vPos;
				// 대강의 캐릭터 AABB를 정한다
				aabb.vmin.x = vPos->x - h;
				aabb.vmin.y = 0;
				aabb.vmin.z = vPos->z - h;
				aabb.vmax.x = vPos->x + h;
				aabb.vmax.y = h * 5;
				aabb.vmax.z = vPos->z + h;

				bHit = HitBoundingBox(aabb.vmin, aabb.vmax, m_vCameraPos, vDirCameraToHero, vHitCoord);
				fDist = D3DXVec3Length(&(m_vCameraPos - vHitCoord));
				// 알파 처리할 캐릭터는 따로 벡터에 모아넣는다
				if (bHit && fDist < fRayDist - 10)
				{
					ALPHAOBJECT alphaobj;
                    alphaobj.fDist = fDist;
					alphaobj.pObject = (AppObjectBasis*)m_Container[iObj];
					((AppObjectBasis*)m_Container[iObj])->m_fCharAlphaValue = 0.2f;
					m_vecAlphaObject.push_back(alphaobj);
				}
				else
				{
					((AppObjectBasis*)m_Container[iObj])->m_fCharAlphaValue = 1;
				}
			}
		}
	}

	// 거리순 정렬
	std::sort(m_vecAlphaObject.begin(), m_vecAlphaObject.end(), PredAlphaObjSort());

	return true;
}

bool	AppObjectContainer::RenderShadow()
{
	if (m_iReserveCnt <= 0 || m_iCurrent < 1) return true;

	FProjectiveShadow* pProjShadow = AppMap::GetThis()->GetProjShadow();
	pProjShadow->BeginAddShadow(D3DCOLOR_ARGB(70, 0, 0, 0));

	for (int iObj = 0; iObj < m_iReserveCnt; iObj++)
	{
		if (m_Container[iObj]!= 0) 
		{
			if (((AppObjectBasis*)m_Container[iObj])->m_bVisible)
				((AppObjectBasis*)m_Container[iObj])->RenderShadow();
		}
	}

	pProjShadow->EndShadow();

	return true;
}


bool	AppObjectContainer::Render()
{
	if (m_iReserveCnt <= 0 || m_iCurrent < 1) return true;

	for (int iObj = 0; iObj < m_iReserveCnt; iObj++)
	{
		if (m_Container[iObj]!= 0) 
		{
			if (((AppObjectBasis*)m_Container[iObj])->m_bVisible && ((AppObjectBasis*)m_Container[iObj])->m_fCharAlphaValue == 1)
			{
				((AppObjectBasis*)m_Container[iObj])->Render();
			}
		}
	}

	return true;
}

bool	AppObjectContainer::RenderAlpha()
{
	for (int i=0; i<(int)m_vecAlphaObject.size(); ++i)
	{
		m_vecAlphaObject[i].pObject->Render();
	}
	return true;
}

bool	AppObjectContainer::Release()
{	
	if (m_iReserveCnt < 1) return true;

	DeleteAllContainer();

	if (m_Container == 0) return true;

	delete [] m_Container;

	m_Container = NULL;

	return true;
}

AppObjectContainer::AppObjectContainer(void)
{
	m_iReserveCnt = 0;
	m_Container = 0;

	m_vecAlphaObject.reserve(100);
}

AppObjectContainer::~AppObjectContainer(void)
{
}


AppObjectBasis* AppObjectContainer::FindTargetNpc()
{
	for (int i=0; i<m_iCurrent; ++i)
	{
		if (GetObject(i)->m_nNpcType == NPCTYPE_TARGET)
			return GetObject(i);
	}
	return NULL;
}