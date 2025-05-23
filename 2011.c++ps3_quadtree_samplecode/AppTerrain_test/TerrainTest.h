#pragma once

#include <PSSG.h>
#include <PSSGTerrain/PSSGTerrain.h>
#include <Extra/PSSGExtra.h>


#include "commonApp.h"
#include "CommonSingleton.h"
#include "CommonMath.h"

#include "TriContainer.h"
#include "SpaceTree.h"
#include <PSSGCg/PSSGCg.h>
#include <PSSGDDS/PSSGDDS.h>
#include <PSSGHDDCache/PSSGHDDCache.h>
#include <PSSGTerrain/PSSGTerrain.h>

using namespace Vectormath::Aos;
using namespace PSSG;

#ifdef _DEBUG_TERRAINTEST


//  説明：
// <c>PTextureCacheElement</c>クラスは、ストリーミングした地形テクスチャの現在の状態を保持します。
// このクラスは、レンダーインタフェースがマッピングするアドレスへのポインタを格納することにより、直接にRSXメモリに対するストリーミングデータの処理をサポートします。
class PSampleTextureCacheElement : public PTextureCacheElement<3>, public PSimpleListElement<PSampleTextureCacheElement>
{
public:
	static PSSG::PTextureCacheElementBase::PTextureCacheInitInfo	s_initInfo[3];
};

typedef PTextureCache<PSampleTextureCacheElement, 100+36+4> PSampleTextureCache;

// 説明:
//  <c>PTerrainStreamingThread</c>クラスは、ストリーミング用のテクスチャ・ジオメトリキャッシュの使い方を示す、サンプルクラスです。
class PTerrainStreamingThread
{
	PSSG_PRIVATE_ASSIGNMENT_OPERATOR(PTerrainStreamingThread);

protected:
	PSampleTextureCache		&m_textureCache;		// ストリーミングしたテクスチャの管理に使用するテクスチャキャッシュ。
	PGeometryCache			&m_geometryCache;		// ストリーミングしたジオメトリの管理に使用するジオメトリキャッシュ。
	PTerrainManager<>		&m_terrain;				// ジオメトリのストリーミング対象となる地形。
	volatile bool			m_quit;					// ストリーミングスレッドの停止を示すフラグ。
	volatile bool			m_isRunning;			// ストリーミングスレッドが実行中であることを示すフラグ。
	bool					m_useCopyTexture;		// PRenderInterface::copyTextureの使用を示すフラグ。

	// 説明:
	//  ストリーミングする拡散テクスチャの名前を取得します。
	//  引数: 
	// id : テクスチャをストリーミングする地形ブロックのID。
	// output : 名前の出力バッファ。
	static void getDiffName(const PString &id, PString &output)
	{
		output = id + ".craw";
	}

	// 説明:
	//  ストリーミングする法線テクスチャの名前を取得します。
	//  引数: 
	// id : テクスチャをストリーミングする地形ブロックのID。
	// output : 名前の出力バッファ。
	static void getNMapName(const PString &id, PString &output)
	{
		output = id + "_norm.craw";
	}

	// 解説:
	// ストリーミングの対象となるライティングテクスチャの名前を取得します。
	// 引数:
	// id : テクスチャをストリーミングする地形ブロックのID。
	// output :名前の出力バッファ。
	static void getLitName(const PString &id, PString &output)
	{
		output = id + "_lit.craw";
	}

	// 説明:
	//  ストリーミングが必要なテクスチャ・ジオメトリ用のスレッドのエントリポイント。
	//  引数: 
	// data : スレッドシステムに渡されるユーザデータへのポインタ。起動する<c>PTerrainStreamingThread</c>を指しています。
	//  返り値: 
	//  <c>start()</c>メソッドからのOS標準のスレッド完了値。
	static unsigned int startStreamingThread(void *data)
	{
		PTerrainStreamingThread *streamingThread = (PTerrainStreamingThread *)data;
		return streamingThread->start();
	}

