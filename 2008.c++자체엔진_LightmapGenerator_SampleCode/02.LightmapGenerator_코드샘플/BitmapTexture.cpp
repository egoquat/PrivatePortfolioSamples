#include ".\bitmaptexture.h"

BOOL CBitmapTexture::Init( char* szFileName, UINT* ardwColor, UINT uiWidth, UINT uiHeight )
{
	BYTE		a, r, g, b;		a=r=g=b=0;
	UINT		uiCur = 0, uiByteIdx = 0;
	UINT		SrcCo = 0;

	_uiCnt = uiWidth * uiHeight;
	if( 1 > _uiCnt ) return FALSE;

	_aruiColor = new UINT[_uiCnt];
	//_arbyColor = new BYTE[_uiCnt * 3];

	memset( _aruiColor, 0, sizeof(UINT) * _uiCnt );

	_uiWidth = uiWidth;		_uiHeight = uiHeight;

	for( UINT uiX = 0; uiX < _uiWidth; ++uiX )
	{
		for( UINT uiY = 0; uiY < _uiHeight; ++uiY )
		{
			uiCur = ( uiY * _uiWidth ) + uiX;

			SrcCo = ardwColor[uiCur];

			a = PickA(SrcCo);	r = PickR(SrcCo);	g = PickG(SrcCo);	b = PickB(SrcCo);

			_arbyColor[uiByteIdx++] = b;
			_arbyColor[uiByteIdx++] = g;
			_arbyColor[uiByteIdx++] = r;			
		}
	}

	char		szFileEx[8];
	
	m_pFilePathUtil = m_pFilePathUtil->GetThis();
	if( TRUE == m_pFilePathUtil->ExtractFileEx_( szFileName, szFileEx ) )
	{
		if( 0 != strcmp( strlwr(szFileEx), TEXT("bmp") ) )
		{
			sprintf( _szBmpFileName, "%s.bmp", szFileName );
		}
		else
		{
			strcpy( _szBmpFileName, szFileName );
		}
	}
	else
	{
		sprintf( _szBmpFileName, "%s.bmp", szFileName );
	}

	
	return TRUE;
}

BOOL CBitmapTexture::Init( char* szFileName, UINT uiWidth, UINT uiHeight )
{
	
	_uiCnt = uiWidth * uiHeight;
	if( 1 > _uiCnt ) return FALSE;

	_uiWidth = uiWidth;		
	_uiHeight = uiHeight;

	_aruiColor = new UINT[_uiCnt];
	//_arbyColor = new BYTE[_uiCnt * 3];

	memset( _aruiColor, 0, sizeof(UINT) * _uiCnt );

	sprintf( _szBmpFileName, "%s.bmp", szFileName );

	return TRUE;
}

void CBitmapTexture::Flip()
{
	_arbyColor = new BYTE[_uiCnt * 3];

	ConvertToByte( _aruiColor, _uiWidth, _uiHeight, _arbyColor );
}

BOOL CBitmapTexture::Flip_Save()
{
	_arbyColor = new BYTE[_uiCnt * 3];

	ConvertToByte( _aruiColor, _uiWidth, _uiHeight, _arbyColor );

	return Save();
}

void CBitmapTexture::ConvertToByte( const UINT *aruiSrcColor, UINT uiWidth, UINT uiHeight, BYTE *arbyDstColor )
{
	UINT uiCur = 0, uiByteIdx = 0;
	UINT SrcCo, a,r,g,b; SrcCo=a=r=g=b=0;

	for( UINT uiX = 0; uiX < _uiWidth; ++uiX )
	{
		for( UINT uiY = 0; uiY < _uiHeight; ++uiY )
		{
			uiCur		= ( uiY * _uiWidth ) + uiX;
			//uiByteIdx	= (( uiY * _uiWidth ) + uiX) * 3;
			uiByteIdx	= (( ( (_uiHeight-1) - uiY) * _uiWidth ) + uiX) * 3;
			
			if( _uiCnt * 3 < uiByteIdx )
			{
				char szDebug[512];

				sprintf( szDebug, "OverRange ! " );

				OutputDebugString( szDebug );
			}

			SrcCo = aruiSrcColor[uiCur];

			a = PickA(SrcCo);	r = PickR(SrcCo);	g = PickG(SrcCo);	b = PickB(SrcCo);

			arbyDstColor[uiByteIdx++] = b;
			arbyDstColor[uiByteIdx++] = g;
			arbyDstColor[uiByteIdx++] = r;	
		}
	}
}

