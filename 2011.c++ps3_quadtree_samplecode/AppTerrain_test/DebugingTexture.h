#pragma once

#include <PSSG.h>
#include <Extra/PSSGExtra.h>

#include <PSSGDebug/PSSGDebug.h>

using namespace PSSG;

class PDebugPrintfCallback : public PPrintfCallback
{
public:
	// 解説:
	// <c>PDebugPrintfCallback</c>クラスのコンストラクタ。
	PDebugPrintfCallback() : m_texture(NULL) {}

	// 解説:
	// <c>PDebugPrintfCallback</c>クラスのデストラクタ。
	virtual ~PDebugPrintfCallback() {}

	PTexture *m_texture;	// デバッグテクスチャ。

	// 解説:
	// <c>PSSG_PRINTF()</c>からの処理済み文字列を出力する;
	// 引数:
	// str : 出力する文字列
	virtual void printPrintf(const char *str)
	{
		if(m_texture)
		{
			PSSG::Extra::debugTexturePrintString(str, *m_texture);
		}
	}
} static g_debugPrintfCallback;

class CDebugingTexture
{
public:
		PTexture *m_debugTexture;

public:
	CDebugingTexture(void);
	~CDebugingTexture(void);

public:
	void Init();

	void printText(PChar *szOutput);
};
