#pragma once

#include "Vec_.h"


// ==============================================================================================================
// 4 By 4 Materix
// ==============================================================================================================
class Matrix44
{
public:
	Vec4 col[4];

	Matrix44() {}
	Matrix44(const Matrix44 &m)
	{
		Assign(m);
	}
	void	Assign(const Matrix44 &m)
	{
		col[0] = m.col[0];
		col[1] = m.col[1];
		col[2] = m.col[2];
		col[3] = m.col[3];
	}


	// float 형태로 돌려줌
	float *f(void)
	{
		return &(col[0][0]);
	}

	Matrix44 &operator =(const Matrix44 &rhs)
	{
		Assign(rhs);

		return *this;
	}


	Matrix44	operator * (const Matrix44 &b)
	{
		Matrix44 ret(*this);

		ret *= b;

		return ret;
	}


	Vec4 	operator * (const Vec4 &v)
	{
		Vec4 ret;

		ret.x =	(*this)[0][0]*v[0] + (*this)[0][1]*v[1] + (*this)[0][2]*v[2] + (*this)[0][3]*v[3];
		ret.y =	(*this)[1][0]*v[0] + (*this)[1][1]*v[1] + (*this)[1][2]*v[2] + (*this)[1][3]*v[3];
		ret.z =	(*this)[2][0]*v[0] + (*this)[2][1]*v[1] + (*this)[2][2]*v[2] + (*this)[2][3]*v[3];
		ret.w = (*this)[3][0]*v[0] + (*this)[3][1]*v[1] + (*this)[3][2]*v[2] + (*this)[3][3]*v[3];

		return ret;
	}


	Vec3 	operator * (const Vec3 &v)
	{
		Vec3 ret;

		ret.x =	(*this)[0][0]*v[0] + (*this)[1][0]*v[1] + (*this)[2][0]*v[2] + (*this)[3][0];
		ret.y =	(*this)[0][1]*v[0] + (*this)[1][1]*v[1] + (*this)[2][1]*v[2] + (*this)[3][1];
		ret.z =	(*this)[0][2]*v[0] + (*this)[1][2]*v[1] + (*this)[2][2]*v[2] + (*this)[3][2];

		return ret;
	}

	Matrix44	operator * (float f)
	{
		Matrix44 ret(*this);

		ret *= f;

		return ret;
	}

	Matrix44	operator / (float f)
	{
		Matrix44 ret(*this);

		ret /= f;

		return ret;
	}

	Matrix44	operator + (const Matrix44 &rhs)
	{
		Matrix44 ret(*this);

		ret += rhs;

		return ret;
	}

	Matrix44	operator - (const Matrix44 &rhs)
	{
		Matrix44 ret(*this);

		ret -= rhs;

		return ret;
	}



	Vec4		&operator [] (int index)
	{
		if (index<0 || index>3)
			Throw__(Exception__index_out_of_range);

		return (Vec4&) col[index];
	}

	const Vec4		&operator [] (int index) const
	{
		if (index<0 || index>3)
			Throw__(Exception__index_out_of_range);

		return (Vec4&) col[index];
	}


	// 다른 matrix를 더한다.
	Matrix44		&operator += (const Matrix44 &rhs)
	{
		int r,c;

		for (r = 0; r < 4; r++) 
		{
			for (c = 0; c < 4; c++) 
			{
				(*this)[r][c] += rhs[r][c];
			}
		}

		return *this;
	}

	// 다른 matrix를 뺀다.
	Matrix44		&operator -= (const Matrix44 &rhs)
	{
		int r,c;

		for (r = 0; r < 4; r++) 
		{
			for (c = 0; c < 4; c++) 
			{
				(*this)[r][c] -= rhs[r][c];
			}
		}

		return *this;
	}


	// 다른 matrix를 뒤에 곱한다.
	Matrix44		&operator *= (const Matrix44 &rhs)
	{
		Matrix44 t;

		int r,c;

		for (c = 0; c < 4; c++) 
		{
			for (r = 0; r < 4; r++) 
			{
				float f = 0;

				f += col[0][r] * rhs[c][0];
				f += col[1][r] * rhs[c][1];
				f += col[2][r] * rhs[c][2];
				f += col[3][r] * rhs[c][3];

				t[c][r] = f;
			}
		}


		*this = t;

		return *this;
	}



