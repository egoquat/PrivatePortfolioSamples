#include "NaviManager.h"

#ifdef _DEBUG_NAVIGATION
using namespace NAVIPART;


void CNaviManager::BuildupNewMappingNavi( set<int> &slistCellsGoal, set<int> &slistCellsBlock )
{
	CNaviCells	&naviCells = m_NaviCells_basement;
	naviCells.constructMappingNavigation( slistCellsGoal, slistCellsBlock );
}

void CNaviManager::BuildNewNavi(	set<int> &slistCellsGoal, set<int> &slistCellsBlock )
{
	BuildupNewMappingNavi(slistCellsGoal, slistCellsBlock);
}

void CNaviManager::Initialize(	CTriContainer	*ptriContainer,
								CSpaceTree		*pSpaceTree )
{
	m_ptriContainer = ptriContainer;
	m_pSpaceTree = pSpaceTree;

	m_NaviCells_basement.Initialize(*m_ptriContainer);
}

void CNaviManager::Release()
{
	if(m_vecNaviCells.size())
	{
		for(int i=0; i<(int)m_vecNaviCells.size(); ++i)
		{
			m_vecNaviCells[i].Release();
		}

		m_vecNaviCells.clear();
	}
}

CNaviManager::CNaviManager()
{
	m_ptriContainer = NULL;
	m_pSpaceTree = NULL;

	m_uiCntGoal = 0;
}

CNaviManager::~CNaviManager()
{

}

#endif // _DEBUG_NAVIGATION
