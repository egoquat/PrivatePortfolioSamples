#include ".\alphatexture.h"

// 실제 알파 채널을 alpha data에 갱신 : Image를 실제 Loading 하여 Alpha Texture 여부 판단
void CAlphaTexture::RealizeTextureOnlyAlphaChannel(const char *szFileName, int iBasisWidth, int iBasisHeight)
{
	if( TRUE == m_AlphaWasUpdated ) return;

	bool alpha_flag = false;

	{ // Alpha가 지원 되는 형식인지, ex .TGA, .DDS(DXT5)
		CFilePathUtil* pFUtil = pFUtil->GetThis();
		pFUtil->SplitPath(szFileName);
		pFUtil->ext[0] = toupper(pFUtil->ext[0]);
		pFUtil->ext[1] = toupper(pFUtil->ext[1]);
		pFUtil->ext[2] = toupper(pFUtil->ext[2]);
		pFUtil->ext[3] = toupper(pFUtil->ext[3]);

		if (strcmp(pFUtil->ext,".TGA")==0)
		{
			alpha_flag = true;
		}
		else if (strcmp(pFUtil->ext,".DDS")==0)
		{
			char dxtsignal[5];
			dxtsignal[4] = 0;
			FILE *f = fopen(szFileName,"rb");
			fseek(f,0x54,SEEK_SET);
			fread(dxtsignal,4,1,f);
			fclose(f);

			if ( strcmp(dxtsignal,"DXT5")==0 || 
				strcmp(dxtsignal, "DXT4")==0 ||
				strcmp(dxtsignal, "DXT3")==0 ||
				strcmp(dxtsignal, "DXT2")==0 )
			{
				alpha_flag = true;	
			}
			else
			{
				alpha_flag = false;
			}
		}

	}

	if (alpha_flag)
	{

		this->m_alpha_flag = true;

		ilImage img;

		img.Load((char*)szFileName);

		ILubyte *buf = img.GetData();

		if( -1 == iBasisWidth ) { this->_width = img.Width(); } else { this->_width = iBasisWidth; }
		if( -1 == iBasisHeight ) { this->_height = img.Height(); } else { this->_height = iBasisHeight; }

		this->m_alpha_data.resize(this->_width * this->_height);

		int bpp = img.Bpp();
		UINT uiCur = 0;

		// image alpha channel만 this->m_alpha_data[idx]
		int i = 0,ni = this->_height;
		for( i = 0 ; i < ni ; ++i )
		{
			int j = 0,nj = this->_width;
			for( j = 0 ; j < nj ; ++j )
			{
				int idx = i * this->_width + j;
				uiCur = (idx * bpp) + 3;
				this->m_alpha_data[idx] = buf[uiCur];
			}
		}				
	}
	else
	{
		this->m_alpha_flag = false;
	}

	m_AlphaWasUpdated = TRUE;
}


CAlphaTexture::CAlphaTexture(void)
{
	m_AlphaWasUpdated = FALSE;
	m_alpha_data.resize( 0 );
}

CAlphaTexture::~CAlphaTexture(void)
{
	m_alpha_data.clear();
}