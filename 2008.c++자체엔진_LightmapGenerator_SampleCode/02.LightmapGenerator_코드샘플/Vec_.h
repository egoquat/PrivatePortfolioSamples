#pragma once

#include "Common_.h"

#include "DefMath.h"

#include "Exception_.h"

// ==============================================================================================================
// 2D Vector
// ==============================================================================================================
class Vec2
{
public:
	float x,y;

	Vec2(void)
	{
		Set(0,0);
	}

	Vec2(float a_x,float a_y)
	{
		Set(a_x,a_y);
	}

	static Vec2 *Create()
	{
		return new Vec2;
	}


	float *f(void)
	{
		return &x;
	}

	void Set(float a_x,float a_y)
	{
		x	=				a_x;
		y	=				a_y;
	}

	float LengthSquare() const
	{
		return (x * x + y * y);
	}

	float Length() const
	{
		return (float) sqrt( LengthSquare() );
	}

	float SafeNormalize(void)
	{
		float size = Length();

		if( _TOOSMALL_EPSILON > size )
		{
			x = 0; y = 0;
		}
		else
		{
			*this /= size;
		}

		return size;
	}

	void Invert(void)
	{
		x = -x;
		y = -y;
	}

	float GetMax(void)
	{
		return max( x, y );
	}

	float GetMin(void)
	{
		return min( x, y );
	}

	float &operator[](int index)
	{
		if (index<0 || index>1)
			Throw__(Exception__index_out_of_range);

		return ((float*)this)[index];
	}

	const float &operator[](int index) const
	{
		if (index<0 || index>1)
			Throw__(Exception__index_out_of_range);

		return ((float*)this)[index];
	}

	const Vec2 &operator +=(const Vec2 &a_v)
	{
		x	+=	a_v.x;
		y	+=	a_v.y;

		return *this;
	}

	const Vec2 &operator -=(const Vec2 &a_v)
	{
		x	-=	a_v.x;
		y	-=	a_v.y;

		return *this;
	}

	const Vec2 &operator /=(float a_v)
	{
		// 0으로 나눌 수는 없음
		if (a_v==0.0f) return *this;

		x	/=	a_v;
		y	/=	a_v;

		return *this;
	}

	const Vec2 &operator *=(float a_v)
	{
		x	*=	a_v;
		y	*=	a_v;

		return *this;
	}


	const Vec2 &operator =(const Vec2 &rhs)
	{
		x	=	rhs.x;
		y	=	rhs.y;

		return *this;
	}

	bool operator ==(const Vec2 &rhs)
	{
		if (x	!=	rhs.x) return false;
		if (y	!=	rhs.y) return false;

		return true;
	}

	bool operator !=(const Vec2 &rhs)
	{
		if (x	!=	rhs.x) return true;
		if (y	!=	rhs.y) return true;

		return false;
	}
};

// v 는 vector, s 는 scala
// v = v + v
inline
Vec2 operator +(const Vec2 &lhs,const Vec2 &rhs)
{
	Vec2 rv;

	rv.x	=	lhs.x	+	rhs.x;
	rv.y	=	lhs.y	+	rhs.y;

	return rv;
}

// v = v - v
inline
Vec2 operator -(const Vec2 &lhs,const Vec2 &rhs)
{
	Vec2 rv;

	rv.x	=	lhs.x	-	rhs.x;
	rv.y	=	lhs.y	-	rhs.y;

	return rv;
}

// v = s * v
inline
Vec2 operator *(const float &lhs,const Vec2 &rhs)
{
	Vec2 rv;

	rv.x	=	lhs		*	rhs.x;
	rv.y	=	lhs		*	rhs.y;

	return rv;
}

// v = v * s
inline
Vec2 operator *(const Vec2 &lhs,const float &rhs)
{
	Vec2 rv;

	rv.x	=	lhs.x		*	rhs;
	rv.y	=	lhs.y		*	rhs;

	return rv;
}

// v = v / s
inline
Vec2 operator /(const Vec2 &lhs,const float &rhs)
{
	// 0으로 나눌 수는 없음
	if (rhs==0.0f) return lhs;

	Vec2 rv;

	rv.x	=	lhs.x		*	rhs;
	rv.y	=	lhs.y		*	rhs;

	return rv;
}

inline
float DotProduct_(const Vec2 &lhs,const Vec2 &rhs)
{
	float rv;

	rv		=	lhs.x	*	rhs.x
		+	lhs.y	*	rhs.y;
	return rv;
}


