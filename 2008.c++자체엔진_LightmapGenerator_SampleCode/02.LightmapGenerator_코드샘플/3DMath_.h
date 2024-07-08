
#pragma once

#include "DefMath.h"

#include "Vec_.h"

#include "Matrix__.h"


// Vector3
//==================================================================================
inline BOOL IsLightBehindSurface_( const Vec3& v3WorldLightVector,const Vec3& v3TriangleNormal, float fEpsilon = _EPSILON )
{	
	// Check if the light is in front of the triangle.
	const FLOAT Dot = DotProduct_( v3WorldLightVector, v3TriangleNormal );	// lightVector 내적 TriangleNormal
	return Dot - fEpsilon < 0.0f;										// 빛 영향을 받음.
	//return Dot < 0.0f;										// 빛 영향을 받음.
}

inline BOOL AllmostSameLinkedNormal( const Vec3& v3Normal1, 
							 const Vec3& v3Normal2, 
							 float fInvEpsilon_SimilarNormal )
{
	float fMagnitude = DotProduct_( v3Normal1, v3Normal2 );
	if( fMagnitude < fInvEpsilon_SimilarNormal ) 
	{
		return FALSE;
	}

	return TRUE;
}
//==================================================================================
// Vector3

// Matrix
//==================================================================================
Matrix44 MultMatrix (Matrix44 &l,Matrix44 &r);
Vec4 ApplyMatrixVM (Vec4 &v,Matrix44 &m);


// 이동변환 matrix를 만든다
Matrix44	MakeTranslateMatrix(float x, float y, float z);
inline Matrix44	MakeTranslateMatrix(Vec3 v) { return MakeTranslateMatrix(v.x,v.y,v.z); }
inline Matrix44	MakeTranslateMatrix(Vec4 v) { return MakeTranslateMatrix(v.x,v.y,v.z); }

// 크기변환 matrix를 만든다
Matrix44	ScalingMatrix(float x, float y, float z, float w = 1.0f);
inline Matrix44	ScalingMatrix(Vec3 v) { return ScalingMatrix(v.x,v.y,v.z); }
inline Matrix44	ScalingMatrix(Vec4 v) { return ScalingMatrix(v.x,v.y,v.z,v.w); }

// 회번변환 matrix를 만든다
// 각 축에 대하여
Matrix44	RotateXMatrix(float rad);
Matrix44	RotateYMatrix(float rad);
Matrix44	RotateZMatrix(float rad);

// Rotation Matrix를 구성한다.
Matrix44	RotateYXZMatrix_XYZ(const Vec3 &hpb);
Matrix44	RotateZYXMatrix(const Vec3 &hpb);

// 임의의 축에 대하여
Matrix44	RotateMatrix(const Vec4 &axis,float rad);

// camPos 에서 target을 바라보는 matrix를 만든다.
// 이때, camUp 방향이 위가 되도록 한다.
Matrix44	LookAtMatrix(const Vec4 &camPos,
						 const Vec4 &camUp, 
						 const Vec4 &target );

// 원근법을 적용한 투시변환을 하는 matrix를 만든다.
Matrix44	FrustumMatrix(float left, float right,
						  float bottom, float top, 
						  float znear, float zfar);

// 원근법을 적용한 투시변환을 하는 matrix를 만든다.
// fovY 는 field of view (Y) 이다.
// aspect 는 화면의 가로축과 세로축의 비율이다.
Matrix44	PerspectiveMatrix(float fovY, float aspect, 
							  float n, float f);

// 원근법을 적용하지않은 투시변환을 하는 matrix를 만든다.
Matrix44	OrthoMatrix(float left, float right,
						float bottom, float top, 
						float znear, float zfar);

//==================================================================================
// Matrix

