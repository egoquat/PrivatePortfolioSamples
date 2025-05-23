#include "OutputText.h"



void S_PTextFormat::Init(PChar *szFontInstanceName_, 
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
						  float fCenterY_)
{
	szFontInstanceName = szFontInstanceName_;
	szFontFace = szFontFace_;
	iFontSize = iFontSize_;

	byTopColorR = byTopColorR_;
	byTopColorG = byTopColorG_;
	byTopColorB = byTopColorB_;
	byBottomColorR = byBottomColorR_;
	byBottomColorG = byBottomColorG_;
	byBottomColorB = byBottomColorB_;
	fOffsetX = fOffsetX_;
	fOffsetY = fOffsetY_;
	fOffsetZ = fOffsetZ_;
	fHeight = fHeight_;
	fCenterX = fCenterX_;
	fCenterY = fCenterY_;
}

S_PTextFormat::S_PTextFormat()					
{
	szFontInstanceName	= "DEFAULTFONT";
	szFontFace			= "Tuffy.ttf";
}

S_PTextFormat::~S_PTextFormat()
{
}




void COutputText::Init( PDatabase *pDatabase, 
					  PNode *pNode,
					  S_PTextFormat &TextFormat )
{
	PResult		result;
	memcpy( &m_FormatText, &TextFormat, sizeof(S_PTextFormat) );

	m_FormatText.pPFont = pDatabase->createObject<PFont>(
		PDatabaseUniqueNameHelper(*pDatabase, "font"));	//  データベースに新しいフォントオブジェクトを追加します
	m_FormatText.pPFont->setFace(m_FormatText.szFontFace, m_FormatText.iFontSize);

	PVisibleRenderNode * tr = pNode->createChildNode<PVisibleRenderNode>(
		PDatabaseUniqueNameHelper(*pDatabase, "/text_root"), &result);
	m_pTextInstance = tr->createRenderInstance<PText>
		(PDatabaseUniqueNameHelper(*pDatabase, m_FormatText.szFontInstanceName));	

	m_pTextInstance->setFont(m_FormatText.pPFont);							//  そのフォントを、先ほど開いたフォントに設定します
	m_pTextInstance->setText(" ");							//  そしてテキストをレンダリングします

	m_pTextInstance->setColor(PText::PE_TOP,
		m_FormatText.byTopColorR,m_FormatText.byTopColorG,m_FormatText.byTopColorB);	

	m_pTextInstance->setColor(PText::PE_BOTTOM,
		m_FormatText.byBottomColorR,m_FormatText.byBottomColorG,m_FormatText.byBottomColorB);	

	m_pTextInstance->setXYOffset(false, m_FormatText.fOffsetX, m_FormatText.fOffsetY);
	m_pTextInstance->setZOffset(false,m_FormatText.fOffsetZ);			
	m_pTextInstance->setScale(false);						
	m_pTextInstance->setHeight(m_FormatText.fHeight);
	m_pTextInstance->setCenter(m_FormatText.fCenterX,m_FormatText.fCenterY);
}

void COutputText::RenderText( PChar *szText )
{
	if( m_pTextInstance )
	{
		m_pTextInstance->setText(szText);
	}
}



COutputText::COutputText(void)
{
	m_pTextInstance = NULL;
}

COutputText::~COutputText(void)
{
}
