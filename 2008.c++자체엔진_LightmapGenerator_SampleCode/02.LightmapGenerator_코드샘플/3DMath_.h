
#pragma once

#include "DefMath.h"

#include "Vec_.h"

#include "Matrix__.h"


// Vector3
//==================================================================================
inline BOOL IsLightBehindSurface_( const Vec3& v3WorldLightVector,const Vec3& v3TriangleNormal, float fEpsilon = _EPSILON )
{	
	// Check if the light is in front of the triangle.
	const FLOAT Dot = DotProduct_( v3WorldLightVector, v3TriangleNormal );	// lightVector ���� TriangleNormal
	return Dot - fEpsilon < 0.0f;										// �� ������ ����.
	//return Dot < 0.0f;										// �� ������ ����.
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


// �̵���ȯ matrix�� �����
Matrix44	MakeTranslateMatrix(float x, float y, float z);
inline Matrix44	MakeTranslateMatrix(Vec3 v) { return MakeTranslateMatrix(v.x,v.y,v.z); }
inline Matrix44	MakeTranslateMatrix(Vec4 v) { return MakeTranslateMatrix(v.x,v.y,v.z); }

// ũ�⺯ȯ matrix�� �����
Matrix44	ScalingMatrix(float x, float y, float z, float w = 1.0f);
inline Matrix44	ScalingMatrix(Vec3 v) { return ScalingMatrix(v.x,v.y,v.z); }
inline Matrix44	ScalingMatrix(Vec4 v) { return ScalingMatrix(v.x,v.y,v.z,v.w); }

// ȸ����ȯ matrix�� �����
// �� �࿡ ���Ͽ�
Matrix44	RotateXMatrix(float rad);
Matrix44	RotateYMatrix(float rad);
Matrix44	RotateZMatrix(float rad);

// Rotation Matrix�� �����Ѵ�.
Matrix44	RotateYXZMatrix_XYZ(const Vec3 &hpb);
Matrix44	RotateZYXMatrix(const Vec3 &hpb);

// ������ �࿡ ���Ͽ�
Matrix44	RotateMatrix(const Vec4 &axis,float rad);

// camPos ���� target�� �ٶ󺸴� matrix�� �����.
// �̶�, camUp ������ ���� �ǵ��� �Ѵ�.
Matrix44	LookAtMatrix(const Vec4 &camPos,
						 const Vec4 &camUp, 
						 const Vec4 &target );

// ���ٹ��� ������ ���ú�ȯ�� �ϴ� matrix�� �����.
Matrix44	FrustumMatrix(float left, float right,
						  float bottom, float top, 
						  float znear, float zfar);

// ���ٹ��� ������ ���ú�ȯ�� �ϴ� matrix�� �����.
// fovY �� field of view (Y) �̴�.
// aspect �� ȭ���� ������� �������� �����̴�.
Matrix44	PerspectiveMatrix(float fovY, float aspect, 
							  float n, float f);

// ���ٹ��� ������������ ���ú�ȯ�� �ϴ� matrix�� �����.
Matrix44	OrthoMatrix(float left, float right,
						float bottom, float top, 
						float znear, float zfar);

//==================================================================================
// Matrix

