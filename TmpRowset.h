/* 
	*************************************************************************

	TmpRowset.h

	Author:    Kristjan Valur Jonsson
	Created:   Feb. 2005
	OS:        Win32
	Project:   EVE Server Database Access

	Description:   

		Classes that store a rowset temporarily during traversal. We cannot
		create python rowsets directly since this happens on a thread, and python
		is'nt multithreaded.  So, here we create something very similar to python
		rowsets.
		
	Dependencies:

		Python

	(c) CCP 2005

	*************************************************************************
*/

#ifndef _TMPROWSET_H_
#define _TMPROWSET_H_

#include <string.h> //for _strdup()
#include <vector>
#include <string>
#include <atlstr.h>
#include <atldbcli.h>
#include "utils.h"
#include "DelayedException.h"
#include "StringStore.h"
#include "BlockAllocator.h"


//A temporary thing, use this to locate the place where we are getting error results with no exception
template <class T>
void ForceException(T v, const char *msg){
	if (!v && !PyErr_Occurred( )) {
		std::string s("Forced Exception, Notify Kris! : ");
		s += msg;
		PyErr_SetString(PyExc_RuntimeError, s.c_str());
	}
}

//A subclass from CDynamicAccessor that provides methods to access the structure
//linearly, not through the ordinalID, which costs mucho runtime.  Code copied from atldbcli.h
class OurAccessor : public CDynamicParameterAccessor
{
public:
	DBORDINAL GetOrdinal_ByIndex(int index) {
		const int nColumn = index;
		CCP_ASSERT(nColumn < (int)GetColumnCount());
		return m_pColumnInfo[nColumn].iOrdinal;
	}
	LPOLESTR GetColumnName_ByIndex(int index) {
		const int nColumn = index;
		return m_pColumnInfo[nColumn].pwszName;
	}
	DBTYPE GetColumnType_ByIndex(int index) {
		const int nColumn = index;
		return m_pColumnInfo[nColumn].wType;
	}
	DBSTATUS GetStatus_ByIndex(int index) {
		const int nColumn = index;
		DBBYTEOFFSET nOffset = (DBBYTEOFFSET)(ULONG_PTR)m_pColumnInfo[nColumn].pTypeInfo;
		IncrementAndAlignOffset( nOffset, m_pColumnInfo[nColumn].ulColumnSize, __alignof(DBLENGTH) );
		IncrementAndAlignOffset( nOffset, sizeof(DBLENGTH), __alignof(DBSTATUS) );
		return *(DBSTATUS*)( m_pBuffer + nOffset );
	}
	DBLENGTH GetLength_ByIndex(int index) {
		const int nColumn = index;
		DBBYTEOFFSET nOffset = (DBBYTEOFFSET)(ULONG_PTR)m_pColumnInfo[nColumn].pTypeInfo;
		IncrementAndAlignOffset( nOffset, m_pColumnInfo[nColumn].ulColumnSize, __alignof(DBLENGTH) );
		return *(DBLENGTH*)( m_pBuffer + nOffset );
	}
	template <class ctype>
	void GetValue_ByIndex(int index, ctype *pData) {
		_GetValue(index, pData);
	}
};
			



struct ColumnDescriptor
{
	ColumnDescriptor(const char *name) : mName(name), mType(0), mOffset(0), mSize(0){}
	static DelayedException *TypeTranslate(DBTYPE &type, int &size);

	std::string mName;
	int mOffset;
	DBTYPE mType;
	char mSize;
};


//A row descriptor object that isn't python.  We can't always create python objects
class NSession;
class RowDescriptor
{
public:
	RowDescriptor(NSession *sess) : mNSession(sess) {}
	DelayedException *Init(OurAccessor &a);
	PyObject *ToPython(PyObject *blueModule);

	//members.  First a list of descriptors
	typedef std::vector<ColumnDescriptor> columnList_t;
	columnList_t mColumnList;
	
	//row data is laid out thus:
	//[binary columns , null flags, objects]
	//there is a null flag for every column, indexed by column number, also
	//for Object columns (even though we could use PyNone there) for simplicity.

	int mSNull;	//start of null flags, in bits
	int mDataLen; //length of integer data and null flags only, in bytes
	int mSObjects;  //offset to first object, in pointers.
	int mNObjects; //number of objects trailing data
	
	int mTotalLen; //total length of data (including object pointers)
	
	NSession * const mNSession; //convenient place to store this
};


class Row
{
public:
	static DelayedException *NewRow(Row **res, SimplePoolAllocator &ba, DBLENGTH &recvLen, RowDescriptor const &d, OurAccessor &a, int numCols, StringStore &e);
	static void DeleteRow(Row *r, RowDescriptor const &d);
	PyObject *ToPython(RowDescriptor const &d, PyObject *pyrd, class ToPythonCtxt &ctxt);

private:
	DelayedException *Init(DBLENGTH &recvData, RowDescriptor const &d, OurAccessor &a, int numCols, StringStore &stringStore);
	
	void *GetDataPtr(const RowDescriptor &d, DBORDINAL i) const {return (void*)((char*)&mData + d.mColumnList[i].mOffset);}
	int GetDataSize(const RowDescriptor &d, DBORDINAL i) const {return d.mColumnList[i].mSize;}

	char *GetBitPtr(DBORDINAL &bit, const RowDescriptor &d, DBORDINAL bitoffset) const {
		bit = bitoffset % 8;
		DBORDINAL byte = bitoffset / 8;
		CCP_ASSERT((int)byte < d.mDataLen);
		return (char*)&mData + byte;
	}

