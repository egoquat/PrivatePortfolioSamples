#pragma once

#include <PSSG.h>

#include "CommonApp.h"

#include "CommonSingleton.h"

#include "BoundaryVolume.h"
#include "Tri.h"

#include <set>

using namespace PSSG;
using namespace Vectormath::Aos;

namespace TRIPART
{
	typedef vector<CTri>	VEC_TRIS;

	class CTriContainer : public CSingleton<CTriContainer>
	{
	public:
		VEC_TRIS				m_vecTris;
		BV::_BV_AABB			m_bvAABB;

	public:
		void Initialize();
		void Release();

	private:
		int getSizeDataType( PTypeID EDataType );

	public:
		const VEC_TRIS	&getTris(){ return m_vecTris; };
		unsigned int getCountTries(){ return m_vecTris.size(); };
		bool getTri(const unsigned int iIdxTriangle, CTri &triangle);
		CTri &getTri(unsigned int uiSeqTri)	{ return m_vecTris[uiSeqTri]; };

		bool InsertAllTriangles(		const void			*pArrIndexVertex,
										PTypeID				eTypeData,
										const unsigned int	iCntIndexVertices,
										const float			*parVertexData,
										vector<CTri>		&vecTris,
										Matrix4				*matTransform=NULL );

		bool constructAllTriangles( PRenderDataSource *renderDataSource, Matrix4 *matTransform = NULL );
		//	void constructAllTrianglesWithoutIV_test( PRenderDataSource *renderDataSource );

	public:
		CTriContainer();
		~CTriContainer();
	};
}



