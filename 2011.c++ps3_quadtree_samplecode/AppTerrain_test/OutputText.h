#pragma once

#include "CommonApp.h"

#include <PSSG.h>
#include <Framework/PSSGFramework.h>
#include <Extra/PSSGExtra.h>
#include <PSSGText/PSSGText.h>

using namespace PSSG;

#define DEFAULT_FONTFACE			(PChar)"Tuffy.ttf"
#define DEFAULT_FONTINSTANCENAME	(PChar)"DEFAULTFONT"
#define DEFAULT_FONTSIZE			30

struct S_PTextFormat
{
	PChar	*szFontInstanceName;
	PChar	*szFontFace;

	PFont	*pPFont;

	int	iFontSize;

	int	byTopColorR;
	int	byTopColorG;
	int	byTopColorB;
	int	byBottomColorR;
	int	byBottomColorG;
	int	byBottomColorB;

	float	fOffsetX;
	float	fOffsetY;
	float	fOffsetZ;

	float	fHeight;

	float	fCenterX;
	float	fCenterY;

	void Init(PChar *szFontInstanceName_, 
		PChar *szFontFace_, 
		int iFontSize_,
		int byTopColorR_, 
		int byTopColorG_, 
		int byTopColorB_, 
		int byBottomColorR_,
		int byBottomColorG_,
		int byBottomColorB_,
		float fOffsetX_,
		float fOffsetY_,
		float fOffsetZ_,
		float fHeight_,
		float fCenterX_,
		float fCenterY_);


	S_PTextFormat();
	~S_PTextFormat();
};

class COutputText
{
private:
	PText			*m_pTextInstance;
	PDatabase		*m_pDatabase;

	S_PTextFormat	m_FormatText;

public:
	void Init( PDatabase *pDatabase, 
		PNode *pNode,
		S_PTextFormat &TextFormat );

	void RenderText( PChar *szText );

public:
	COutputText(void);
	~COutputText(void);
};
