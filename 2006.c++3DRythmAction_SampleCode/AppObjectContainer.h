//=============================================================================================================
//	�ۼ���		:	06/05/19
//	Ŭ������	:	AppObjectContainer
//	�ۼ�����	:	        Object �� Container�� ���� �մϴ�.
//					�ܺο����� �� Container�� ���� Object ������ ����մϴ�..
//=============================================================================================================

#pragma once

class AppObjectBasis;
struct	ALPHAOBJECT	// ī�޶�-���ΰ� �þ߸� ������ ĳ���͸� ����ó���ϱ� ���� ����ü
{
	float				fDist;
	AppObjectBasis*		pObject;
};



#include <vector>

#include "AppObjectBasis.h"
#include "AppObjectHero.h"
#include "AppObjectNpc.h"

#include "AppCharacter.h"
#pragma comment(lib, "../datalib/AppCharacter.lib")

#ifdef APPOBJECT_EXPORTS
#define APPOBJECTCONTAINER_DLL __declspec(dllexport)
#else
#define APPOBJECTCONTAINER_DLL __declspec(dllimport)
#endif

class APPOBJECTCONTAINER_DLL AppObjectContainer
{
// --------------------------------------------------------------- ��ü �ν��Ͻ�
public:
	static	AppObjectContainer*			m_pObjectContainer;
    static	AppObjectContainer*			GetThis();
	void								DestroyInstance();
// --------------------------------------------------------------- ��ü �ν��Ͻ�

public:
	UINT64*								m_Container;

	AppCharacter*						m_pChar;

	int									m_iReserveCnt;
	int									m_iCurrent;

	D3DXVECTOR3							m_vCameraPos;
	D3DXVECTOR3							m_vHeroPos;

public:

	void			Add(	float fPositionX, float fPositionY, float fPositionZ, float fScaleValue,
							APP_DIRECTION_INDEX iDirection,
							APP_OBJECT_ACTION_INDEX iAniIndex,
							APP_NPCTYPE_INDEX iNpcType,
							APP_AI_INDEX iAIType,
							APP_OBJECT_INDEX iObjectIndex = OBJECT_NULL);

	void			DeleteAllContainer()
	{
		if (m_Container == 0) return;
		if (m_iCurrent <= 0 || m_iReserveCnt <= 0) return;

		for (int i = 0; i < m_iReserveCnt; i++)
		{
			DeleteObject(i);
		}
		
		if(m_Container != NULL)
		{
			delete[] m_Container;
			m_Container = 0;
		}

		m_iReserveCnt	= 0;

		m_iCurrent		= 0;
	}

	void			DeleteObject(int iIndex)
	{
		if (m_Container[iIndex] == 0) return;

		AppObjectBasis*		pObject;
		pObject = (AppObjectBasis*)(m_Container[iIndex]);

		if (pObject != 0)
		{
			pObject->Release();
			delete pObject;
		}
		
        m_Container[iIndex] = 0;
		pObject = 0; 
	}

	void			SetReserveCount(int iReserveCnt)
	{
		if(m_Container != 0)
		{
			delete[] m_Container;
			m_Container = 0;
		}

		m_Container		= new UINT64[iReserveCnt];

		m_iReserveCnt	= iReserveCnt;

		m_iCurrent		= 0;
	}

	AppObjectBasis*	GetObject(int iIdx)
	{
		return (AppObjectBasis*)(m_Container[iIdx]);
	}

	AppObjectBasis*	FindTargetNpc();

public:
	bool			Init();
	bool			Frame();
	bool			RenderShadow();
	bool			Render();
	bool			RenderAlpha();
	bool			Release();

public:
	AppObjectContainer(void);
	~AppObjectContainer(void);

private:

	std::vector<ALPHAOBJECT>	m_vecAlphaObject;
};
