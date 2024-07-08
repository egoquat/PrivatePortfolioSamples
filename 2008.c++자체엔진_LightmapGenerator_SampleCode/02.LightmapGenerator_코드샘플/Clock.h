#pragma once

#include <windows.h>
#include <time.h>

#include "Singleton.h"

class CClock_ : public CSingleton<CClock_>
{
protected:
	unsigned long		m_ulFixedTPS;
	unsigned long		m_ulBaseTick;

public:
	unsigned long		GetCurrTick();
	unsigned long		GetTPS();

	int		GetRandNum(int iMinNum, int iMaxNum);

public:
	CClock_(void);
	~CClock_(void);
};