	Matrix44 &mult (const Matrix44 &rhs)
	{
		*this *= rhs;

		return *this;
	}

	// 모든 항에 상수를 곱한다.
	Matrix44		&operator *= (const float f)
	{
		for( int i = 0; i < 4; ++i )
		{
			for( int j = 0; j < 4; ++j )
			{
				col[i][j] *= f;
			}
		}

		return *this;
	}


	// 각 항을 상수로 나눈다.
	Matrix44		&operator /= (const float rhs)
	{
		int r,c;

		for (r = 0; r < 4; r++) 
		{
			for (c = 0; c < 4; c++) 
			{
				col[r][c] /= rhs;
			}
		}

		return *this;
	}

	// 항등행렬로 만든다
	Matrix44		&Identity()
	{
		col[0].Set(1,0,0,0);
		col[1].Set(0,1,0,0);
		col[2].Set(0,0,1,0);
		col[3].Set(0,0,0,1);

		return *this;
	}

	// 전치 행렬로 만든다
	Matrix44		&Transpose()
	{
		float t;
		int r,c;

		for (c = 0; c < 4; c++) 
		{
			for (r = c + 1; r < 4; r++) 
			{
				t = col[c][r];
				col[c][r] = col[r][c];
				col[r][c] = t;
			} 
		} 

		return *this;
	}