// s = v . v
inline
float operator ^(const Vec2 &lhs,const Vec2 &rhs)
{
	float rv;

	rv = DotProduct_(lhs,rhs);

	return rv;
}

inline
BOOL VectorsAllmostSame( const Vec2& v2_1, const Vec2& v2_2, float fComparison = _EPSILON )
{
	Vec2& v2Comp = v2_1 - v2_2;

	if( _EPSILON < abs(v2Comp.x) || _EPSILON < abs(v2Comp.y) )
	{
		return FALSE;
	}

	return TRUE;
}
// ==============================================================================================================
// /까지 2D Vector
// ==============================================================================================================



// ==============================================================================================================
// 3D Vector
// ==============================================================================================================
class Vec3
{
public:
	float x,y,z;

	Vec3(void)
	{
		Set(0,0,0);
	}

	Vec3(float a_x,float a_y,float a_z)
	{
		Set(a_x,a_y,a_z);
	}

	static Vec3 *Create()
	{
		return new Vec3;
	}


	float *f(void)
	{
		return &x;
	}

	void Set(float a_x,float a_y,float a_z)
	{
		x	=				a_x;
		y	=				a_y;
		z	=				a_z;
	}

	void SetXZ( float a_x, float a_z )
	{
		x	=				a_x;
		z	=				a_z;
	}

	void SetXZ( const Vec2 &a_v)
	{
		x	=				a_v.x;
		z	=				a_v.y;
	}

	Vec2 GetXZ() const
	{
		return Vec2( x, z );
	}

	bool IsAllZero() const
	{
		return ( x == 0.0f && y == 0.0f && z == 0.0f );
	}

	float SafeNormalize(void)
	{
		float size = Length();

		if( size == 1.f )
		{
			return 1.f;
		}
		else if( size < _TOOSMALL_EPSILON ) 
		{
			x = y = z = 0.f;
		}
		else
		{
			*this /= size;
		}

		return size;
	}

	void Invert(void)
	{
		x = -x;
		y = -y;
		z = -z;
	}

	float Length() const
	{
		return (float) fabs(sqrt( LengthSquare() ));
	}

	const float LengthSquare( void ) const
	{
		return ( x * x + y * y + z * z);
	}

	float GetMax(void)
	{
		return max( max( x, y ), z );
	}

	float GetMin(void)
	{
		return min( min( x, y ), z );
	}

	void SetAbs(void)
	{
		x = fabs(x);
		y = fabs(y);
		z = fabs(z);
	}

	const Vec3 GetAbs(void)
	{
		return Vec3(fabs(x), fabs(y), fabs(z));
	}

	const Vec3 operator -(void) const
	{
		return Vec3(-x,-y,-z);
	}

	float &operator[](int index)
	{
		if (index<0 || index>2)
			Throw__(Exception__index_out_of_range);

		return ((float*)this)[index];
	}

	const float &operator[](int index) const
	{
		if (index<0 || index>2)
			Throw__(Exception__index_out_of_range);

		return ((float*)this)[index];
	}

	const Vec3 &operator +=(const Vec3 &a_v)
	{
		x	+=	a_v.x;
		y	+=	a_v.y;
		z	+=	a_v.z;

		return *this;
	}

	const Vec3 &operator -=(const Vec3 &a_v)
	{
		x	-=	a_v.x;
		y	-=	a_v.y;
		z	-=	a_v.z;

		return *this;
	}

	const Vec3 &operator /=(float a_v)
	{
		// 0으로 나눌 수는 없음
		if (a_v==0.0f) return *this;

		x	/=	a_v;
		y	/=	a_v;
		z	/=	a_v;

		return *this;
	}

	const Vec3 &operator *=(float a_v)
	{
		x	*=	a_v;
		y	*=	a_v;
		z	*=	a_v;

		return *this;
	}


	const Vec3 &operator =(const Vec3 &rhs)
	{
		x	=	rhs.x;
		y	=	rhs.y;
		z	=	rhs.z;

		return *this;
	}

	bool operator ==(const Vec3 &rhs) const
	{
		if (x	!=	rhs.x) return false;
		if (y	!=	rhs.y) return false;
		if (z	!=	rhs.z) return false;

		return true;
	}

	bool operator !=(const Vec3 &rhs) const 
	{
		if (x	!=	rhs.x) return true;
		if (y	!=	rhs.y) return true;
		if (z	!=	rhs.z) return true;

		return false;
	}
};


