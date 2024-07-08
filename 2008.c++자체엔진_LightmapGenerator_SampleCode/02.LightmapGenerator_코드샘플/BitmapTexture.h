#pragma once

#include <stdio.h>

#include <string.h>

#include "DefLightMap.h"

#include "FilePathUtil.h"

// 한장 당 한개 씩
class CBitmapTexture
{
private:
	CFilePathUtil		*m_pFilePathUtil;

private:
	BOOL SaveBitmap(char* lpszFileName, BYTE *arby1DColors, UINT uiWidth, int uiHeight);

	void ConvertToByte( const UINT *aruiSrcColor, UINT uiWidth, UINT uiHeight,BYTE *arbyDstColor );

public:
	char		_szBmpFileName[512];
	UINT		_uiWidth, _uiHeight, _uiCnt;

	BYTE*		_arbyColor;
	UINT*		_aruiColor;

	BOOL Init( char *szFileName, UINT *ardwColor,UINT uiWidth, UINT uiHeight );
	BOOL Init( char *szFileName, UINT uiWidth, UINT uiHeight );

	void Flip();

	BOOL Flip_Save();

	BOOL Save();

	BOOL Release();
public:
	CBitmapTexture(void);
	~CBitmapTexture(void);
};