	// 역행렬을 구한다.
	// 자기 자신을 바꿔 치도록 되어있으므로 주의!
	// - intel의 문서를 참조하였음
	// AP-928 Streaming SIMD Externsions - Inverse of 4x4 Matrix44
	// March 1999
	// http://www.intel.com
	Matrix44 &Invert( void )
	{
		Matrix44 rv;
		float *mat, *dst;

		mat = &(col[0][0]);
		dst = &(rv[0][0]);

		float tmp[12]; /* temp array for pairs */
		float src[16]; /* array of transpose source matrix */
		float det; /* determinant */
		/* transpose matrix */
		for ( int i = 0; i < 4; ++i) {
			src[i] = mat[i*4];
			src[i + 4] = mat[i*4 + 1];
			src[i + 8] = mat[i*4 + 2];
			src[i + 12] = mat[i*4 + 3];
		}
		/* calculate pairs for first 8 elements (cofactors) */
		tmp[0] = src[10] * src[15];
		tmp[1] = src[11] * src[14];
		tmp[2] = src[9] * src[15];
		tmp[3] = src[11] * src[13];
		tmp[4] = src[9] * src[14];
		tmp[5] = src[10] * src[13];
		tmp[6] = src[8] * src[15];
		tmp[7] = src[11] * src[12];
		tmp[8] = src[8] * src[14];
		tmp[9] = src[10] * src[12];
		tmp[10] = src[8] * src[13];
		tmp[11] = src[9] * src[12];
		/* calculate first 8 elements (cofactors) */
		dst[0] = tmp[0]*src[5] + tmp[3]*src[6] + tmp[4]*src[7];
		dst[0] -= tmp[1]*src[5] + tmp[2]*src[6] + tmp[5]*src[7];
		dst[1] = tmp[1]*src[4] + tmp[6]*src[6] + tmp[9]*src[7];
		dst[1] -= tmp[0]*src[4] + tmp[7]*src[6] + tmp[8]*src[7];
		dst[2] = tmp[2]*src[4] + tmp[7]*src[5] + tmp[10]*src[7];
		dst[2] -= tmp[3]*src[4] + tmp[6]*src[5] + tmp[11]*src[7];
		dst[3] = tmp[5]*src[4] + tmp[8]*src[5] + tmp[11]*src[6];
		dst[3] -= tmp[4]*src[4] + tmp[9]*src[5] + tmp[10]*src[6];
		dst[4] = tmp[1]*src[1] + tmp[2]*src[2] + tmp[5]*src[3];
		dst[4] -= tmp[0]*src[1] + tmp[3]*src[2] + tmp[4]*src[3];
		dst[5] = tmp[0]*src[0] + tmp[7]*src[2] + tmp[8]*src[3];
		dst[5] -= tmp[1]*src[0] + tmp[6]*src[2] + tmp[9]*src[3];
		dst[6] = tmp[3]*src[0] + tmp[6]*src[1] + tmp[11]*src[3];
		dst[6] -= tmp[2]*src[0] + tmp[7]*src[1] + tmp[10]*src[3];
		dst[7] = tmp[4]*src[0] + tmp[9]*src[1] + tmp[10]*src[2];
		dst[7] -= tmp[5]*src[0] + tmp[8]*src[1] + tmp[11]*src[2];
		/* calculate pairs for second 8 elements (cofactors) */
		tmp[0] = src[2]*src[7];
		tmp[1] = src[3]*src[6];
		tmp[2] = src[1]*src[7];
		tmp[3] = src[3]*src[5];
		tmp[4] = src[1]*src[6];
		tmp[5] = src[2]*src[5];
		tmp[6] = src[0]*src[7];
		tmp[7] = src[3]*src[4];
		tmp[8] = src[0]*src[6];
		tmp[9] = src[2]*src[4];
		tmp[10] = src[0]*src[5];
		tmp[11] = src[1]*src[4];
		/* calculate second 8 elements (cofactors) */
		dst[8] = tmp[0]*src[13] + tmp[3]*src[14] + tmp[4]*src[15];
		dst[8] -= tmp[1]*src[13] + tmp[2]*src[14] + tmp[5]*src[15];
		dst[9] = tmp[1]*src[12] + tmp[6]*src[14] + tmp[9]*src[15];
		dst[9] -= tmp[0]*src[12] + tmp[7]*src[14] + tmp[8]*src[15];
		dst[10] = tmp[2]*src[12] + tmp[7]*src[13] + tmp[10]*src[15];
		dst[10]-= tmp[3]*src[12] + tmp[6]*src[13] + tmp[11]*src[15];
		dst[11] = tmp[5]*src[12] + tmp[8]*src[13] + tmp[11]*src[14];
		dst[11]-= tmp[4]*src[12] + tmp[9]*src[13] + tmp[10]*src[14];
		dst[12] = tmp[2]*src[10] + tmp[5]*src[11] + tmp[1]*src[9];
		dst[12]-= tmp[4]*src[11] + tmp[0]*src[9] + tmp[3]*src[10];
		dst[13] = tmp[8]*src[11] + tmp[0]*src[8] + tmp[7]*src[10];
		dst[13]-= tmp[6]*src[10] + tmp[9]*src[11] + tmp[1]*src[8];
		dst[14] = tmp[6]*src[9] + tmp[11]*src[11] + tmp[3]*src[8];
		dst[14]-= tmp[10]*src[11] + tmp[2]*src[8] + tmp[7]*src[9];
		dst[15] = tmp[10]*src[10] + tmp[4]*src[8] + tmp[9]*src[9];
		dst[15]-= tmp[8]*src[9] + tmp[11]*src[10] + tmp[5]*src[8];
		/* calculate determinant */
		det=src[0]*dst[0]+src[1]*dst[1]+src[2]*dst[2]+src[3]*dst[3];
		/* calculate matrix inverse */
		det = 1/det;
		for ( int j = 0; j < 16; ++j)
			dst[j] *= det;

		*this = rv;

		return *this;
	}

	// 역행열로 만든다
	// 회전과 이동 그리고, 반사에만 사용할 수 있다.
	// 이외의 변환을 사용했을 경우는 사용할 수 없음
	Matrix44		&FastInvert()
	{
		Matrix44 rv;

		rv[0].Set(col[0][0],col[1][0],col[2][0],0.0f);
		rv[1].Set(col[0][1],col[1][1],col[2][1],0.0f);
		rv[2].Set(col[0][2],col[1][2],col[2][2],0.0f);

		rv[3][0] = -(col[3]^col[0]);
		rv[3][1] = -(col[3]^col[1]);
		rv[3][2] = -(col[3]^col[2]);
		rv[3][3] = 1.0f;

		*this = rv;

		return *this;
	}

	Matrix44 &TransitionTo(Matrix44 &to,float fraction)
	{
		for( int i = 0; i < 4; ++i )
		{
			for( int j = 0; j < 4; ++j )
			{
				col[i][j] += (to[i][j] - col[i][j]) * fraction;
			}
		}

		return *this;
	}
};
// ==============================================================================================================
// /여기 까지 Matrix44
// ==============================================================================================================
