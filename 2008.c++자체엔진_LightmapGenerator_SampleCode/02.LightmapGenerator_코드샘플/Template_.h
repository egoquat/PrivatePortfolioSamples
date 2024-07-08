
#pragma once 

#include "Common_.h"

#include "CommonEx_.h"

#define ARR_MIN_CAPACITY				(1)

#define DEFAULT_ARR_CAPACITY			(16)

#define ARR_MAX_CAPACITY				(2<<18) // 2^19

#define ARR_FALSE						(-1)

#define ARR_TRUE						!(-1)

template <class T>
class Array_
{
	//protected:
	public:
		UINT m_uiCount;
		UINT m_uiSize;
		T *m_pData;

		T *m_pNullData;

	protected:
		UINT IncreaseValidateCapacity( const UINT iRequestIndex )
		{
			UINT iNewSize = 1, iRequestIndex_ = iRequestIndex;

			if( ARR_MIN_CAPACITY - 1 > iRequestIndex_ )
			{
#if 1 == SHOW_DEBUG_MODE
				char		szMsg[512];

				sprintf( szMsg, 
					"요구하는 Array_의 크기(%d)는 Array_의 최소 Capacity(%d) 미만입니다. Capacity(%d)로 초기화 합니다.",
					iRequestIndex_, ARR_MIN_CAPACITY, ARR_MIN_CAPACITY );
				DEBUGMSG( szMsg );
#endif
				iRequestIndex_ = ARR_MIN_CAPACITY;
			}
			else if( ARR_MAX_CAPACITY < iRequestIndex_  )
			{
#if 1 == SHOW_DEBUG_MODE
				char		szMsg[512];

				sprintf( szMsg, 
					"요구하는 Array_의 크기(%d)는 Array_의 최소 Capacity(%d) 미만입니다. Capacity(%d)로 초기화 합니다.",
					iRequestIndex_,	ARR_MAX_CAPACITY, ARR_MAX_CAPACITY );
				DEBUGMSG( szMsg );
#endif
				iNewSize = ARR_MAX_CAPACITY;
			}

			iNewSize = ( 1 > m_uiSize )? 1 : m_uiSize;
			for ( ; iNewSize <= iRequestIndex_; iNewSize *= 2)
			{
			}

			return iNewSize;
		}

		UINT GetRealSize() const { return (m_uiSize); }

	public:
		Array_();	///< Constructor
		~Array_();	///< Destructor

		void Clear_( void);

		void NewAllocateBuffer( UINT nIndex = DEFAULT_ARR_CAPACITY );

		void InvalidateAll( void);	///< 버퍼의 내용만 초기화 / 버퍼 크기는 유지
		void InvalidateAndShrink( void);	///< 버퍼 크기 축소
		void IncreaseBufferSize_( UINT iSize);	///< 버퍼 크기를 늘린다.

		void BufferZero();

		T& Get_( UINT iIndex);

		T& GetLast_( UINT& uiCurSeq );

		const T& Get_( UINT iIndex) const;

		const T& GetLast_( UINT& uiCurSeq ) const;

		T* GetDataPtr() const { return ( m_pData); }

		UINT GetCount() const { return ( m_uiCount); }
		UINT GetSize() const { return ( m_uiCount); }

		UINT Find( const T& Data) const;

		UINT Copy( T *pSrc, UINT count);

		T Set__( UINT iIndex, T& Data);

		UINT Set_( T& Data);
		void Remove( UINT iIndex);
		
		void InvalidateFrom( UINT iIndex);


	protected:
		void IncreaseAllocateBuffer( UINT nIndex);

		/// @name 연산자 오버로딩
public:
		UINT operator = ( const T& Data);
		//T& operator[]( UINT iIndex );
		BOOL operator==( Array_<T>& OtherArray ) const;
		BOOL operator!=( Array_<T>& OtherArray ) const;
};


template <class T>
Array_<T>::Array_()
{
	m_pNullData = 0;
	m_uiCount	= 0;
	m_uiSize	= DEFAULT_ARR_CAPACITY * 2;

	m_pData		= 0;

	//NewAllocateBuffer();
	m_pData = new T [DEFAULT_ARR_CAPACITY * 2];
}

template <class T>
Array_<T>::~Array_()
{
	Clear_();
}

template <class T>
void Array_<T>::Clear_( void)
{
	m_uiCount = 0;
	m_uiSize = 0;

	SAFE_DEL_ARR( m_pData );
	m_pData = 0;
	SAFE_DEL( m_pNullData );
	m_pNullData = 0;
}

template <class T>
void Array_<T>::InvalidateAll( void)
{
	m_uiCount = 0;
}

template <class T>
void Array_<T>::InvalidateAndShrink( void)
{
	SAFE_DEL_ARR(m_pData);
	m_uiCount = 0;
	m_uiSize = 16;
	m_pData = new T [m_uiSize];
}

#include <assert.h>
template <class T>
void Array_<T>::NewAllocateBuffer( UINT iIndex = DEFAULT_ARR_CAPACITY)
{
	UINT iNewSize = 0;

	iNewSize = IncreaseValidateCapacity( iIndex );

	SAFE_DEL_ARR( m_pData );

	m_pData = new T [iNewSize];
	m_uiSize = iNewSize;
}


template<class T>
void Array_<T>::BufferZero()
{
	if( 1 > m_uiSize ) return;

	memset( m_pData, 0, sizeof(T) * m_uiSize );
}