	// 説明:
	//  テクスチャ・ジオメトリに必要なストリーミング用のスレッドとして働きます。
	//  返り値: 
	//  OSに無事完了を知らせる0。
	unsigned int start()
	{
		PTerrainBlock *blocksEnd = m_terrain.m_blocks + m_terrain.getBlockCount();
		m_isRunning = true;
		while(!m_quit)
		{
			PTerrainBlock *it;

			// ここで、可能な要素をすべて解放します（LODがすべていっぱいの場合は、後で解放できます）。
			// 全レベルのキャッシュが一杯の場合に使用するグローバルLODループ。
			for(it = m_terrain.m_blocks ; it != blocksEnd && !m_quit ; it++)
			{
				PTerrainBlock &block = *it;

				PGeometryCacheElement *goldElement = block.m_geometry.m_old;
				if(goldElement)
				{
					m_geometryCache.release(*goldElement);
					block.m_geometry.m_old = NULL;
				}

				int lod = block.m_lodValue;
				if (lod >= PD_TERRAIN_BLOCK_MAX_LOD)
				{
					block.m_texture.m_next = NULL;
					block.m_geometry.m_next = NULL;
				}
			}

			m_textureCache.recycleTextureCacheElements();

			for(int i = 0 ; i < (int)m_textureCache.getLODCount() && !m_quit ; i++)
			{
				if(m_textureCache.isFull(i))
					continue;

				for(it = m_terrain.m_blocks ; it != blocksEnd && !m_quit ; it++)
				{
					PTerrainBlock &block = *it;

					PGeometryCacheElement *goldElement = block.m_geometry.m_old;
					if(goldElement)
					{
						m_geometryCache.release(*goldElement);
						block.m_geometry.m_old = NULL;
					}

					int lod = block.m_lodValue;
					if (lod >= PD_TERRAIN_BLOCK_MAX_LOD)
					{
						block.m_texture.m_next = NULL;
						block.m_geometry.m_next = NULL;
						continue;
					}

					//  ジオメトリの更新を処理します
					PGeometryCacheElement *next = block.m_geometry.m_next;
					if(lod == 0)
					{
						if(!next)
#ifdef __CELLOS_LV2__
							next = m_geometryCache.add(block.getBlockPacketsName() +  ".data");
#else // __CELLOS_LV2__
							next = m_geometryCache.add(block.getBlockPacketsName() +  ".pc.data");
#endif  // __CELLOS_LV2__
					}
					else if(block.m_geometry.m_current)
					{
						next = NULL;
					}
					block.m_geometry.m_next = next;

					if(lod != i)
						continue;

					PTextureCacheElementBase *element = block.m_texture.m_next;
					int oldLOD = element ? (int)element->m_lod : -1;
					if(oldLOD == i)
						continue;

					const char *id = block.m_id;
					PString diffName, normName, litName;
					getDiffName(id, diffName);
					getNMapName(id, normName);
					getLitName(id, litName);
					const char *textureNames[3] = {diffName, normName, litName};

					PTextureCacheElementBase *newCacheEl;
					PTextureCacheElementBase *oldCacheEl = m_useCopyTexture ? block.m_texture.m_current : NULL;
					m_currentBlock = &block;
					newCacheEl = m_textureCache.addComp(textureNames, lod, oldCacheEl);
					m_currentBlock = NULL;

					if(newCacheEl)
						block.m_texture.m_next = newCacheEl;

					//  より高いLODで領域を解放している場合には、それを使います！
					if(oldLOD >= 0 && oldLOD < i)
					{
						i = -1;
						break;
					}
				}
			}
			PSSGOS::sleep(1);
		}
		m_quit = false;
		return 0;
	}

public:

	PTerrainBlock *m_currentBlock;	// ロードされる現在ブロック。

	// 説明:
	//  <c>PTerrainStreamingThread</c>クラスのコンストラクタ。
	//  引数: 
	// textureCache :  ストリーミングされたテクスチャを管理するためのテクスチャキャッシュ。
	// geometryCache : ストリーミングされたジオメトリを管理するためのジオメトリキャッシュ。
	// terrain :       ジオメトリをストリーミングする地形。
	PTerrainStreamingThread(PSampleTextureCache &textureCache, PGeometryCache &geometryCache, PTerrainManager<>	&terrain)
		: m_textureCache(textureCache)
		, m_geometryCache(geometryCache)
		, m_terrain(terrain)
		, m_quit(false)
		, m_isRunning(false)
		, m_useCopyTexture(false)
		, m_currentBlock(NULL)
	{
	}

	// 説明:
	//  この<c>PTerrainStreamingThread</c>インスタンスを使うスレッドの終了を待ちます。
	void quit()
	{
		if(m_isRunning)
		{
			m_quit = true;
			while(m_quit)
				PSSGOS::sleep(1);
		}
	}

