#pragma once

#include <set>
#include <list>
#include <vector>

#include "DefLightmap.h"

#include "DefMath.h"

#include "Matrix__.h"

#include "Triangle.h"

#include "BoundingVolume.h"

// Debug 용도
////////////////////////////////////////////////////////////////////////////////////////
struct	AABBDebug
{
	Vec3		_v3Pos[8];
	BOOL		_bProcessed;
	UINT		_uiProcessCount;
	AABBDebug()
	{
		_bProcessed = FALSE; 
		_uiProcessCount = 0;
	}

	~AABBDebug()
	{

	}
};

struct StNodeValidate 
{
	int			_iNodeIdx;
	_BV*		_bv;
	set<int>*	_setTries;
};

struct KDLineCollisionChecker
{
	/**
	* Where the collision results get stored
	*/
	//FCheckResult* Result;
	// Constant input vars
	const Vec3& Start;
	const Vec3& End;
	/**
	* Flags for optimizing a trace
	*/
	const DWORD TraceFlags;
	// Locally calculated vectors
	Vec3 LocalStart;
	Vec3 LocalEnd;
	Vec3 LocalDir;
	Vec3 LocalOneOverDir;

	// Start, End 지점을 Local로 변환하고, Normal을 구한뒤, LocalOneOverDir을 LocalDir의 1의 역수로 취한다.
	KDLineCollisionChecker(
		const Vec3& InStart,
		const Vec3& InEnd,
		DWORD InTraceFlags ) :
		Start(InStart), 
		End(InEnd), 
		TraceFlags(InTraceFlags)
	{
		// 1. WorldToLocal Matrix : identity matrix
		Matrix44 WorldToLocal;

		WorldToLocal.Identity();
		// 2. Local Start, Local End, Local Dir로 변환
		LocalStart = WorldToLocal * InStart;
		LocalEnd = WorldToLocal * InEnd;
		LocalDir = LocalEnd - LocalStart;					// LocalDir (Start->End)
		// 1의 역수로 Direction을 변환
		// Build the one over dir
		LocalOneOverDir.x = LocalDir.x ? 1.f / LocalDir.x : 0.f;		
		LocalOneOverDir.y = LocalDir.y ? 1.f / LocalDir.y : 0.f;
		LocalOneOverDir.z = LocalDir.z ? 1.f / LocalDir.z : 0.f;
	}
};
	
struct LineSegment
{
	Vec3	_v3LStart, _v3LEnd;

	Vec3	_v3DirLine, 
			_v3DirLineAbs, 
			_v3HalfLineDir, 
			_v3CenterLine;

	float	_fHalfLenLine;
public:
	void CalSegment( const Vec3 &v3LStart, const Vec3 &v3LEnd )
	{
		_v3LStart.Set( v3LStart.x, v3LStart.y, v3LStart.z );
		_v3LEnd.Set( v3LEnd.x, v3LEnd.y, v3LEnd.z );

		Vec3	v3Line			= _v3LEnd - _v3LStart;
		GetNormalize( v3Line, _v3DirLine );
		_v3DirLineAbs			= _v3DirLine.GetAbs();

		_fHalfLenLine			= v3Line.Length() / 2;
		_v3HalfLineDir			= _v3DirLine * _fHalfLenLine;
		_v3CenterLine			= _v3LStart + _v3HalfLineDir;
	}
public:
	LineSegment(){};
	~LineSegment(){};
};

class CKDTree
{
public:
	UINT			m_depth;

	UINT			m_leafNodeIdx;

	static UINT		m_uiLeafNodeSeq;

	BOOL			m_bIsLeafNode;

	int				m_axis;
	_BV				m_BV;

	CKDTree			*m_child[2];
	CKDTree			*m_parent;

	set<int>		m_triangles;

	static UINT		m_uiNodeCount;

	// For Debug
public:
	AABBDebug		AABBDEBUG;

	void BuildNodeAABB_ForDebug( vector<AABBDebug> &vKDTreeAABB );
	// For Debug

public:

	void Clear();

	BOOL DoPolygonIntersect( const Vec3* arv3Polygon, const Vec3& v3SrcPos, const Vec3& v3LEnd, Vec3& v3Intersect);
	
	void GetDimensionAsRoot( vector<CRasterTri>& vec_whole_triangle_array );

	void SplitKDNode( vector<CRasterTri>& vec_whole_triangle_array, float fOctreeRes, UINT uiNodeDepth );
	
	void Culling( vector<CRasterTri>& vec_whole_triangle_array );

	void Serialize(list<CKDTree *> &collector);

	void CollectTriangleFromNode(Vec3 &v3LEnd,Vec3 &v3LStart,set<int> &vtris);

	void CollectTriangleFromNode_LineDebug(LineSegment &_LineSeg);

	void CollectTriangleFromNode_Slab(LineSegment &_LineSeg,set<int> &vtris);

	void CollectTriangleFromNode_Slab_LineDebug(LineSegment &_LineSeg);

	void CollectVisibleNodes_Temp( Vec3 &v3TexelPos,Vec3 &v3LStart,set<int> &vtris );

	void CollectVisibleNodesChecker( Vec3 &v3TexelPos,Vec3 &v3LStart,set<int> &vtris );

	BOOL LineCollisionCheck( KDLineCollisionChecker &LineChecker);

	void Validate_CollectAllLeafNodeTriangles( list<StNodeValidate>& listNodeValidateInfo );

	void Dump(string &str,int level = 0);

	CKDTree(void);
	virtual ~CKDTree(void);
};
