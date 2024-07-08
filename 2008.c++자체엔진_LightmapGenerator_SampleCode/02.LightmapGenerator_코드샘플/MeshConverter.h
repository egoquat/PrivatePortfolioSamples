
// �ӽ� �޽� ������ ���� �ܺ� �� ���� �긮�� �� ������ ��Ȱ�� ��ȭ��Ű��.

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
	// Wz Map�� ��� Script Loading �������� Terrain Geometry�� ���� ������
	// Terrain Object Geometry ������ ���� ���� �о�� �Ѵ�.
	// ��� ������ ������ Array LMMeshes �� Parent Meshes �� �����ϰ� �̷�� ����.
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


	
	// �켱 �ӽ������� �Ѱ��� Parent �޽��� ���� .
	void			ConvertToLMMeshes_WzMapTerrain( _CParserTerrainMesh& _TerrainMesh,
													Array_<CParentLMMesh*>&		pArParentMeshes, 
													Array_<CLMMesh_*>&			parLMMeshes,
													vector<CRasterTri>&		pArrWholeTriangles, 
													UINT& uiCntRealityOutputLMObject,
													UINT& uiGlobalyLMCnt,
													CLMOption& LMOption );


	// Alpha Texture ������ �� ���Ŀ� ȣ�� �Ǿ�� �Ѵ�.
	// Alpha Texture ���� ��� ��� 
	//		1. �ܺο��� AlphaBuffer�� ���� ���ڷ� �޾Ƽ�, ���� Array�� ��ȯ
	//				ex) manager->SetAlphaBuffer(MeshID, AlphaBuffer);		ok
	//		2. �ܺο��� ���� Array�� ������ �� �ִ� ���ڸ� ����
	//				 ex) manager->GetMesh(n)->SetAlphaArray(nIndex, ALphaVIsibility);
	//		3. �ܺο��� AlphaTextureFileName�� �޾Ƽ�, ���� ��ȯ			ok
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
