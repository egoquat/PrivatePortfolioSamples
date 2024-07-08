
// 임시 메쉬 컨버터 차후 외부 와 내부 브리지 및 컨버터 역활로 승화시키자.

#pragma once

#include <map>

#include <vector>

#include "Common_.h"

#include "CommonWz.h"

#include "LightMapMesh_.h"

#include "Global_.h"

#include "_parserlightmapmesh.h"

#include "_parserTerrainMesh.h"

#include "_parserlightmapmeshinstance.h"

class CMeshConverter
{
protected:

public:
	// Wz Map의 경우 Script Loading 시점에서 Terrain Geometry에 대한 정보와
	// Terrain Object Geometry 정보를 각각 따로 읽어야 한다.
	// 결과 적으로 구성은 Array LMMeshes 와 Parent Meshes 에 동일하게 이루어 진다.
	BOOL ConvertToLMMeshes_WzMap(	const _CParserTerrainMesh& _TerrainMesh,
									const mapParserMeshes& ObjectInstances, 
									Array_<CLMMesh_*>& ArrLMMeshes, 
									Array_<CParentLMMesh*>& ArrParentMeshes,
									vector<CRasterTri>& ArrWholeTriangles, 
									UINT& uiCntRealityOutputLMObject,
									CCommonWz& _Wz,
									CLMOption& LMOption,
									vector<LM_OBJECTPROXY_IDENTITY> *pvExceptionCastShadowObjectsList,
									vector<LM_OBJECTPROXY_IDENTITY> *pvExceptionReceiveShadowObjectsList )
	{
		UINT		uiGlobalyLMCnt = 0;

		ConvertToLMMeshes_WzMapTerrain( const_cast<_CParserTerrainMesh&>(_TerrainMesh), 
										ArrParentMeshes, 
										ArrLMMeshes,
										ArrWholeTriangles,
										uiCntRealityOutputLMObject,
										uiGlobalyLMCnt,
										LMOption );

		ConvertToLMMeshes( ObjectInstances, 
							ArrLMMeshes, 
							ArrParentMeshes,
							ArrWholeTriangles, 
							_Wz,
							uiCntRealityOutputLMObject,
							uiGlobalyLMCnt,
							LMOption,
							pvExceptionCastShadowObjectsList,
							pvExceptionReceiveShadowObjectsList );

		return TRUE;
	}


	
	// 우선 임시적으로 한개의 Parent 메쉬로 구성 .
	void			ConvertToLMMeshes_WzMapTerrain( _CParserTerrainMesh& _TerrainMesh,
													Array_<CParentLMMesh*>&		pArParentMeshes, 
													Array_<CLMMesh_*>&			parLMMeshes,
													vector<CRasterTri>&		pArrWholeTriangles, 
													UINT& uiCntRealityOutputLMObject,
													UINT& uiGlobalyLMCnt,
													CLMOption& LMOption );


	// Alpha Texture 설정이 된 이후에 호출 되어야 한다.
	// Alpha Texture 설정 방법 몇가지 
	//		1. 외부에서 AlphaBuffer를 직접 인자로 받아서, 내부 Array로 변환
	//				ex) manager->SetAlphaBuffer(MeshID, AlphaBuffer);		ok
	//		2. 외부에서 내부 Array로 접근할 수 있는 인자를 도출
	//				 ex) manager->GetMesh(n)->SetAlphaArray(nIndex, ALphaVIsibility);
	//		3. 외부에서 AlphaTextureFileName을 받아서, 내부 변환			ok
	BOOL			ConvertToLMMeshes( const mapParserMeshes& ObjectInstances, 
										Array_<CLMMesh_*>& ArrLMMeshes, 
										Array_<CParentLMMesh*>& ArrParentMeshes,
										vector<CRasterTri>& ArrWholeTriangles, 
										CCommonWz& _Wz,
										UINT& uiCntRealityOutputLMObject,
										UINT& uiGlobalyLMCnt,
										CLMOption& LMOption,
										vector<int> *pvExceptionCastShadowObjectsList,
										vector<int> *pvExceptionReceiveShadowObjectsList );

public:
	CMeshConverter(void);
	~CMeshConverter(void);
};
