
#pragma once

#include <assert.h>

template <class T>
class CSingleton  
{
public:
	static T* s_pSingleton;

	CSingleton()
	{
		assert( !s_pSingleton);
		int iOffset = ( int)( T*)1 - ( int)( CSingleton <T>*)( T*)1;
		s_pSingleton = ( T*)( ( int)this + iOffset);
	}
	virtual ~CSingleton()
	{
		assert( s_pSingleton);
		s_pSingleton = 0;
	}

	static T* GetThis( void)
	{
		if( s_pSingleton == 0 )
		{
			s_pSingleton = new T;
		}
		return ( s_pSingleton );
	}

	void DestroyThis()
	{
		if( s_pSingleton )
		{
			delete s_pSingleton;
		}

		s_pSingleton = 0;
	}
};


template <class T>
T* CSingleton<T>::s_pSingleton = 0;

#define	SAFE_SINGLETON_DESTROY(T) { if(T) { T->DestroyThis(); } T = 0; }