	// 説明:
	//  この<c>PTerrainStreamingThread</c>インスタンスを使ってテクスチャ・ジオメトリをストリーミングするスレッドを作成します。
	//  返り値リスト: 
	// PE_RESULT_OBJECT_OF_SAME_NAME_EXISTS : スレッドはすでに初期化済みです。
	// PE_RESULT_NO_ERROR: スレッドの初期化に成功しました。
	// それ以外: 内部メソッドからのエラー。
	PResult run()
	{
		if(m_isRunning)
			return PE_RESULT_OBJECT_OF_SAME_NAME_EXISTS;
		PThreadID threadId;
		PSSG_TRY(PThread::create(threadId, 0, PTerrainStreamingThread::startStreamingThread, this, PThread::PE_THREADPRIORITY_LOWER, "PhyreEngineStreamingThread"));
		PSSG_TRY(PThread::close(threadId));
		return PE_RESULT_NO_ERROR;
	}

	//  説明：
	// ミップレベルデータを、再ロードではなく古いキャッシュエントリから転送する（可能な場合）ことを示すフラグを設定します。この処理には、レンダーインタフェースによる<c>copyTexture()</c>のサポートが必要です。
	//  引数：
	// useCopyTexture - <c>copyTexture()</c>を使用する場合はtrue、再ロードする場合はfalse。
	void setUseCopyTexture(bool useCopyTexture)
	{
		m_useCopyTexture = useCopyTexture;
	}
};

class CTerrainTest
{
private:
	
	PRootNode				*m_prootNodetotal;
	PNode					*m_pNoderootTerr;

	vector<PLightNode*>		m_vecLightNodes;

	bool					m_wireframe;

	PRenderInterface		*m_textureCacheRenderInterface;	

	//public:
	PTerrainManager<>		m_terrain;						

	PSampleTextureCache		m_textureCache;			
	PGeometryCache			m_geometryCache;			
	PTerrainStreamingThread	m_streamingThread;
	PTerrainStatusTexture	m_statusTexture;		
	bool					m_showTerrainStatus;	

	PSampleTextureCacheElement::PShaderGroupBinding m_shaderGroupBinding;

	PRenderDataSource		*m_pLores;

	Vector3					m_v3BoundaryMin;
	Vector3					m_v3BoundaryMax;

public:
	CSpaceTree				*m_pQuadTree;
	CSpaceTree				*getSpaceTree()
	{
		return m_pQuadTree;
	}

	// @ Space Tree
	CTriContainer			*m_pContainerTri;
	CTriContainer			*getTriContainer()
	{
		return m_pContainerTri;
	}

private:
	float interpolateValue(int i, int maxInt, float maxFloat);
	float interpolateValueDistribute(int i, int maxInt, float maxFloat);


	PShaderGroup *createTerrainShader(	PDatabase &database, 
										const char *name, 
										PResult *result = NULL);


	bool addSpotlighttoWorld(	const Vector3	&v3ColorLight,
								const Vector3	&v3PosLight,
								const float		fThetaLight );

	PShaderInstance *createTestTexturedShaderInstance(	PDatabase *database,
														unsigned int width = 128, 
														unsigned int height = 128, 
														unsigned int checkerWidth = 8, 
														unsigned int checkerHeight = 8 );


	PModifierNetworkInstance *instanceSegment(	PSSG::PDatabase &database, 
												PSSG::PRenderDataSource *source0, 
												PSSG::PRenderDataSource *source1, 
												PSSG::PModifierNetwork *network, 
												PSSG::PShaderInstance *shaderInstance );


	bool constructSpaceTree( PDatabase			*pDatabase );

public:
#ifdef _DEBUG_SPACETREE
	void renderspacetree_fordebug(	const Vector3	&v3colorLine,
									const Vectormath::Aos::Matrix4 *matViewPrjCamera );
#endif // #ifdef _DEBUG_SPACETREE

	bool getIntersectTri(const BV::_Ray &_ray, 
						Vector4 *pPickedTriangle_out, 
						Vector3 &v3PosIntersect_out);

	PResult constructSurfaceRenderDataSource(	const PSSG::PRenderDataSource	*pRenderDataSrc_in,
												PDatabase						*pdatabase,
												PRenderInterface					&renderInterface,
												PSSG::PRenderDataSource			*&pRenderDataSrc_out	 );

	// @ Main Process
	bool createTerrain_test(	PRootNode			*pnodeRoot,
								PDatabase			*pDatabase,
								PCameraNode			**arCamera_,
								int					icntCamera,
								PSSG::PRenderInterface *prenderInterface_,
								PString				szPathData,
								PString				szNameFileTerrain,
								PString				szNameIDShader,
								PString				szNameDiffuseFile,
								PString				szNameNormalFile,
								PString				szNameLitFile );

	bool RenderPre(			PCameraNode	*pNodeCamera );

	void Release();

public:
	CTerrainTest();
	~CTerrainTest();
};

#endif // _DEBUG_TERRAINTEST
