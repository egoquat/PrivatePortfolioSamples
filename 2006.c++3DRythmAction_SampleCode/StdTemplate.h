//=============================================================================================================
//	작성일		:	06/06/19
//	클래스명	:	Stack, Queue
//	작성내용	:	Stack, Queue Templat
//=============================================================================================================

#pragma once

template <typename insideItem> class Stack
{
private:
	typedef struct node
	{
		insideItem				m_iItem;
		node*					m_pNext;
		node*					m_pPrev;
		node()
		{
			m_pNext = 0;
			m_pPrev = 0;
		}

		~node() 
		{
		}
	}Nptr;

public:

	Nptr*		nTop;
	int			iCnt;

public:
	Stack()
	{
		nTop = NULL;
		iCnt = 0;
	};

	Stack(const Stack& S)
	{};

	~Stack()
	{
		int	iTemp;
		while (!IsEmpty())
		{
			iTemp = Pop();
		}
	};

	bool	IsEmpty()
	{
		return (nTop == NULL);
	}

	void	Push(insideItem iItem)
	{
		Nptr*	nTemp		= new Nptr;

		nTemp->m_iItem		= iItem;
		nTemp->m_pNext		= nTop;
		nTop				= nTemp;
		iCnt++;
	}

	insideItem	Pop()
	{
		insideItem	iItem;

		if (!IsEmpty())
		{
			Nptr*			nTemp	= nTop;
			iItem					= nTemp->m_iItem;
			nTop					= nTop->m_pNext;
			iCnt--;
			delete nTemp;
			return iItem;
		}

		return iItem;
	}

	void	FreeList()
	{
		Nptr*	nTemp = nTop;

		while (nTemp != NULL)
		{
			nTop		= nTop->m_pNext;
			delete		nTemp;
			nTemp		= nTop;
		}

		iCnt = 0;
	}
};

template <typename insideItem> class Queue
{
private:
	typedef struct node
	{
		insideItem				m_iItem;
		node*					m_pNext;
		node*					m_pPrev;
	}Nptr;

public:
	Nptr*		rear;
	Nptr*		front;
	int			iCnt;

public:
	Queue()
	{
		rear	= new Nptr();
		front	= new Nptr();

		rear->m_pNext	= front;
		rear->m_pPrev	= rear;
		front->m_pPrev	= rear;
		front->m_pNext	= front;

		iCnt = 0;
	};

	~Queue()
	{
		int			iTemp;
		insideItem	tmpItem;
		while (!IsEmpty())
		{
			iTemp = (int)get(tmpItem);
		}
		// -_-
		delete rear;
		delete front;
	};

	bool	IsEmpty()
	{
		return (iCnt < 1);
	}

	void	put(insideItem iItem)
	{
		Nptr*		pInsert = new Nptr();

		pInsert->m_iItem		= iItem;

		pInsert->m_pNext		= rear->m_pNext;
		rear->m_pNext->m_pPrev	= pInsert;
		pInsert->m_pPrev		= rear;
		rear->m_pNext			= pInsert;

		iCnt++;
	}

	bool	get(insideItem& iItem)
	{
		if (!IsEmpty())
		{
			Nptr*	pGetItem			= front->m_pPrev;
			iItem						= pGetItem->m_iItem;

			front->m_pPrev				= pGetItem->m_pPrev;
			pGetItem->m_pPrev->m_pNext	= front;

			iCnt--;

			delete pGetItem;

			return true;
		}

		return false;
	}

	void	FreeList()
	{
		Nptr*	pTemp;

		pTemp		= rear;

		while (pTemp != front)
		{
			pTemp = rear->m_pNext;
			rear->m_pNext = pTemp->m_pNext;
			delete		pTemp;
		}

		iCnt = 0;
	}
};

