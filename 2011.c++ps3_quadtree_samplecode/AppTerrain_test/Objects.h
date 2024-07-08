#pragma once

#include "CommonApp.h"

#ifdef _DEBUG_OBJECTS

#include "Object.h"
#include "TerrainMesh.h"

#include "ProcessorObjects.h"
#include "NaviCells.h"

namespace OBJ
{
#define DEFAULT_MUTANTCNT				16
#define DEFAULT_SECTURNINGPOINT		8

#define DEFAULT_DIST_TARGET			20.f

	enum	E_STATE_OBJECT
	{
		OBJ_STATE_NULL=-1,
		OBJ_STATE_IDLE,
		OBJ_STATE_WALK_SPCTREE,
		OBJ_STATE_WALK_NAVI,
		OBJ_STATE_END,
	};

	struct S_TARGET_AREA
	{
		Vector3		_v3PosTarget;
		int			_iTriSeq;
	};

typedef vector<CObject*>	VEC_OBJECT;

	struct S_OBJ_RESOURCE
	{
		PSSG::PDatabaseID		_DatabaseID;
		PSSG::PNode				*_pBNode_RenderRsc;
		Vector3					_v3SizeObj;

		void setObjRsc( PSSG::PDatabaseID DatabaseID_,
						PSSG::PNode *pBNode_RenderRsc_,
						Vector3 &v3SizeObj_ )
					{
						_DatabaseID=DatabaseID_;
						_pBNode_RenderRsc=pBNode_RenderRsc_;
						memcpy( &_v3SizeObj,&v3SizeObj_,sizeof(Vector3) );
					};
	};

	class CObjects
	{
	public:
		// @ Container
		S_OBJ_RESOURCE			m_arObjResource[TYPEOBJ_DYN_END];
		VEC_OBJECT				m_arrObjRenderInstance[OBJTYPE_END];
		vector<S_TARGET_AREA>	m_vecTrisTargetMove_forSpcTree;
		
		// @ Process
		PSSG::PRootNode			*m_pnodeRoot;
		CProcessorObjects		*m_pProcessorObj;
		E_STATE_OBJECT			m_eStateObj;
		bool					m_bLandingProcessForObjDynamic;
		CSpaceTree				*m_pTerrainSpaceTree;
		PDatabase				*m_pDatabaseScene;

	private:
		// @ Get / Set
		int	getRandSingle(const int iNumLmit, const int iSingleNum)
		{
			int iRandNum=0;
			do 
			{
				iRandNum=rand()%iNumLmit;
			} while(iRandNum==iSingleNum);

			return iRandNum;
		}

		void setTargettoOBJ(	TRIPART::CTri &triCurr_in, 
								const int iTriSeqTarget_in,
								S_TARGET_AREA &targetArea_out, 
								CObjectDynamic *&pobjDynamic_out )
		{
			Vector3		v3PosTriCntr, v3PosObj, v3DirBasement;
			triCurr_in.GetCenterTri(v3PosTriCntr);

			pobjDynamic_out->getPosObj(v3PosObj);
			v3PosObj.setY(v3PosTriCntr.getY());
			v3DirBasement = normalize(v3PosTriCntr-v3PosObj);


			targetArea_out._iTriSeq = iTriSeqTarget_in;
			targetArea_out._v3PosTarget=v3PosTriCntr;

			pobjDynamic_out->setDirBasement( v3DirBasement );
		}

	public:
		// @ Get / Set
		unsigned int getCntofObjsTotal()
		{
			unsigned int uiSumObjs=0;

			for(int iTypeObj=0; iTypeObj<OBJTYPE_END; ++iTypeObj)
			{
				uiSumObjs=+m_arrObjRenderInstance[iTypeObj].size();
			}

			return uiSumObjs;
		}

		// @ Function
#ifdef _DEBUG_NAVIGATION
		bool SetStatus_ObjDyn_switching();
		void SetStatus_ObjDyn(	E_STATE_OBJECT eStateObj	);

		E_STATE_OBJECT GetStatus_ObjDyn(){	return m_eStateObj;};
#endif // _DEBUG_NAVIGATION

	public:
		// @ Process
		void resetObjDynamics_forSpcTree(	VEC_OBJECT &vecObjDynamic,
											CProcessorObjects *pProcessorObj,
											vector<S_TARGET_AREA> &vecTrisTargetMove_forSpcTree,
											CTriContainer *triCollected );

		void resetObjDynamics_forNAVI(	VEC_OBJECT &vecObjDynamic,
										CProcessorObjects *pProcessorObj );

		void InsertNewObjDyn(	const E_TYPE_OBJDYNAMIC eTypeObj_in,
								Vector3 &v3Position_in,
								const Vector3 &v3Dir_in,
								const float fSpeedAnimation_in,
								const float fSpeedPerSec_in );

		void deleteObjDynAll();
		void deleteObjDynTypeAll(	const E_TYPE_OBJDYNAMIC eTypeObj_in );

		void batchNewObjsDynType_test(	const E_TYPE_OBJDYNAMIC eTypeObj_in,
										const int iCntObj,
										const Vector3 &v3Position_in,
										const Vector3 &v3Dir_in,
										const float fSpeedAnimation_in,
										const float fSpeedPerSec_in );


		// @ Process
		bool InitializeObjDynamics(	PDatabase *databaseScene,
									PSSG::PRootNode *pnodeRoot, 
									CSpaceTree *pTerrainSpaceTree,
		#ifdef _DEBUG_NAVIGATION
									NAVIPART::CNaviCells *pNavi,
		#endif // _DEBUG_NAVIGATION
									const Vector3 &v3TransObj,
									PString szFileName_path,
									E_STATE_OBJECT eStateObj=OBJ_STATE_WALK_SPCTREE,
									unsigned int uiMutantCntPerType = DEFAULT_MUTANTCNT,
									float fSpeedObjAnimation = DEFAULT_SPEEDANIMATION,
									float fObjectMovementSpeedperSec = DEFAULT_SPEEDMOVEMENT_PERSEC );

		void UpdateObj(	double &dElapsedTime );
		void UpdateTest(	double &dElapsedTime );

		void UpdateObjsDynamic(	VEC_OBJECT &vecObjects, const double &dElapsedTime );
		void UpdateObjsStatic(	VEC_OBJECT &vecObjects, const double &dElapsedTime );

		// 	void Render(		PSSG::PRenderInterface *renderInterface, 
		// 		int width, 
		// 		int height, 
		// 		PSSG::PCameraNode *nodeCamera );

		void Release();

	public:
		CObjects()
		{
			m_eStateObj = OBJ_STATE_WALK_SPCTREE;
			m_pTerrainSpaceTree = NULL;
			m_pDatabaseScene=NULL;
			m_pnodeRoot=NULL;
		};
		~CObjects()
		{
			m_eStateObj = OBJ_STATE_WALK_SPCTREE;
			m_pTerrainSpaceTree = NULL;
			m_pDatabaseScene=NULL;
			m_pnodeRoot=NULL;
		};
	};

};

#endif // _DEBUG_OBJECTS