	void SetBit(const RowDescriptor &d, DBORDINAL bitoffset) {
		DBORDINAL bit;
		char *ptr = GetBitPtr(bit, d, bitoffset);
		*ptr |= 1<<bit;
	}
	void ClrBit(const RowDescriptor &d, DBORDINAL bitoffset) {
		DBORDINAL bit;
		char *ptr = GetBitPtr(bit, d, bitoffset);
		*ptr &= (char) ~(1<<bit);
	}
	bool GetBit(const RowDescriptor &d, DBORDINAL bitoffset) const {
		DBORDINAL bit;
		char *ptr = GetBitPtr(bit, d, bitoffset);
		return ((*ptr) & (1<<bit)) != 0;
	}
	
	template<class T>
	void GetData(const RowDescriptor &d, T &r, DBORDINAL i) const {
		ASSERT(GetDataSize(d, i) == sizeof(T));
		r = *((T*)GetDataPtr(d, i));
	}
	template<class T>
	void SetData(const RowDescriptor &d, const T &r, DBORDINAL i) {
		CCP_ASSERT(GetDataSize(d, i) == sizeof(T));
		*((T*)GetDataPtr(d, i)) = r;
	}

	//special object accessor
	template<class T>
	void GetObject(const RowDescriptor &d, int i, T* &o) {o = ((T**)&mData)[d.mSObjects + i];}
	template<class T>
	void SetObject(const RowDescriptor &d, int i, T* o) {((T**)&mData)[d.mSObjects + i] = o;}

	//specializations for bool
	template<>
	void GetData<bool>(const RowDescriptor &d, bool &r, DBORDINAL i) const {
		CCP_ASSERT(d.mColumnList[i].mType == DBTYPE_BOOL && d.mColumnList[i].mSize == -1);
		r = GetBit(d, d.mColumnList[i].mOffset);
	}
	
	template<>
	void SetData<bool>(const RowDescriptor &d, const bool &r, DBORDINAL i) {
		CCP_ASSERT(d.mColumnList[i].mType == DBTYPE_BOOL && d.mColumnList[i].mSize == -1);
		if (r)
			SetBit(d, d.mColumnList[i].mOffset);
		else
			ClrBit(d, d.mColumnList[i].mOffset);
	}

	//Link in a warning at the head of the list.
	void Warn(DelayedException *w) {
		w->mNext = mWarning;
		mWarning = DelayedException_ptr(w);
	}
	
public:
	DelayedException_ptr mWarning; // Any warnings raised

private:
	__int64 mData;
};

class ToPythonCtxt
{
public:
	ToPythonCtxt(PyObject *blue, PyObject *beNice, int beNiceEvery, PyObject *getMem) :
		mBlue(blue), mBeNice(beNice), mBeNiceEvery(beNiceEvery), mGetMem(getMem)
	{
		mRowCounter = mTotalPyBytes = mLastPyBytes = 0;
	}
	size_t GetMem() const
	{
		if (!mGetMem)
			return 0;
		BluePy m(PyObject_CallObject(mGetMem, 0));
		if (m) {
			size_t now;
			if (sizeof(size_t) == 4)
				now = PyLong_AsUnsignedLongMask( m );
			else
				now = (size_t)PyLong_AsUnsignedLongLongMask( m );
			if (now != (size_t)-1)
				return now;
		}
		PyErr_Clear();
		return 0;
	}
	bool BeNice() const
	{
		if (!mBeNice)
			return true;
		BluePy r(PyObject_CallObject(mBeNice, 0));
		return r ? true : false;
	}

	PyObject *mBlue;
	PyObject *mBeNice;
	PyObject *mGetMem;
	size_t	mRowCounter;
	size_t	mTotalPyBytes;
	size_t	mLastPyBytes;
	int mBeNiceEvery;
};

class TmpRowset
{
public:
	TmpRowset(NSession *s, SimplePoolAllocator &allocator, StringStore &store) :
		mRD(s), mRC(DB_COUNTUNAVAILABLE), mAllocator(allocator), mStringStore(store),
		mRows(rows_a(allocator))
	{}
	typedef CAccessorRowset<OurAccessor, CBulkRowset> rowset_t;
	DelayedException *Get(DBLENGTH &recvLen, rowset_t &a);
	~TmpRowset();

	//dissolve and change to python;
	PyObject *ToPython(ToPythonCtxt &ctxt);

	DBROWCOUNT mRC;

private:
	//typedef std::vector<Row*> rows_t;
	typedef StlPoolAllocator<Row*> rows_a;
	typedef std::list<Row*, rows_a> rows_t;
	typedef rows_t::iterator rows_i;

	RowDescriptor mRD;
	rows_t mRows;
	SimplePoolAllocator &mAllocator;
	StringStore &mStringStore;
};


class TmpRowsetList
{
public:
	TmpRowsetList() : mProcResult(0) , mAllocator(8*1024), mStringStore(mAllocator){}
	~TmpRowsetList();
	typedef CCommand<OurAccessor, CBulkRowset, CMultipleResults> command_t;
	DelayedException *Get(DBLENGTH &recvLen, NSession *sess, command_t &cmd, DBROWCOUNT rc);

	//dissolve and change to python.
	PyObject *ToPython(ToPythonCtxt &ctxt);
	size_t GetMemSaved() const {return mStringStore.GetMemSaved();}

private:
	typedef std::vector<TmpRowset*> rowsets_t;
	typedef rowsets_t::iterator rowsets_i;
	
	rowsets_t mRowsets;
	int mProcResult; //used when list is empty
	SimplePoolAllocator mAllocator;
	StringStore mStringStore; //for string reuse in the rowset
};


#endif //_TMPROWSET_H_

