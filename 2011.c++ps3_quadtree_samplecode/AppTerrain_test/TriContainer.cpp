#include "TriContainer.h"

using namespace TRIPART;

void CTriContainer::Initialize()
{

}

void CTriContainer::Release()
{
	m_vecTris.clear();
}



int CTriContainer::getSizeDataType(PTypeID EDataType )
{
	switch(EDataType)
	{
	case PSSG::PE_TYPE_UCHAR:
		{
			return sizeof(unsigned char);
		}
		break;
	case PSSG::PE_TYPE_USHORT:
		{
			return sizeof(unsigned short);
		}
		break;
	case PSSG::PE_TYPE_UINT:
		{
			return sizeof(unsigned int);
		}
		break;
	default:
		return -1;
	}
}

bool CTriContainer::InsertAllTriangles(	const void					*pArrIndexVertex,
											  PTypeID				eTypeData,
											  const unsigned int	iCntIndexVertices,
											  const float			*parVertexData,
											  vector<CTri>			&vecTris,
											  Matrix4				*matTransform )
{
	Vector3			v3TriVer[3];
	Vector4			v4CurrentVertex(1.0f);
	unsigned int	ariTriIdx[3];
	unsigned int	iCntTries = (iCntIndexVertices/3)+1;
	CTri			triCurrent;
	bool			bTransform = (matTransform!=NULL);

	m_vecTris.reserve( m_vecTris.size()+iCntTries );

	switch( eTypeData )
	{
		case PE_TYPE_UCHAR:
			{
				unsigned char *pArrIV = (unsigned char*)pArrIndexVertex;

				for( unsigned int iIdx=0, iTri=0; iIdx<iCntIndexVertices; iIdx+=3,++iTri )
				{
					ariTriIdx[0] = (unsigned int)pArrIV[iIdx];
					ariTriIdx[1] = (unsigned int)pArrIV[iIdx+1];
					ariTriIdx[2] = (unsigned int)pArrIV[iIdx+2];

					for( int itV=0; itV<3; ++itV )
					{
						v4CurrentVertex.setX( *(parVertexData + (ariTriIdx[itV]*3)) );
						v4CurrentVertex.setY( *(parVertexData + (ariTriIdx[itV]*3+1)) );
						v4CurrentVertex.setZ( *(parVertexData + (ariTriIdx[itV]*3+2)) );

						if(bTransform)
						{
							v4CurrentVertex = *(matTransform)*v4CurrentVertex;
						}

						vectorCopy4to3(v3TriVer[itV],v4CurrentVertex);

						if(v3TriVer[itV].getX()>m_bvAABB._v3Max.getX()) { m_bvAABB._v3Max.setX(v3TriVer[itV].getX()); }
						if(v3TriVer[itV].getY()>m_bvAABB._v3Max.getY()) { m_bvAABB._v3Max.setY(v3TriVer[itV].getY()); }
						if(v3TriVer[itV].getZ()>m_bvAABB._v3Max.getZ()) { m_bvAABB._v3Max.setZ(v3TriVer[itV].getZ()); }

						if(v3TriVer[itV].getX()<m_bvAABB._v3Min.getX()) { m_bvAABB._v3Min.setX(v3TriVer[itV].getX()); }
						if(v3TriVer[itV].getY()<m_bvAABB._v3Min.getY()) { m_bvAABB._v3Min.setY(v3TriVer[itV].getY()); }
						if(v3TriVer[itV].getZ()<m_bvAABB._v3Min.getZ()) { m_bvAABB._v3Min.setZ(v3TriVer[itV].getZ()); }
					}

					triCurrent.Set(v3TriVer, ariTriIdx);
					vecTris.push_back( triCurrent );
				}
			}
			break;
		case PE_TYPE_USHORT:
			{
				unsigned short *pArrIV = (unsigned short*)pArrIndexVertex;

				for( unsigned int iIdx=0, iTri=0; iIdx<iCntIndexVertices; iIdx+=3,++iTri )
				{
					ariTriIdx[0] = (unsigned int)pArrIV[iIdx];
					ariTriIdx[1] = (unsigned int)pArrIV[iIdx+1];
					ariTriIdx[2] = (unsigned int)pArrIV[iIdx+2];

					for( int itV=0; itV<3; ++itV )
					{
						v4CurrentVertex.setX( *(parVertexData + (ariTriIdx[itV]*3)) );
						v4CurrentVertex.setY( *(parVertexData + (ariTriIdx[itV]*3+1)) );
						v4CurrentVertex.setZ( *(parVertexData + (ariTriIdx[itV]*3+2)) );

						if(bTransform)
						{
							v4CurrentVertex = *(matTransform)*v4CurrentVertex;
						}

						vectorCopy4to3(v3TriVer[itV],v4CurrentVertex);

						if(v3TriVer[itV].getX()>m_bvAABB._v3Max.getX()) { m_bvAABB._v3Max.setX(v3TriVer[itV].getX()); }
						if(v3TriVer[itV].getY()>m_bvAABB._v3Max.getY()) { m_bvAABB._v3Max.setY(v3TriVer[itV].getY()); }
						if(v3TriVer[itV].getZ()>m_bvAABB._v3Max.getZ()) { m_bvAABB._v3Max.setZ(v3TriVer[itV].getZ()); }

						if(v3TriVer[itV].getX()<m_bvAABB._v3Min.getX()) { m_bvAABB._v3Min.setX(v3TriVer[itV].getX()); }
						if(v3TriVer[itV].getY()<m_bvAABB._v3Min.getY()) { m_bvAABB._v3Min.setY(v3TriVer[itV].getY()); }
						if(v3TriVer[itV].getZ()<m_bvAABB._v3Min.getZ()) { m_bvAABB._v3Min.setZ(v3TriVer[itV].getZ()); }
					}

					triCurrent.Set(v3TriVer, ariTriIdx);
					vecTris.push_back( triCurrent );
				}
			}
			break;
		case PE_TYPE_UINT:
			{
				unsigned int *pArrIV = (unsigned int*)pArrIndexVertex;

				for( unsigned int iIdx=0, iTri=0; iIdx<iCntIndexVertices; iIdx+=3,++iTri )
				{
					ariTriIdx[0] = pArrIV[iIdx];
					ariTriIdx[1] = pArrIV[iIdx+1];
					ariTriIdx[2] = pArrIV[iIdx+2];

					for( int itV=0; itV<3; ++itV )
					{
						v4CurrentVertex.setX( *(parVertexData + (ariTriIdx[itV]*3)) );
						v4CurrentVertex.setY( *(parVertexData + (ariTriIdx[itV]*3+1)) );
						v4CurrentVertex.setZ( *(parVertexData + (ariTriIdx[itV]*3+2)) );

						if(bTransform)
						{
							v4CurrentVertex = *(matTransform)*v4CurrentVertex;
						}

						vectorCopy4to3(v3TriVer[itV],v4CurrentVertex);

						if(v3TriVer[itV].getX()>m_bvAABB._v3Max.getX()) { m_bvAABB._v3Max.setX(v3TriVer[itV].getX()); }
						if(v3TriVer[itV].getY()>m_bvAABB._v3Max.getY()) { m_bvAABB._v3Max.setY(v3TriVer[itV].getY()); }
						if(v3TriVer[itV].getZ()>m_bvAABB._v3Max.getZ()) { m_bvAABB._v3Max.setZ(v3TriVer[itV].getZ()); }

						if(v3TriVer[itV].getX()<m_bvAABB._v3Min.getX()) { m_bvAABB._v3Min.setX(v3TriVer[itV].getX()); }
						if(v3TriVer[itV].getY()<m_bvAABB._v3Min.getY()) { m_bvAABB._v3Min.setY(v3TriVer[itV].getY()); }
						if(v3TriVer[itV].getZ()<m_bvAABB._v3Min.getZ()) { m_bvAABB._v3Min.setZ(v3TriVer[itV].getZ()); }
					}

					triCurrent.Set(v3TriVer, ariTriIdx);
					vecTris.push_back( triCurrent );
				}
			}
			break;
		default:
			{
				assert( 0 && "Error : Undefinition type of indexbyffer while Construct Triangle." );
				return false;
			}
	}

	return true;
}


