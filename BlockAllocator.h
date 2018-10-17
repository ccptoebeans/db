/* 
	*************************************************************************

	BlockAllocator.h

	Author:    Kristjan Valur Jonsson
	Created:   Oct. 2009
	OS:        Win32
	Project:   EVE Server Database Access

	Description:   

		A simple class to dole out stuff, with a single cleanup at the end.
		
	Dependencies:

		Python

	(c) CCP 2009

	*************************************************************************
*/
#ifndef _BLOCKALLOCATOR_H_
#define _BLOCKALLOCATOR_H_

//This is your typical pool allocator.  Just gobbles up memory and then has a
//final free() function.
class SimplePoolAllocator
{
public:
	SimplePoolAllocator(size_t blockSize = 8*1024);
	~SimplePoolAllocator();
	void *malloc(size_t s); //returns s bytes or 0, default alignment
	void *align(size_t s, int alignment);
	void free(void *p); //does nothing.
	void free(); //returns all the memory

private:
	bool AddBlock(size_t s, int a);
	void *AddBigBlock(size_t s, int a);

private:
	const size_t mBlockSize;  // hos much to allocate each time
	//internal rounding functions
	char *RoundUp(char *p, int alignment);
	size_t RoundUp(size_t s, int alignment);

	//each block has a link to the next block at the head. mLink points to this link of the
	//current block, so that the next one can be linked in.
	void *mHead;
	void *mBigHead;
	char *mCurrent; //current pointer in the current block, aligned
	size_t mCurrentLeft; //bytes left in the current page
};


//A STL allocator that uses a SimplePoolAllocator instance
//Useful when one constructs stuff and then tears it all down
//at once.
//Note that it is technically non-standard because it has a state, and
//therefore not all instances of it are equal.  See:
//See Effective STL (Item 10).  Avoid list::splice() and std::swap()
//because of this..
template<class T>
class StlPoolAllocator : 
	public std::allocator<T>
{
public:
	using pointer = T * ;
	using size_type = size_t;
	StlPoolAllocator(SimplePoolAllocator &a) throw() : mA(a) {}
	StlPoolAllocator(const StlPoolAllocator<T> &o) throw() : mA(o.mA) {}
	template <class T2>
	StlPoolAllocator(const StlPoolAllocator<T2> &o) throw() : mA(o.mA) {}

	template <class T2>
	struct rebind {
		typedef StlPoolAllocator<T2> other;
	};

	pointer allocate(size_type _Count, const void *hint) {
		return reinterpret_cast<pointer>(mA.malloc(_Count * sizeof(T)));
	}
	pointer allocate(size_type _Count) {
		return allocate(_Count, 0);
	}
	void deallocate(pointer _ptr, size_type _Count) {} //do nothing

public:
	SimplePoolAllocator &mA;
};

template <class T>
bool operator == (StlPoolAllocator<T> &left, StlPoolAllocator<T> &right)
{
	return &left.mA == &right.mA;
}

template <class T>
bool operator != (StlPoolAllocator<T> &left, StlPoolAllocator<T> &right)
{
	return &left.mA != &right.mA;
}



#endif //_BLOCKALLOCATOR_H_
