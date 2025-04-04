/* SCE CONFIDENTIAL
PhyreEngine(TM) Package 2.6.0
* Copyright (C) 2009 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/
 
//
// PSSG Create Meshサンプル
//

#include <PSSG.h>
#include <Framework/PSSGFramework.h>
#include <Extra/PSSGExtra.h>

#include "CommonApp.h"
#include "CommonSingleton.h"
#include "TerrainMesh.h"
#include "TerrainTest.h"

#include "DebugingTexture.h"
#include "OutputText.h"

#include "Objects.h"

#include "BoundaryVolume.h"
#include "DrawForDebug.h"

#include "Srcex/PSSGIntersectionTraversals.h"
#include "CustomTime.h"

#include <PSSGMesh/PSSGMesh.h>
#include <PSSGIntersectMesh/PSSGIntersectMesh.h>
#include <PSSGUtilTraversal/PSSGUtilTraversal.h>

#ifdef _DEBUG_NAVIGATION
#include "NaviManager.h"
#endif // _DEBUG_NAVIGATION

using namespace Vectormath::Aos;
using namespace PSSG;

// 解説：
// サンプルアプリケーションの定義
class CApplicationMain : public PApplication
{
#ifdef _DEBUG_CUSTOMTIME
enum	E_TYPE_TIMEDEBUG
{
	E_TIME_CHKPERFORM_ST = 0,
	E_TIME_CHKPERFORM_INTSCT_PH,
	E_TIME_INPUTCNTRL,
	E_TIME_UPDATEOBJ,
	E_TIME_DEBUG_RENDER,
	E_TIME_COUNT,
};
#endif // _DEBUG_CUSTOMTIME

enum	E_MODE_COLLECTNAVI
{
	E_MODE_COLLECTNAVY_INSERT=0,
	E_MODE_COLLECTNAVY_DELETE,
};

struct STRIPICKED
{
	int			_iIdxTri;
	Vector4		_v4Tri3Points[3];

	bool operator < (const STRIPICKED &rhs)
	{
		return (_iIdxTri<rhs._iIdxTri);
	};
};



private:
	static const char	*s_databaseName;	
#ifdef _DEBUG_TERRAINTILE
	CTerrainTile		m_terrainTile_test;
#endif // #ifdef _DEBUG_TERRAINTILE

#ifdef _DEBUG_TERRAINTEST
	CTerrainTest		m_terrainMesh_test;
#endif // _DEBUG_TERRAINTEST

#ifdef _DEBUG_TERRAINSHAPE_TEST
	CTerrainShapeForSpaceTree
						m_terrainShape_test;
#endif // _DEBUG_TERRAINSHAPE_TEST

#ifdef _DEBUG_NAVIGATION
	NAVIPART::CNaviManager
						*m_pnaviManager;
#endif // _DEBUG_NAVIGATION

#ifdef _DEBUG_SPACETREE
	CSpaceTree			*m_pQuadTree;
#endif // _DEBUG_SPACETREE

	PCameraNode*		m_arCamera[CAMERALIST_CNT];
	E_CAMERALIST		m_eCurrentCamera;			// 현재 카메라 인덱스

	bool				m_bOutputDebugInfo;
	CDebugingTexture	m_debugingTexture;

	COutputText			m_OutputText;

	PSegmentSet			*m_meshSegmentSet;
	bool				m_wireframe;	

#ifdef _DEBUG_CUSTOMTIME
	CCustomTime			m_arrTimedebug[E_TIME_COUNT];
	CCustomTimer		m_arrTimerdebug[E_TIME_COUNT];
#endif // _DEBUG_CUSTOMTIME

#ifdef _DEBUG_OBJECTS
	OBJ::CObjects	m_Objects;
#endif // _DEBUG_OBJECTS

	// @ For TESTDEBUG
#ifdef _DEBUGFOR_RENDER
	CDrawForDebug		*m_pdrawForDebug;

	#ifdef _DEBUG_SPACETREE
		bool				m_bRenderAllTreeNodes_fordebug;
	#endif // _DEBUG_SPACETREE
#endif // _DEBUGFOR_RENDER
	// @ For TESTDEBUG

	bool				m_bpickedTriInTree;
	BV::_Ray			m_rayfordebugTree;
	STRIPICKED			m_triPickedSpceTree;

	bool				m_bpickedPhyre;
	BV::_Ray			m_rayfordebugPhyre;
	PNode				*m_intersectedNode;
	STRIPICKED			m_triPickedPhyre;

#ifdef _DEBUG_COLLECTPICKING_FORNAVI
	bool				m_bModeNaviFlipBlocks;

	bool				m_bFlipNaviBlocks;
	bool				m_bFlipNaviGoals;

	BV::_Ray			m_rayfordebugNaviBlocks;
	BV::_Ray			m_rayfordebugNaviGoals;

	vector<int>			m_vecTrisPicked_naviBlocks;
	vector<int>			m_vecTrisPicked_naviGoals;
#endif // _DEBUG_COLLECTPICKING_FORNAVI

private:
#ifdef _DEBUG_COLLECTPICKING_FORNAVI
	bool IntersectRayCollectTris(	const bool			bRayStartasMousePos,
									PCameraNode			*cameraActive, 
									BV::_Ray			&ray_out,
									vector<int>			&vecTrisPicked,
									E_MODE_COLLECTNAVI	eModeCollectNavi );
#endif // _DEBUG_COLLECTPICKING_FORNAVI

#ifdef _DEBUG_SPACETREE
	bool IntersectCameraRay_ST(	const bool			bRayStartasMousePos,
											PCameraNode			*cameraActive, 
											BV::_Ray			&ray_out,
											Vector4				*v4PickedTri_out );
#endif // _DEBUG_SPACETREE

	bool IntersectCameraRay_Traversal(	const PSSG::PDatabaseID	dbaseID,
											const bool				bRayStartasMousePos,
											PCameraNode				*cameraActive,
											BV::_Ray				&ray_out,
											Vector4					*v4PickedTri_out );


	// Application Derived Function
	
	bool InitApplication(char ** argv, int argc);

	bool InitScene();
	bool HandleInputs();

	void HandleInputs_CurrentCamera(PSSG::PNode &node, float moveScale = 1.0f, float rotateScale = 1.0f,  float perpendicularScale = 1.0f);

	bool Animate();

	bool Render();
	void RenderPickedTri(	const Vector4 *arTriPicked, 
							const PCameraNode *pActiveNodeCamera, 
							const Vector3 &v3Color );

#ifdef _DEBUG_SPACETREE
	void RenderSelectedTris(const vector<int> &vecTrisPicked,
							const PCameraNode *pActiveCamera,
							const Vector3 &v3Color );
#endif // _DEBUG_SPACETREE

	void RenderUsefulValues_debug();

	bool ExitScene();
	bool ExitApplication();

public:
	CApplicationMain()
	{
		SetInitialWindowName("Create Mesh Sample");
		m_wireframe = false;
		m_eCurrentCamera = CAMERALIST_DEBUG;
		m_meshSegmentSet = NULL;
		m_bOutputDebugInfo = true;

		m_iNumSPURSSPUs = DEFAULT_NUMNSPURSSPUS;
		m_iNumPSSGSPUs = DEFAULT_NUMPSSGSPUS;

#ifdef _DEBUG_COLLECTPICKING_FORNAVI
		m_bModeNaviFlipBlocks = true;

		m_bFlipNaviBlocks = false;
		m_bFlipNaviGoals	= false;
#endif // _DEBUG_COLLECTPICKING_FORNAVI
	};

	~CApplicationMain()
	{
#ifdef _DEBUG_NAVIGATION
		m_pnaviManager->destroyInstance();
#endif // _DEBUG_NAVIGATION

#ifdef _DEBUG_COLLECTPICKING_FORNAVI
		m_bFlipNaviBlocks = false;
		m_bFlipNaviGoals	= false;
#endif // _DEBUG_COLLECTPICKING_FORNAVI
	};
};

