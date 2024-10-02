#include "stdafx.h"
#include "StringStore.h"



StringStoreElem::StringStoreElem(char *data, size_t elems, bool isBytes)
{
	mPython = false;
	mCData = data;
	mElements = elems;
	mElementType = isBytes ? BYTESTRING : STRING;
	Hash();
}


StringStoreElem::StringStoreElem(wchar_t *data, size_t elems)
{
	mPython = false;
	mWData = data;
	mElements = elems;
	mElementType = UNICODE;
	Hash();
}


StringStoreElem::StringStoreElem(const StringStoreElem &o)
{
	mPython = o.mPython;
	if (mPython) {
		mObject = o.mObject;
		Py_XINCREF(mObject);
	} else {
		mCData = o.mCData;
		mElements = o.mElements;
		mHash = o.mHash;
		mElementType = o.mElementType;
	}
}


StringStoreElem::~StringStoreElem()
{
	if (mPython)
		Py_DECREF(mObject);
}


StringStoreElem &StringStoreElem::operator =(const StringStoreElem &o)
{
	if (o.mPython)
		Py_XINCREF(o.mObject);
	if (mPython)
		Py_XDECREF(mObject);
	mPython = o.mPython;
	if (mPython) {
		mObject = o.mObject;
	} else {
		mCData = o.mCData;
		mElements = o.mElements;
		mHash = o.mHash;
		mElementType = o.mElementType;
	}
	return *this;
}


DelayedException *StringStoreElem::Claim(SimplePoolAllocator &allocator)
{
	_ASSERT(!mPython);
	size_t elen = mElementType == UNICODE ? sizeof( wchar_t ) : sizeof( char );
	char * tmp = (char*)allocator.align(mElements*elen, (int)elen);
	if (!tmp)
		return DelayedException::NoMem();
	memcpy(tmp, mCData, mElements*elen);
	mCData = tmp;
	return 0;
}


bool StringStoreElem::operator < (const StringStoreElem &rhs) const
{
	if(mPython)
		return false; //converted to python.  This happens at the end, so we just stop
	int cmp;
	if(mElementType < rhs.mElementType)
	{
		return true;
	}
	if(mElementType == UNICODE)
	{
		cmp = wmemcmp( mWData, rhs.mWData, min( mElements, rhs.mElements ) );
	}
	else
	{
		cmp = memcmp( mCData, rhs.mCData, min( mElements, rhs.mElements ) );
	}
	if (cmp != 0)
		return cmp<0;
	return mElements < rhs.mElements;
}


// Taken from VC10 xhash, since it doesn't exist anymore in VC2017, and we want to maintain compatible hashing
template <class _InIt>
inline size_t _Hash_value( _InIt _Begin, _InIt _End )
{	// hash range of elements
	size_t _Val = 2166136261U;

	while (_Begin != _End)
		_Val = 16777619U * _Val ^ (size_t)*_Begin++;
	return (_Val);
}


void StringStoreElem::Hash()
{
	//hash first 64 elements
	//TODO: See if this is perhaps too much.  We for example only interns strings under 30 chars.
	//Perhaps we need to hash as little as 10 chars for good performance and deal with collisions
	//in larger strings
	if (mPython)
		return;
	const size_t maxHash = 64;
	if (mElementType == UNICODE)
		mHash = _Hash_value(mWData, mWData+min(mElements,maxHash));
	else
	{
		// XOR with mElementType to ensure unique hashing between string and byte element types
		mHash = _Hash_value(mCData, mCData+min(mElements,maxHash)) ^ mElementType;
	}
}


bool StringStoreElem::ToPython()
{
	_ASSERT( !mPython );
	PyObject* p;
	switch(mElementType)
	{
	case BYTESTRING: {
		p = PyBytes_FromStringAndSize( mCData, mElements );
		break;
	}
	case STRING:
		p = PyUnicode_FromStringAndSize( mCData, mElements );
		break;
	case UNICODE: {
		p = PyUnicode_FromWideChar( mWData, mElements );
		if(p && mElements < 20)
		{
			//Intern short python strings from database
			PyUnicode_InternInPlace( &p );
		}
		break;
	}
	}

	if(!p)
		return false;
	mPython = true;
	mObject = p;
	return true;
}


// returns a borrowed reference to the python object
PyObject *StringStoreElem::GetPython()
{
	if (!mPython && !ToPython())
		return 0;
	_ASSERT(mPython);
	return mObject;
}
	

StringStore::StringStore(SimplePoolAllocator &allocator) :
	mAllocator(allocator),
	mSet(traits_t(), allocator_t(allocator))
{
	mMemSaved = 0;
}


DelayedException *StringStore::Insert(StringStoreElem* &res, char *data, size_t elems, bool isBytes)
{
	StringStoreElem tmp(data, elems, isBytes);
	std::pair<StringStoreSet_i, bool> r = mSet.insert(tmp);
	
	// This is slightly evil. The insert gives us back a const iterator
	// to the StringStoreElem either found or just added. This used to be a
	// regular iterator, but with VS2010 it is a const iterator.
	// It would be cleaner to change this to a map - separate the
	// key out from the value. We know that the ordering operator
	// used on the StringStoreElem will not change so this does work - no
	// operations we can do on the element will invalidate the set.
	// Note that the std::set documentation clearly states that
	// values should not change after they are added to the set.
	StringStoreSet_i& elIt = r.first;
	StringStoreElem& el = const_cast<StringStoreElem&>( *elIt );
	
	res = 0;
	if (r.second) {
		//aha, new insertion.

		DelayedException *e = el.Claim(mAllocator);
		if (e)
			return e;
	} else
		mMemSaved += elems;
	res = &el;
	return 0;
}


DelayedException *StringStore::Insert(StringStoreElem* &res, wchar_t *data, size_t elems)
{
	StringStoreElem tmp(data, elems);
	std::pair<StringStoreSet_i, bool> r = mSet.insert(tmp);

	// This is slightly evil. The insert gives us back a const iterator
	// to the StringStoreElem either found or just added. This used to be a
	// regular iterator, but with VS2010 it is a const iterator.
	// It would be cleaner to change this to a map - separate the
	// key out from the value. We know that the ordering operator
	// used on the StringStoreElem will not change so this does work - no
	// operations we can do on the element will invalidate the set.
	// Note that the std::set documentation clearly states that
	// values should not change after they are added to the set.
	StringStoreSet_i& elIt = r.first;
	StringStoreElem& el = const_cast<StringStoreElem&>( *elIt );

	res = 0;
	if (r.second) {
		//aha, new insertion.
		DelayedException *e = el.Claim(mAllocator);
		if (e)
			return e;
	} else
		mMemSaved += elems*sizeof(wchar_t);
	res = &el;
	return 0;
}


size_t StringStore::GetMemSaved() const
{
	return mMemSaved;
}