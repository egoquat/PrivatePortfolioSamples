#pragma once

#include <vector>

#include "DefWz.h"

#include "Common_.h"

#include "Template_.h"

#include "CommonWz.h"

#include "LightMapMesh_.h"

#include "_parserlightmapmesh.h"

#include "_ParserLightmapMeshInstance.h"

#include "LightComp.h"

#include "3DMath_.h"

class _CParserTerrainMesh
{
public:
	UINT					m_uiGroupTypeID;				// Terrain : WZPARSETERRAIN_IDX

public:
	DWORD					m_dwNumVerts;			//.MAP ������ ��� �о�� ������(������)
	Array_<Vec3>			m_arList_Vertex;		// Vertices Pos

	DWORD					m_dwNumVPs;				//
	Array_<WzMeshVertPack>	m_arList_VertexPack;	// Index Vertices, Vertices Normal, UV

	DWORD					m_dwNumTries;			// ��ü Triangle ��
	Array_<WzFace>			m_arList_Tries;			// Triangle ����
	Array_<int>				m_arListTri_TextureIdx;	// Triangle �� ���� Texture Index Table


	DWORD					m_dwNumRealityTextures;	//��ü Texture �� : ���� OutputDefaultScript�� ����� ����
	DWORD					m_dwNumTextures;		//��ü Texture �� : Parsing �ÿ� �������� Texture�� ��
	Array_<int>				m_arListTextureIdx;		//���� Texture Index 
	Array_<int>				m_arCntFacesPerTexture;	//������ ��ǻ�� �ؽ��ĸ��� ���̽��� �������

	DWORD					m_dwNumUVVerts;
	Array_<WzUVVert>		m_arList_UVVerts;		//Face�� ������� ���ĵ� UVVert(�������� ���̽������� �� ���� UV��ǥ ������ ����)

	Array_<UINT>			m_arConvertToLMSeqFromParseTextureIdx;

	DWORD					m_dwAmbientColor;


public:
	void AllocateTextureIdxs( DWORD dwNumTextures = DEFAULT_ARR_CAPACITY )
	{
		m_arListTextureIdx.NewAllocateBuffer( dwNumTextures );
	}

	void DeallocateTextureIdxs()
	{
		m_arListTextureIdx.Clear_();
	}

	void AllocateCntFacesPerTexture( DWORD dwNumTextures )
	{
		m_dwNumTextures = dwNumTextures;
		m_arCntFacesPerTexture.NewAllocateBuffer( (UINT)dwNumTextures );
	}

    void DeallocateCntFacesPerTexture()
	{
		m_dwNumTextures = 0;
		m_arCntFacesPerTexture.Clear_();
	}

	void	AllocateMeshVertices( DWORD dwNumVerts )
	{
		m_dwNumVerts = dwNumVerts;
		m_arList_Vertex.NewAllocateBuffer( (UINT)dwNumVerts );
	}

	void	DeallocateMeshVertices()
	{
		m_arList_Vertex.Clear_();
		m_dwNumVerts = 0;
	}

	void	AllocateFaces( DWORD dwNumFaces )
	{
		m_dwNumTries = dwNumFaces;
		m_arList_Tries.NewAllocateBuffer( (UINT)dwNumFaces );
		m_arListTri_TextureIdx.NewAllocateBuffer( (UINT)dwNumFaces );
	}

	void	DeallocateFaces()
	{
		m_arList_Tries.Clear_();
		m_arListTri_TextureIdx.Clear_();
		m_dwNumTries = 0;
	}

	void	AllocateMeshVP( DWORD dwNumVP )
	{
		m_dwNumVPs = dwNumVP;
		m_arList_VertexPack.NewAllocateBuffer( (UINT)dwNumVP );
	}

	void	DeallocateVP()
	{
		m_arList_VertexPack.Clear_();
		m_dwNumVPs = 0;
	}

	void	AllocateUVVerts( DWORD dwNumUVVerts )
	{
		m_dwNumUVVerts = dwNumUVVerts;
		m_arList_UVVerts.NewAllocateBuffer( (UINT)dwNumUVVerts );
	}

	void	DeallocateUVVertx()
	{
		m_arList_UVVerts.Clear_();
		m_dwNumUVVerts = 0;
	}

public:
	Array_<CLMMesh_*>*		m_parLMMeshes;			// Terrain Objects Array

public:
	void Clear()
	{
		m_uiGroupTypeID	= WZPARSETERRAIN_IDX;
		m_dwAmbientColor = 0;

	}

public:

	void Release()
	{
		DeallocateMeshVertices();
		DeallocateFaces();
		DeallocateMeshVertices();
		DeallocateUVVertx();
		DeallocateCntFacesPerTexture();

		DeallocateTextureIdxs();
	}

public:
	_CParserTerrainMesh() 
	{
		Clear();
		Release();
	};

	~_CParserTerrainMesh()
	{
		Release();
	};
};