// v 는 vector, s 는 scala
// v = v + v
inline
Vec3 operator +(const Vec3 &lhs,const Vec3 &rhs)
{
	Vec3 rv;

	rv.x	=	lhs.x	+	rhs.x;
	rv.y	=	lhs.y	+	rhs.y;
	rv.z	=	lhs.z	+	rhs.z;

	return rv;
}

// v = v - v
inline
Vec3 operator -(const Vec3 &lhs,const Vec3 &rhs)
{
	Vec3 rv;

	rv.x	=	lhs.x	-	rhs.x;
	rv.y	=	lhs.y	-	rhs.y;
	rv.z	=	lhs.z	-	rhs.z;

	return rv;
}

// v = s * v
inline
Vec3 operator *(const float &lhs,const Vec3 &rhs)
{
	Vec3 rv;

	rv.x	=	lhs		*	rhs.x;
	rv.y	=	lhs		*	rhs.y;
	rv.z	=	lhs		*	rhs.z;

	return rv;
}

// v = v * s
inline
Vec3 operator *(const Vec3 &lhs,const float &rhs)
{
	Vec3 rv;

	rv.x	=	lhs.x		*	rhs;
	rv.y	=	lhs.y		*	rhs;
	rv.z	=	lhs.z		*	rhs;

	return rv;
}

// v = v / s
inline
Vec3 operator /(const Vec3 &lhs,const float &rhs)
{
	// 0으로 나눌 수는 없음
	if (rhs==0.0f) return lhs;

	Vec3 rv;

	rv.x	=	lhs.x		/	rhs;
	rv.y	=	lhs.y		/	rhs;
	rv.z	=	lhs.z		/	rhs;

	return rv;
}

inline
float DotProduct_(const Vec3 &lhs,const Vec3 &rhs)
{
	float rv;

	rv		=	lhs.x	*	rhs.x
		+	lhs.y	*	rhs.y
		+	lhs.z	*	rhs.z;
	return rv;
}

inline
const Vec3 CrossProduct_(const Vec3 &lhs,const Vec3 &rhs)
{
	Vec3 rv;

	rv.x	=	lhs.y*rhs.z - lhs.z*rhs.y;
	rv.y	=	lhs.z*rhs.x - lhs.x*rhs.z;
	rv.z	=	lhs.x*rhs.y - lhs.y*rhs.x;

	return rv;
}

inline
void CrossProduct_( const Vec3 &lhs,const Vec3 &rhs, Vec3& v3Return )
{
	v3Return.x	=	lhs.y*rhs.z - lhs.z*rhs.y;
	v3Return.y	=	lhs.z*rhs.x - lhs.x*rhs.z;
	v3Return.z	=	lhs.x*rhs.y - lhs.y*rhs.x;
}

// s = v . v
inline
float operator ^(const Vec3 &lhs,const Vec3 &rhs)
{
	float rv;

	rv = DotProduct_(lhs,rhs);

	return rv;
}

// v = v x v
inline
Vec3 operator %(const Vec3 &lhs,const Vec3 &rhs)
{
	Vec3 rv;

	rv = CrossProduct_(lhs,rhs);

	return rv;
}

inline
BOOL VectorsAllmostSame( const Vec3& v3_1, const Vec3& v3_2, float fComparison = _EPSILON )
{
	Vec3& v3Comp = v3_1 - v3_2;

	if( _EPSILON < abs(v3Comp.x) || _EPSILON < abs(v3Comp.y) || _EPSILON < abs(v3Comp.z) )
	{
		return FALSE;
	}

	return TRUE;
}
// ==============================================================================================================
// /까지 3D Vector
// ==============================================================================================================



// ==============================================================================================================
// 4D Vector
// ==============================================================================================================
class Vec4
{
public:
	float x,y,z,w;

	Vec4(void)
	{
		Set(0,0,0);
	}

	Vec4(const Vec3 &r)
	{
		(*this) = r;
	}

	Vec4(float a_x,float a_y,float a_z,float a_w = 1.0f)
	{
		Set(a_x,a_y,a_z,a_w);
	}

	static Vec4 *Create()
	{
		return new Vec4;
	}


	float *f(void)
	{
		return &x;
	}

	// <- 결과값을 배열로 사용하면 대략 낭패
	Vec3 &v3(void)
	{
		return *((Vec3*)(this));
	}

	const Vec3 &v3(void) const
	{
		return *((const Vec3*)(this));
	}

	void Set(float a_x,float a_y,float a_z,float a_w = 1.0f)
	{
		x	=				a_x;
		y	=				a_y;
		z	=				a_z;
		w	=				a_w;
	}