// protected		// 에러 유발 가능 : 내부에 동적 메모리 할당 된 사항은 메모리 Copy시에 무시가 되버린다.
template <class T>
void Array_<T>::IncreaseAllocateBuffer( UINT iIndex)
{
	if ( iIndex >= m_uiSize)
	{	// 넘쳤으면 크기를 늘려준다.
		UINT iNewSize = IncreaseValidateCapacity(iIndex);

		// 크기를 늘린 다음 복사
		T *pTempBuffer = new T [iNewSize];
		memcpy( pTempBuffer, m_pData, m_uiSize * sizeof ( T));
		SAFE_DEL_ARR( m_pData );
		m_pData = pTempBuffer;
		m_uiSize = iNewSize;
	}
}

template <class T>
void Array_<T>::IncreaseBufferSize_( UINT iSize)
{
	IncreaseAllocateBuffer( iSize);
}

// protected
template <class T>
T Array_<T>::Set__( UINT iIndex, T& Data )
{
	IncreaseAllocateBuffer( iIndex);

	int iSize = sizeof(T);

	//memcpy( &m_pData[iIndex], &Data, iSize );
	m_pData[iIndex] = Data;

	m_uiCount = max( iIndex + 1, m_uiCount);

	return ( Data);
}

template <class T>
UINT Array_<T>::Set_( T& Data)
{
	Set__( m_uiCount, Data);

	return ( m_uiCount - 1);
}

template <class T>
T& Array_<T>::Get_( UINT iIndex)
{
	if( iIndex >= m_uiCount )
	{
		if( iIndex >= m_uiSize)
		{
			//// 범위 밖인 경우 0 으로 채워서 리턴
			/*if( 0 == m_pNullData )
			{
				m_pNullData = new T;
				memset( &m_pNullData, 0, sizeof( T));
			}
			return (*m_pNullData);*/

			//// 범위 밖인 경우 새로운 범위에 맞는 테이블로 재 구성 후 해당 Index 데이터 반환
			IncreaseAllocateBuffer( iIndex );
		}

		m_uiCount = iIndex + 1;
	}

	return ((T*)m_pData)[iIndex];
	//return ( m_pData[iIndex]);
}

template <class T>
const T& Array_<T>::Get_( UINT iIndex) const
{
	if( iIndex >= m_uiCount )
	{
		if( iIndex >= m_uiSize)
		{
			//// 범위 밖인 경우 0 으로 채워서 리턴
			/*if( 0 == m_pNullData )
			{
			m_pNullData = new T;
			memset( &m_pNullData, 0, sizeof( T));
			}
			return (*m_pNullData);*/

			//// 범위 밖인 경우 새로운 범위에 맞는 테이블로 재 구성 후 해당 Index 데이터 반환
			IncreaseAllocateBuffer( iIndex );
		}

		m_uiCount = iIndex + 1;
	}

	return ((T*)m_pData)[iIndex];
	//return ( m_pData[iIndex]);
}


template <class T>
T& Array_<T>::GetLast_( UINT& uiCurSeq )
{
	uiCurSeq = m_uiCount;

	return Get_( uiCurSeq );
}


template <class T>
const T& Array_<T>::GetLast_( UINT& uiCurSeq ) const
{
	uiCurSeq = m_uiCount;

	return Get_( uiCurSeq );
}

template <class T>
UINT Array_<T>::Copy( T *pSrc, UINT count)
{
	UINT index = m_uiCount;
	IncreaseAllocateBuffer( index + count);
	memcpy( m_pData + index, pSrc, sizeof( T) * count); // 메모리 배열로 값을 복사한다
	m_uiCount = max( index + count, m_uiCount);
	return ( m_uiCount - 1);
}

template <class T>
void Array_<T>::Remove( UINT iIndex)
{
	if ( --m_uiCount > iIndex)
	{
		T *pTempBuffer = new T [m_uiCount - iIndex];
		memcpy( pTempBuffer, &m_pData[iIndex + 1], ( m_uiCount - iIndex) * sizeof ( T));	// 한칸씩 밀린 데이터 복사
		memcpy( &m_pData[iIndex], pTempBuffer, ( m_uiCount - iIndex) * sizeof ( T));
		SAFE_DEL_ARR(pTempBuffer);
	}
}

template <class T>
void Array_<T>::InvalidateFrom( UINT iIndex)
{
	m_uiCount = min( iIndex, m_uiCount);
}

template <class T>
UINT Array_<T>::Find( const T& Data) const
{
	for ( UINT i = 0; i < m_uiCount; ++i)
	{
		if ( m_pData[i] == Data)
		{
			return ( i);
		}
	}

	return ( ARR_FALSE );
}

//template <class T>
//T& Array_<T>::operator[]( UINT iIndex)
//{
//	return Get_(iIndex);
//}

template <class T>
UINT Array_<T>::operator = ( const T& Data)
{
	return Set_(Data);
}

template <class T>
BOOL Array_<T>::operator==( Array_<T>& OtherArray ) const 
{
	if(m_uiCount != OtherArray.GetCount() || m_uiSize != OtherArray.GetRealSize())
	{
		return FALSE;
	}

	for( UINT i = 0; i < m_uiCount; ++i )
	{
		if(!((*this)[i] == OtherArray[i]))
			return FALSE;
	}

	return TRUE;
}

template <class T>
BOOL Array_<T>::operator!=( Array_<T>& OtherArray ) const 
{
	if(m_uiCount != OtherArray.GetCount() || m_uiSize != OtherArray.GetRealSize())
	{
		return TRUE;
	}

	for( UINT i = 0; i < m_uiCount; ++i )
	{
		if(!((*this)[i] == OtherArray[i]))
			return TRUE;
	}

	return FALSE;
}
