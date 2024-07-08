#pragma once

#include <vector>

#include "Common_.h"

#include "reflib\il.h"
#include "reflib\il_wrap.h"

#include "FilePathUtil.h"

#include "CommonEx_.h"

using namespace std;

// Alpha Chennel �� ����
class CAlphaTexture
{
public:
	BOOL					m_AlphaWasUpdated;

	bool					m_alpha_flag;
	int						_width;
	int						_height;
	vector<unsigned char>	m_alpha_data;

public:

	// ���� ���� ä���� alpha data�� ���� : Image�� ���� Loading �Ͽ� Alpha Texture ���� �Ǵ�
	void RealizeTextureOnlyAlphaChannel(const char *szFileName, int iBasisWidth = -1, int iBasisHeight = -1);

public:
	CAlphaTexture();

	~CAlphaTexture();
};