	float LengthSquare(void)
	{
		float rv;
		rv	=	x * x
			+	y * y
			+	z * z;

		return rv;
	}

	float Length(void)
	{
		return sqrtf(LengthSquare());
	}

	float SafeNormalize(void)
	{
		float size = Length();

		if( size == 1.f )
		{
			return 1.f;
		}
		else if( size < _TOOSMALL_EPSILON )
		{
			x = y = z = 0.f;
		}
		else
		{
			*this /= size;
		}

		return size;
	}

	void Invert(void)
	{
		x = -x;
		y = -y;
		z = -z;
		w = -w;
	}

	float GetMax(void)
	{
		return max( max( x, y ), z );
	}

	float GetMin(void)
	{
		return min( min( x, y ), z );
	}


	float &operator[](int index)
	{
		if (index<0 || index>3)
			Throw__(Exception__index_out_of_range);

		return ((float*)this)[index];
	}

	const float &operator[](int index) const
	{
		if (index<0 || index>3)
			Throw__(Exception__index_out_of_range);

		return ((float*)this)[index];
	}

	const Vec4 &operator +=(const Vec4 &a_v)
	{
		x	+=	a_v.x;
		y	+=	a_v.y;
		z	+=	a_v.z;

		return *this;
	}

	const Vec4 &operator -=(const Vec4 &a_v)
	{
		x	-=	a_v.x;
		y	-=	a_v.y;
		z	-=	a_v.z;

		return *this;
	}

	const Vec4 &operator /=(float a_v)
	{
		// 0으로 나눌 수는 없음
		if (a_v==0.0f) return *this;

		x	/=	a_v;
		y	/=	a_v;
		z	/=	a_v;

		return *this;
	}

	const Vec4 &operator *=(float a_v)
	{
		x	*=	a_v;
		y	*=	a_v;
		z	*=	a_v;

		return *this;
	}


	const Vec4 &operator =(const Vec4 &rhs)
	{
		x	=	rhs.x;
		y	=	rhs.y;
		z	=	rhs.z;
		w	=	rhs.w;

		return *this;
	}

	const Vec4 &operator =(const Vec3 &rhs)
	{
		x	=	rhs.x;
		y	=	rhs.y;
		z	=	rhs.z;
		w	=	1;

		return *this;
	}

	bool operator ==(const Vec4 &rhs)
	{
		if (x	!=	rhs.x) return false;
		if (y	!=	rhs.y) return false;
		if (z	!=	rhs.z) return false;
		if (w	!=	rhs.w) return false;

		return true;
	}

	bool operator !=(const Vec4 &rhs)
	{
		if (x	!=	rhs.x) return true;
		if (y	!=	rhs.y) return true;
		if (z	!=	rhs.z) return true;
		if (w	!=	rhs.w) return true;

		return false;
	}
};

// v 는 vector, s 는 scala
// v = v + v
inline
Vec4 operator +(const Vec4 &lhs,const Vec4 &rhs)
{
	Vec4 rv;

	rv.x	=	lhs.x	+	rhs.x;
	rv.y	=	lhs.y	+	rhs.y;
	rv.z	=	lhs.z	+	rhs.z;

	return rv;
}

// v = v - v
inline
Vec4 operator -(const Vec4 &lhs,const Vec4 &rhs)
{
	Vec4 rv;

	rv.x	=	lhs.x	-	rhs.x;
	rv.y	=	lhs.y	-	rhs.y;
	rv.z	=	lhs.z	-	rhs.z;

	return rv;
}

// v = s * v
inline
Vec4 operator *(const float &lhs,const Vec4 &rhs)
{
	Vec4 rv;

	rv.x	=	lhs		*	rhs.x;
	rv.y	=	lhs		*	rhs.y;
	rv.z	=	lhs		*	rhs.z;

	return rv;
}

// v = v * s
inline
Vec4 operator *(const Vec4 &lhs,const float &rhs)
{
	Vec4 rv;

	rv.x	=	lhs.x		*	rhs;
	rv.y	=	lhs.y		*	rhs;
	rv.z	=	lhs.z		*	rhs;

	return rv;
}

// v = v / s
inline
Vec4 operator /(const Vec4 &lhs,const float &rhs)
{
	// 0으로 나눌 수는 없음
	if (rhs==0.0f) return lhs;

	Vec4 rv;

	rv.x	=	lhs.x		/	rhs;
	rv.y	=	lhs.y		/	rhs;
	rv.z	=	lhs.z		/	rhs;

	return rv;
}

