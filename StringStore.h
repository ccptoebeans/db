/* 
	*************************************************************************

	StringStore.h

	Author:    Kristjan Valur Jonsson
	Created:   Dec. 2006
	OS:        Win32
	Project:   EVE Server Database Access

	Description:   

		This guy provides functionality to reuse string and data objects from
		the database.  It is a singleton storage for byte data.
		When getting the rowset, all strings are entered into this, and it
		returns a pointer to a StringStore element, possibly shared.
		
	Dependencies:

		Python

	(c) CCP 2006

	*************************************************************************
*/

#ifndef STRINGSTORE_H
#define STRINGSTORE_H

#include <hash_set>
#include "DelayedException.h"
#include "BlockAllocator.h"


class StoreElementBase
{
public:
	virtual PyObject* GetPython() = 0;
};


template <typename DataType, bool IsText>
class InternalStoreElement : public StoreElementBase
{
public:
    explicit InternalStoreElement(DataType* data, const size_t elems)
    {
    	mPython = false;
    	mData = data;
    	mElements = elems;
    	mHash = Hash();
    }

    InternalStoreElement(const InternalStoreElement& o)
    {
    	mPython = o.mPython;
    	if(mPython)
    	{
    		mObject = o.mObject;
    		Py_XINCREF( mObject );
    	}
    	else
    	{
    		mData = o.mData;
    		mElements = o.mElements;
    		mHash = o.mHash;
    	}
    }

	~InternalStoreElement()
    {
	    if (mPython)
	    {
		    Py_XDECREF( mObject );
	    }
    }

    InternalStoreElement& operator =(const InternalStoreElement& o)
    {
    	if(o.mPython)
    		Py_XINCREF( o.mObject );
    	if(mPython)
    		Py_XDECREF( mObject );
    	mPython = o.mPython;
    	if(mPython)
    	{
    		mObject = o.mObject;
    	}
    	else
    	{
    		mData = o.mData;
    		mElements = o.mElements;
    		mHash = o.mHash;
    	}
    	return *this;
    }

    bool operator <(const InternalStoreElement& rhs) const
    {
	    if(mPython)
	    {
	    	return false; // accessing mData is undefined behavior
	    }

	    int cmp = memcmp( mData, rhs.mData, min( mElements, rhs.mElements ) );
	    if(cmp != 0)
	    {
	    	return cmp < 0;
	    }

	    return mElements < rhs.mElements;
    }
    operator size_t() const { return mHash; }

    DelayedException* Claim(SimplePoolAllocator& allocator)
    {
	    _ASSERT( !mPython );
	    const size_t elen = sizeof( DataType );
	    auto tmp = static_cast<DataType*>(allocator.align( mElements * elen, static_cast<int>(elen) ));
	    if(!tmp)
	    {
	    	return DelayedException::NoMem();
	    }
	    memcpy( tmp, mData, mElements * elen );
	    mData = tmp;
	    return 0;
    }

    PyObject* GetPython() override
    {
    	if(!mPython && !ToPython())
    	{
    		return nullptr;
    	}
    	_ASSERT( mPython );
    	return mObject;
    }

private:
	bool ToPython();

    size_t Hash() const
    {
	    // hash range of elements
	    size_t hash = 2166136261U;
	    const size_t maxHash = 64;
	    DataType* begin = mData;
	    DataType* end = mData + min(mElements, maxHash);
	    while (begin != end)
		    hash = 16777619U * hash ^ (size_t)*begin++;

	    return hash;
    }

	// raw data and python object are stored in a mutually exclusive manner for memory efficiency
	union
    {
		struct
		{
		    DataType* mData;
		    size_t mElements;
		    size_t mHash;
		};

		PyObject* mObject;
    };

	// true if mObject is active part of storage union
    bool mPython;
};



typedef InternalStoreElement<char, true> StringStoreElement;		// utf-8 storage element
typedef InternalStoreElement<char, false> ByteStoreElement;			// byte storage element
typedef InternalStoreElement<wchar_t, true> WStringStoreElement;	// unicode storage element

template <typename ElementType, typename DataType>
class InternalStore
{
public:
	explicit InternalStore(SimplePoolAllocator& allocator) :
	mAllocator( allocator ),
	mSet( traits_t(), allocator_t( allocator ) )
	{
		mMemSaved = 0;
	}

    DelayedException* Insert(ElementType*& res, DataType* data, size_t elems)
    {
    	ElementType tmp( data, elems );
    	std::pair<InternalStoreSet_iterator, bool> r = mSet.insert( tmp );

    	// This is slightly evil. The insert gives us back a const iterator
    	// to the StringStoreElem either found or just added. This used to be a
    	// regular iterator, but with VS2010 it is a const iterator.
    	// It would be cleaner to change this to a map - separate the
    	// key out from the value. We know that the ordering operator
    	// used on the StringStoreElem will not change so this does work - no
    	// operations we can do on the element will invalidate the set.
    	// Note that the std::set documentation clearly states that
    	// values should not change after they are added to the set.
    	InternalStoreSet_iterator& elIt = r.first;
    	auto& el = const_cast<ElementType&>(*elIt);

    	res = 0;
    	if(r.second)
    	{
    		DelayedException* e = el.Claim( mAllocator );
    		if(e)
    		{
    			return e;
    		}
    	}
    	else
    	{
    		mMemSaved += elems * sizeof( DataType );
    	}
    	res = &el;
    	return 0;
    }

    size_t GetMemSaved() const { return mMemSaved; }

private:
    typedef stdext::hash_compare<ElementType> traits_t;
    typedef StlPoolAllocator<ElementType> allocator_t;

    SimplePoolAllocator& mAllocator;
	typedef stdext::hash_set<ElementType, traits_t, allocator_t> InternalStoreSet_t;
    typedef typename InternalStoreSet_t::iterator InternalStoreSet_iterator;

    InternalStoreSet_t mSet;
    size_t mMemSaved; // how much memory (without overhead) saved by the reuse
};

 struct Store
{
	explicit Store(SimplePoolAllocator& allocator) : stringStore(allocator), byteStore(allocator), wStringStore(allocator){}

	size_t GetMemSaved() const { return stringStore.GetMemSaved() + byteStore.GetMemSaved() + wStringStore.GetMemSaved(); }

	InternalStore<StringStoreElement, char> stringStore;
	InternalStore<ByteStoreElement, char> byteStore;
	InternalStore<WStringStoreElement, wchar_t> wStringStore;
};


template <>
inline bool InternalStoreElement<wchar_t, true>::operator <( const InternalStoreElement& rhs ) const
{
	if(mPython)
	{
		return false; // accessing mData is undefined behavior
	}

	int cmp = wmemcmp( mData, rhs.mData, min( mElements, rhs.mElements ) );
	if(cmp != 0)
	{
		return cmp < 0;
	}

	return mElements < rhs.mElements;
}


#endif //STRINGSTORE_H
