#pragma once



class apXy
{
public:
	int x,y;

	apXy()
	{
	}

	apXy(int a_x,int a_y)
	{
		x = a_x;
		y = a_y;
	}

	void Set(int a_x,int a_y)
	{
		x = a_x;
		y = a_y;
	}

	bool operator ==(const apXy &rhs)
	{
		if (x	!=	rhs.x) return false;
		if (y	!=	rhs.y) return false;
		
		return true;
	}
	
	bool operator !=(const apXy &rhs)
	{
		if (x	==	rhs.x) return false;
		if (y	==	rhs.y) return false;
		
		return true;
	}
};

template<typename T>
class Rect__
{
public:
	T l,t,r,b; // left,top,right,bottom

	Rect__() :
	l(0),
	t(0),
	r(0),
	b(0)
	{}

	Rect__(T a_l,T a_t,T a_r,T a_b) :
	l(a_l),
	t(a_t),
	r(a_r),
	b(a_b)
	{}

	void Set(T a_l,T a_t,T a_r,T a_b)
	{
		l = a_l;
		t = a_t;
		r = a_r;
		b = a_b;
	}

	void SetHW(T x,T y,T _w,T _h)
	{
		l = x;
		t = y;
		r = x+_w;
		b = y+_h;
	}

	void Rotate()
	{
		T ww = w();
		T hh = h();
		r = l+hh;
		b = t+ww;
	}
	
	T w() const { return r-l; }
	T h() const { return b-t; }
};

typedef Rect__<int> Recti;
typedef Rect__<float> Rectf;