bool CTriContainer::constructAllTriangles( PRenderDataSource *renderDataSource, Matrix4 *matTransform )
{
	// Vertex Array
	PRenderStream *pRenderStream = renderDataSource->getStream(0);
	PDataBlock *vertexDatablock = pRenderStream->getDataBlock();
	float *parVertexData = (float*)vertexDatablock->getData();

	// Index Vertex Array
	PRenderIndexSource *pIndexSource = renderDataSource->getIndexSource();
	int iCntIndexVertices = pIndexSource->getElementCount();
	
	return InsertAllTriangles(	pIndexSource->getData(),
									pIndexSource->getDataType().m_type,
									iCntIndexVertices, 
									parVertexData, 
									m_vecTris,
									matTransform );
}


// void CTriContainer::constructAllTrianglesWithoutIV_test( PRenderDataSource *renderDataSource )
// {
// 	PRenderStream *pRenderStream = renderDataSource->getStream(0);
// 	PDataBlock *vertexDatablock = pRenderStream->getDataBlock();
// 	float *parVertexData = (float*)vertexDatablock->getData();
// 
// 	unsigned int iCntVertex = vertexDatablock->getElementCount();
// 
// 
// 	unsigned int	iCntIdxVertices_est = (iCntVertex+1) * 3,
// 					iCntIdxVertices = 0,
// 					iSeq = 0, iIdxVertex = 0;
// 	vector<unsigned int>	vecIdxVertices;
// 	vecIdxVertices.reserve( iCntIdxVertices_est );
// 
// 	for( iIdxVertex=0, iSeq=0; iIdxVertex<iCntVertex; ++iIdxVertex,iSeq+=3 )
// 	{
// 		vecIdxVertices.push_back(iIdxVertex);
// 		vecIdxVertices.push_back(iIdxVertex+1);
// 		vecIdxVertices.push_back(iIdxVertex+2);
// 	}
// 	iCntIdxVertices = vecIdxVertices.size();
// 
// 	Vector3		v3CurrentVertex, v3TriVer[3];
// 	unsigned int ariTriIdx[3];
// 	unsigned int iCntTries = (iCntIdxVertices/3)+1;
// 	CTri		triCurrent;
// 	m_vecTris.reserve(iCntTries);
// 
// 	// For TESTDEBUG
// 	char			szoutputdebug[1024];
// 	unsigned int	uiBuf_=0;
// 	memset(szoutputdebug, 0, sizeof(char) * 1024);
// 	// For TESTDEBUG
// 
// 	for( unsigned int iIdx=0, iTri=0; iIdx<iCntIdxVertices; iIdx+=3,++iTri )
// 	{
// 
// 
// 		if( 10038==iIdx )
// 		{
// 			int iBreakPoint = 0;
// 		}
// 
// 		ariTriIdx[0] = vecIdxVertices[iIdx];
// 		ariTriIdx[1] = vecIdxVertices[iIdx+1];
// 		ariTriIdx[2] = vecIdxVertices[iIdx+2];
// 
// 		// For TESTDEBUG
// 		sprintf( szoutputdebug, "(%d/%d) // %d, %d, %d // (%d/%d)\n", 
// 					iIdx, 
// 					iCntIdxVertices, 
// 					ariTriIdx[0], 
// 					ariTriIdx[1], 
// 					ariTriIdx[2], 
// 					iTri, 
// 					iCntTries);
// 		OutputDebugString( szoutputdebug );
// 		// For TESTDEBUG
// 
// 		if( iTri > iCntTries )
// 		{
// 			int iBreakPoint = 0;
// 		}
// 
// 		for( int itV=0; itV<3; ++itV )
// 		{
// 			v3TriVer[itV].setX(*(parVertexData + (ariTriIdx[itV]*3)));
// 			v3TriVer[itV].setY(*(parVertexData + (ariTriIdx[itV]*3+1)));
// 			v3TriVer[itV].setZ(*(parVertexData + (ariTriIdx[itV]*3+2)));
// 
// 			if(v3TriVer[itV].getX()>m_bvAABB._v3Max.getX()) { m_bvAABB._v3Max.setX(v3TriVer[itV].getX()); }
// 			if(v3TriVer[itV].getY()>m_bvAABB._v3Max.getY()) { m_bvAABB._v3Max.setY(v3TriVer[itV].getY()); }
// 			if(v3TriVer[itV].getZ()>m_bvAABB._v3Max.getZ()) { m_bvAABB._v3Max.setZ(v3TriVer[itV].getZ()); }
// 
// 			if(v3TriVer[itV].getX()<m_bvAABB._v3Min.getX()) { m_bvAABB._v3Min.setX(v3TriVer[itV].getX()); }
// 			if(v3TriVer[itV].getY()<m_bvAABB._v3Min.getY()) { m_bvAABB._v3Min.setY(v3TriVer[itV].getY()); }
// 			if(v3TriVer[itV].getZ()<m_bvAABB._v3Min.getZ()) { m_bvAABB._v3Min.setZ(v3TriVer[itV].getZ()); }
// 		}
// 
// 		triCurrent.Set( v3TriVer );
// 		m_vecTris.push_back( triCurrent );
// 	}
// 
// 
// 	// For TESTDEBUG
// 	for ( unsigned int iIdx=0; iIdx<m_vecTris.size(); ++iIdx )
// 	{
// 		CTri		&triCurr = m_vecTris[iIdx];
// 
// 		uiBuf_ = uiBuf_ + sprintf(uiBuf_+szoutputdebug, "%d. Triangle P1(%f, %f, %f) \t",
// 			iIdx,	triCurr.v3PT[0].getX(), 
// 			triCurr.v3PT[0].getY(), 
// 			triCurr.v3PT[0].getZ() );
// 
// 		uiBuf_ = uiBuf_ + sprintf(uiBuf_+szoutputdebug, "P2(%f, %f, %f) \t",
// 			triCurr.v3PT[1].getX(), 
// 			triCurr.v3PT[1].getY(), 
// 			triCurr.v3PT[1].getZ() );
// 
// 		uiBuf_ = uiBuf_ + sprintf(uiBuf_+szoutputdebug, "P3(%f, %f, %f) \n",
// 			triCurr.v3PT[2].getX(), 
// 			triCurr.v3PT[2].getY(), 
// 			triCurr.v3PT[2].getZ() );
// 
// 
// 		OutputDebugString( szoutputdebug );
// 		uiBuf_ = 0;
// 
// 	}
// 
// 	int iBreakPoint = 0;
// 	// For TESTDEBUG
// }


bool CTriContainer::getTri(const unsigned int iIdxTriangle, CTri &triangle)
{
	if( iIdxTriangle > m_vecTris.size() )	
	{
		assert("Accessed Index is out of range in Triangle Container" && 0);
		return false;
	}

	triangle = m_vecTris[iIdxTriangle];

	return true;
}

CTriContainer::CTriContainer()
{
	m_bvAABB._v3Max.setX(-FLT_MAX);
	m_bvAABB._v3Max.setY(-FLT_MAX);
	m_bvAABB._v3Max.setZ(-FLT_MAX);

	m_bvAABB._v3Min.setX(FLT_MAX);
	m_bvAABB._v3Min.setY(FLT_MAX);
	m_bvAABB._v3Min.setZ(FLT_MAX);
}

CTriContainer::~CTriContainer()
{

}