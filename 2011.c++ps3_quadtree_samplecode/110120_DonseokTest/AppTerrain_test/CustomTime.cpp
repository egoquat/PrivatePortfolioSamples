#include "CustomTime.h"

#include <PSSG.h>

void CCustomTime::InitTime()
{
	FWTime::update();
	m_dblInitTime = (double)FWTime::getCurrentTime();
}

void CCustomTime::MarkThisTime( void )
{
	FWTime::update();
	m_dblLastTime = (double)FWTime::getCurrentTime();
}


double CCustomTime::GetElapsedTime( void )
{
	FWTime::update();
	m_dblCurrentTime = (double)FWTime::getCurrentTime();

 	double dblElapsed = (m_dblCurrentTime - m_dblLastTime);
	return (dblElapsed);
}

CCustomTime::CCustomTime(void)
{	
	m_dblInitTime		= 0;

	m_dblLastTime		= 0;
	m_dblCurrentTime	= 0;
}

CCustomTime::~CCustomTime(void)
{
}



bool CCustomTimer::getIntervalTick(double dblElapsedTime)
{
	// 1. Frame Per Second
	static unsigned long ulTickCnt = 0;
	static double dElapsedTimeSummary = 0.0f;

	unsigned long ulTickCntCurrent = 0;

	double dElapsedTime = dblElapsedTime;

	ulTickCntCurrent = ulTickCnt++;
	dElapsedTimeSummary += dElapsedTime;

	if( dElapsedTimeSummary >= 1.0f || 0 == ulTickCnt )
	{
		ulTickCnt = 0;
		dElapsedTimeSummary = 0.0f;	

		return true;
	}
	else
	{
		return false;
	}
};


