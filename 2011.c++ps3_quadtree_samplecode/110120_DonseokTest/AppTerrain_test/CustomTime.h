#pragma once

#include "CommonApp.h"


#include <fw/include/FWTime.h>

#define DEFAULT_TIMEINTERVAL 1.0f

class CCustomTime
{
private:
	double			m_dblInitTime;
	double			m_dblCurrentTime;	
	double			m_dblLastTime;

public:
	void		InitTime();
	void		MarkThisTime( void );
	double		GetElapsedTime( void );

public:
	CCustomTime(void);
	~CCustomTime(void);
};


class CCustomTimer
{
private:
	float m_fSecInterval;

public:
	void setInterval(float fSec) { m_fSecInterval=fSec;	};
	bool getIntervalTick(double dblElapsedTime);

public:
	CCustomTimer()
	{
		m_fSecInterval = DEFAULT_TIMEINTERVAL;
	};

	~CCustomTimer()
	{

	};
};

