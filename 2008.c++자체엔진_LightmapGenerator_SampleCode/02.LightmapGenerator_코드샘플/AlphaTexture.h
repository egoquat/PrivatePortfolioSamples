#pragma once

#include <vector>

#include "Common_.h"

#include "reflib\il.h"
#include "reflib\il_wrap.h"

#include "FilePathUtil.h"

#include "CommonEx_.h"

using namespace std;

// Alpha Chennel 만 갱신
class CAlphaTexture
{
public:
	BOOL					m_AlphaWasUpdated;

	bool					m_alpha_flag;
	int						_width;
	int						_height;
	vector<unsigned char>	m_alpha_data;

public:

	// 실제 알파 채널을 alpha data에 갱신 : Image를 실제 Loading 하여 Alpha Texture 여부 판단
	void RealizeTextureOnlyAlphaChannel(const char *szFileName, int iBasisWidth = -1, int iBasisHeight = -1);

public:
	CAlphaTexture();

	~CAlphaTexture();
};