inline
float DotProduct_(const Vec4 &lhs,const Vec4 &rhs)
{
	float rv;

	rv		=	lhs.x	*	rhs.x
		+	lhs.y	*	rhs.y
		+	lhs.z	*	rhs.z;
	//+	lhs.w	*	rhs.w;
	return rv;
}

inline
float Dot4w(const Vec4 &lhs,const Vec4 &rhs)
{
	float rv;

	rv		=	lhs.x	*	rhs.x
		+	lhs.y	*	rhs.y
		+	lhs.z	*	rhs.z
		+	lhs.w	*	rhs.w;
	return rv;
}

inline
float DotProduct_(const Vec4 &lhs,const Vec3 &rhs)
{
	float rv;

	rv		=	lhs.x	*	rhs.x
		+	lhs.y	*	rhs.y
		+	lhs.z	*	rhs.z;
	return rv;
}

inline
float DotProduct_(const Vec3 &lhs,const Vec4 &rhs)
{
	float rv;

	rv		=	lhs.x	*	rhs.x
		+	lhs.y	*	rhs.y
		+	lhs.z	*	rhs.z;
	return rv;
}

inline
const Vec4 CrossProduct_(const Vec4 &lhs,const Vec4 &rhs)
{
	Vec4 rv;

	rv.x	=	lhs.y*rhs.z - lhs.z*rhs.y;
	rv.y	=	lhs.z*rhs.x - lhs.x*rhs.z;
	rv.z	=	lhs.x*rhs.y - lhs.y*rhs.x;

	return rv;
}

// s = v . v
inline
float operator ^(const Vec4 &lhs,const Vec4 &rhs)
{
	float rv;

	rv = DotProduct_(lhs,rhs);

	return rv;
}

// v = v x v
inline
Vec4 operator %(const Vec4 &lhs,const Vec4 &rhs)
{
	Vec4 rv;

	rv = CrossProduct_(lhs,rhs);

	return rv;
}

inline
BOOL VectorsAllmostSame( const Vec4 &v4_1, const Vec4 &v4_2, float fComparison = _EPSILON )
{
	Vec4&	v4Comp = v4_1 - v4_2;

	if( _EPSILON < abs(v4Comp.x) 
		|| _EPSILON < abs(v4Comp.y) 
		|| _EPSILON < abs(v4Comp.z) )
	{
		return FALSE;
	}

	return TRUE;
}
// ==============================================================================================================
// /까지 4D Vector
// ==============================================================================================================


// ==============================================================================================================
// Simplely Vector Calculation
// ==============================================================================================================
inline void CrossNormal_( Vec3& v3TriNormal, const Vec3* const arPos )
{
	// 무게중심 // 위치 중심
	Vec3 center = arPos[0] + arPos[1] + arPos[2];
	center /= 3.0f;

	// getting triangle normal
	Vec3 face_normal,a,b;
	a = arPos[1]-arPos[0];
	b = arPos[2]-arPos[0];

	if( a.Length() < _EPSILON && b.Length() < _EPSILON )
	{
		v3TriNormal.Set( 0,0,0 );
	}
	else
	{
		CrossProduct_(a,b,v3TriNormal);
		v3TriNormal.SafeNormalize();
	}
}

inline void CrossNormal_( Vec3& v3TriNormal, const Vec3 &Pos1, const Vec3 &Pos2, const Vec3 &Pos3 )
{
	// 무게중심 // 위치 중심
	Vec3 center = Pos1 + Pos2 + Pos3;
	center /= 3.0f;

	// getting triangle normal
	Vec3 face_normal,a,b;
	a = Pos2 - Pos1;
	b = Pos3 - Pos1;

	if( a.Length() < _EPSILON && b.Length() < _EPSILON )
	{
		v3TriNormal.Set( 0,0,0 );
	}
	else
	{
		CrossProduct_(a,b,v3TriNormal);
		v3TriNormal.SafeNormalize();
	}
}

inline void GetNormalize( const Vec2 &v2Dir, Vec2 &v2Return )
{
	v2Return.Set( v2Dir.x, v2Dir.y );
	v2Return.SafeNormalize();
}

inline void GetNormalize( const Vec3 &v3Dir, Vec3 &v3Return )
{
	v3Return.Set( v3Dir.x, v3Dir.y, v3Dir.z );
	v3Return.SafeNormalize();
}

inline void GetNormalize( const Vec4 &v4Dir, Vec4 &v4Return )
{
	v4Return.Set( v4Dir.x, v4Dir.y, v4Dir.z );
	v4Return.SafeNormalize();
}
