#pragma once


// @ ALLOCATION, DEALLOCATION
#define		SAFEDELETE(p)		{if(p!=NULL){delete p;} p = NULL;}
#define		SAFEDELETEARRAY(p)	{if(p!=NULL){delete [] p;} p = NULL;}


// @ TYPE VALUE
#ifdef SN_TARGET_PS3 // Playstation3 Build 인 경우
typedef unsigned char	byte;
typedef byte			BYTE;
#endif // SN_TARGET_PS3

// @ STL
#include	<vector>
#include	<algorithm>

// @ APPLICATION
#define DEFAULT_FILEPATH	256


// @ DEBUG
#ifdef _DEBUG
#include <assert.h>
#endif // _DEBUG


// @ DEFINITION TERRAIN
#define _DEBUG_TERRAINTILE
//#define _DEBUG_TERRAINSHAPE_TEST
//#define _DEBUG_TERRAINTEST // Error : cant access IndexBuffer data on Terrain Data in PSSG.

// @ DEFINITION NAVIGATION
#define _DEBUG_NAVIGATION

#ifdef _DEBUG_NAVIGATION
	#define _DEBUG_COLLECTPICKING_FORNAVI
#endif // _DEBUG_NAVIGATION


// @ DEFINITION OBJECTS
#define _DEBUG_OBJECTS

#define _DEBUG_SPACETREE

// @ DEFINITION PHYRE INTERSECT LINE TRAVERSAL
#define _DEBUG_TRAVERSALINTERSECT_PH

// @ DEFINITION PLATFORM
#define		DEFAULT_NUMNSPURSSPUS		4
#define		DEFAULT_NUMPSSGSPUS			4

// @ DEFINITION FOR DEBUG 
#define		_DEBUG_CUSTOMTIME
//#define		_DEBUG_CUSTOMTIME_UPDATEOBJECTS

#define		_DEBUGFOR_RENDER


#ifdef _WIN32
	#define APPTARGET_IS_WIN
#else // _WIN32
	#define APPTARGET_IS_PS3
#endif // _WIN32

#ifdef APPTARGET_IS_WIN
	#ifdef _DEBUG
		#define MEMORYLEAK_FIND_CRTDBG_WIN32
	#endif // _DEBUG

	#ifdef MEMORYLEAK_FIND_CRTDBG_WIN32
		#define CRTDBG_MAP_ALLOC 
		#include <stdlib.h> 
		#include <crtdbg.h>
	#endif // MEMORYLEAK_FIND_CRTDBG_WIN32

	#define KEY_DOWN__(VK_CODE)	((GetAsyncKeyState(VK_CODE) & 0x8000)?1:0)

	//	#define		_DEBUG_OUTPUTDEBUGSTRING
#endif // APPTARGET_IS_WIN


// @ FOR DEBUG
#ifdef _DEBUG

#ifdef APPTARGET_IS_WIN
#define assert__(CONDITION, FILE__, LINE__, FUNCTION__, TEXTASSERT) {\
	if(!CONDITION){\
	char	szAssertText[256];\
	sprintf_s( szAssertText, "ERROR!!\n FILE::%s / LINE::d\n FUNCTION::%s \nEXPRESS:: %s)",\
	FILE__,LINE__,FUNCTION__, TEXTASSERT );\
	assert( 0 && szAssertText );}}
#else // APPTARGET_IS_WIN
#define assert__(CONDITION, FILE__, LINE__, FUNCTION__, TEXTASSERT) {\
	if(!CONDITION){\
	char	szAssertText[256];\
	sprintf( szAssertText, "ERROR!!\n FILE::%s / LINE::d\n FUNCTION::%s \nEXPRESS:: %s)",\
	FILE__,LINE__,FUNCTION__, TEXTASSERT );\
	assert( 0 && szAssertText );}}
#endif // APPTARGET_IS_WIN

#endif // _DEBUG


#define ENUMTOSTR(a) (#a) 

enum E_CAMERALIST { CAMERALIST_GAME = 0, 
					CAMERALIST_DEBUG,
					CAMERALIST_CNT, };

using namespace std;