BOOL CBitmapTexture::Release()
{
	SAFE_DEL_ARR(_arbyColor);
	SAFE_DEL_ARR(_aruiColor);

	_uiWidth = _uiHeight = _uiCnt = 0;

	return TRUE;
}

BOOL CBitmapTexture::Save()
{
	char		szFullFileName[512];

	if( FALSE == SaveBitmap( _szBmpFileName, _arbyColor, _uiWidth, _uiHeight ) )
	{
		SAFE_DEL( _arbyColor );
		return FALSE;
	}

	SAFE_DEL( _arbyColor );
	return TRUE;
}

BOOL CBitmapTexture::SaveBitmap(char* lpszFileName, BYTE *arby1DColors, UINT uiWidth, int uiHeight)
{
	DWORD dwSizeImage;

	//int nColorTableEntries = 256;
	int nColorTableEntries = 0;

	int i;
	// bmp Info Header

	LPBITMAPINFOHEADER lpBMIH; // buffer containing the BITMAPINFOHEADER



	//헤더를 저장하기 위한 공간확보

	lpBMIH = (LPBITMAPINFOHEADER) new char[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * nColorTableEntries];

	lpBMIH->biSize = sizeof(BITMAPINFOHEADER);

	lpBMIH->biWidth = uiWidth;    //이미지 가로크기

	lpBMIH->biHeight = uiHeight;   //이미지 세로크기

	//lpBMIH->biPlanes = 1;   //예약되어 있는 값
	lpBMIH->biPlanes = 1;   //예약되어 있는 값

	lpBMIH->biBitCount = 24; //8bit (GRAY LAVEL 사용)

	lpBMIH->biCompression = BI_RGB; //RGB모드

	lpBMIH->biSizeImage = uiWidth*uiHeight*3;        //이미지 전체크기

	lpBMIH->biXPelsPerMeter = 0;    //이미지의 가로 해상도

	lpBMIH->biYPelsPerMeter = 0;    //이미지의 세로 해상도

	lpBMIH->biClrUsed = 0;        //256 색상 사용

	lpBMIH->biClrImportant = 0;       //중요한 색상 개수

	dwSizeImage = lpBMIH->biSizeImage;  //이미지 사이즈 정보    

	//파일정보헤더

	BITMAPFILEHEADER bmfh;

	bmfh.bfType = 0x4d42; // 'BM' -> BMP 파일을 의미

	// size of DATA+BITMAPINFOHEADER 
	int nSize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * nColorTableEntries + dwSizeImage;

	bmfh.bfSize = nSize + sizeof(BITMAPFILEHEADER); // 전체 파일 크기

	bmfh.bfReserved1 = bmfh.bfReserved2 = 0;    //예약된 공간

	//실제 데이터가 저장되어 있는 공간을 가리기키 위한 오프셋 정보
	bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * nColorTableEntries; 

	//bfHeader.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER) +((color)*sizeof(RGBQUAD));//Location of bitmap data


	FILE* m_pStream = fopen(lpszFileName,"wb");
	if (m_pStream == NULL)
	{
		char buf[1024];
		sprintf(buf,"LIGHTMAP 결과를 저장할 수 없쌈.");
		MessageBox(NULL, buf,"...",MB_OK);
		return FALSE;
	}

	//m_File.Write((LPVOID) &bmfh, sizeof(BITMAPFILEHEADER));
	fwrite( &bmfh, sizeof(BITMAPFILEHEADER), 1, m_pStream );

	//m_File.Write((LPVOID) lpBMIH, sizeof(BITMAPINFOHEADER));
	fwrite( lpBMIH, sizeof(BITMAPINFOHEADER), 1, m_pStream );

	//m_File.Write((LPBYTE) arby1DColors, lpBMIH->biSizeImage);
	fwrite( (LPBYTE) arby1DColors, lpBMIH->biSizeImage, 1, m_pStream );

	fclose(m_pStream);

	m_pStream = 0;

	delete [] lpBMIH;   //헤더 정보 저장공간 삭제해줌.

	return TRUE;    //bmp 파일 생성에 성공.

}

CBitmapTexture::CBitmapTexture(void)
{
	_arbyColor = 0;
	_aruiColor = 0;
}

CBitmapTexture::~CBitmapTexture(void)
{
	Release();
}
