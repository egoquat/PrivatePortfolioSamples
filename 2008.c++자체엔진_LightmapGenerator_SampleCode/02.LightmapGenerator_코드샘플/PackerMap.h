#pragma once

#include <utility>

#include "_parserlightmapmesh.h"

typedef pair<_CParserLightMapMesh *,int> 
		pairObjectTexBinder_Type;

class CObjectTexBinderClass
{
public:
	set<pairObjectTexBinder_Type > m_members;
	int width,height;
	int region_sum;

	CObjectTexBinderClass()
	{
		width = height = 0;
		region_sum = 0;
	}
};

class CPackerMap
{
public:
	CPackerMap(void);
	~CPackerMap(void);
};
