#include "stdafx.h"
#include "TmpRowset.h"
#include "NSession.h"

#include <comutil.h>

extern ITaskletTimer *ttimer;
#define AUTOTASKLET(c) AutoTasklet _at(ttimer, c)

#if 0
#define TTIMER1 AUTOTASKLET
#define TTIMER2 AUTOTASKLET
#endif

#define TTIMER1 __noop
#define TTIMER2 __noop


DelayedException *ColumnDescriptor::TypeTranslate(DBTYPE &type, int &size)
{
	type &= 0xff;
	DBTYPE stype = type;
	switch(stype) {
	case DBTYPE_BOOL:
		size = 0; break;
	case DBTYPE_I1:
	case DBTYPE_UI1:
		size = 1; break;
	case DBTYPE_I2:
	case DBTYPE_UI2:
		size = 2; break;
	case DBTYPE_I4:
	case DBTYPE_UI4:
	case DBTYPE_R4:
		size = 3; break;
	case DBTYPE_I8:
	case DBTYPE_UI8:
	case DBTYPE_R8:
	case DBTYPE_CY:
	case DBTYPE_FILETIME:
		size = 4; break;
	case DBTYPE_DBTIMESTAMP:
	case DBTYPE_DBDATE:
	case DBTYPE_DBTIME2:
		type = DBTYPE_FILETIME; //will be converted when read
		size = 4; break;
	case DBTYPE_BSTR:
		type = DBTYPE_WSTR;  //they are the same.
	case DBTYPE_STR:
	case DBTYPE_WSTR:
	case DBTYPE_BYTES:
		size = 5; break; //signals a pointer.
	default: {
		CString msg;
		msg.Format("Unexpected type %d", type);
		PYDBERROR(msg); }
	}
	return 0;
}


//Set up the object from the DynamicAccessor
// layout is: [binary columns, null flags, Object columns ]
DelayedException *RowDescriptor::Init(OurAccessor &a)
{
	//first pass.  Build column list, count sizes;
	int sizes[6] = {0};
	mNObjects = 0;
	mColumnList.clear();
	DBORDINAL i;
	DBORDINAL nc = a.GetColumnCount();
	for(i = 0; i<nc; i++) {
		CW2A name(a.GetColumnName(i+1));
		mColumnList.push_back(ColumnDescriptor(name));
		ColumnDescriptor &cd = mColumnList.back();
		DBTYPE type;
		if (!a.GetColumnType(i+1, &type)) {
			CString msg;
			msg.Format("Couldn't get column type of column %d", i);
			PYDBERROR(msg);
		}
		int size;
		DelayedException *de = ColumnDescriptor::TypeTranslate(type, size);
		if (de)
			return de;
		sizes[size]++;
		cd.mType = type;
		cd.mSize = size;
	}

	//now, compute offsets, starting with the largest (real) size
	int offsets[5];
	int offset = 0; //totaloffset to extra data
	for(i=4; i>0; i--) { //regular ints
		offsets[i] = offset;
		offset += sizes[i]*(1<<(i-1));
	}
	//bools
	offset *= 8; //turn into bits;
	offsets[0] = offset; //start of boolean flags, in bits
	offset += sizes[0]; //space required for bits

	//null flags
	mSNull = offset;	//start of null flags in bits
	offset += (int)mColumnList.size(); //space required for bits,

	//round up to bytes again
	offset = (offset+7)/8;

	mDataLen = offset; //this is the length of the data and null flags (in bytes).

	//pointers
	mNObjects = sizes[5];
	if (mNObjects) {
		offset = (offset + (sizeof(void*)-1)) & ~(sizeof(void*)-1); //round up in size.
		mSObjects = offset / sizeof(void*); //get offset for the object counter.
		mTotalLen = offset + mNObjects*sizeof(void*);
	} else {
		mSObjects = 0;
		mTotalLen = mDataLen;
	}

	//second pass, compute offsets and modify sizes
	for(i = 0; i<(int)mColumnList.size(); i++) {
		ColumnDescriptor &cd = mColumnList[i];
		if (cd.mSize > 0 && cd.mSize < 5) {
			//an intergral object or flag
			cd.mOffset = offsets[cd.mSize];
			int size = 1<<(cd.mSize-1);
			offsets[cd.mSize] += size;
			cd.mSize = size;
		} else if (cd.mSize==0) {
			//bools
			cd.mOffset = offsets[0]++;
			cd.mSize = -1;
		} else {
			//pointers
			cd.mOffset = offset;
			cd.mSize = sizeof(void*);
			offset += cd.mSize;
		}
	}
	return 0;
}


PyObject *RowDescriptor::ToPython(PyObject *blueModule)
{
	TTIMER2("DB::NSession::ToPython::RowDescriptor");
	BluePy raw(PyCapsule_New(&mColumnList, "DBRowDescriptor", 0));
	if (!raw)
		return 0;
	return PyObject_CallMethod(blueModule, (char*)"DBRowDescriptor", (char*)"O", raw.o);
}


DelayedException *Row::NewRow(Row ** res, SimplePoolAllocator &ba, DBLENGTH &recvLen, RowDescriptor const &d, OurAccessor &a, int numCols, StringStore &stringStore)
{

	int len = offsetof(Row, mData)+d.mTotalLen;
	void *data = ba.malloc(len);
	*res = 0;
	if (!data)
		return DelayedException::NoMem("couldn't allocate %d bytes for Row", len);
	Row *row = new(data) Row();
	DelayedException *exc = row->Init(recvLen, d, a, numCols, stringStore);
	if (exc) {
		row->~Row();
		return exc;
	}
	*res = row;
	return 0;
}


void Row::DeleteRow(Row *r, RowDescriptor const &d)
{
	r->~Row();
}


DelayedException *Row::Init(DBLENGTH &recvLen, RowDescriptor const &d, OurAccessor &a, int nc, StringStore &stringStore)
{
	memset(&mData, 0, d.mTotalLen);
	recvLen = 0;
	for(int index = 0; index<nc; index++) {
		const DBORDINAL ordinal = a.GetOrdinal_ByIndex(index);
		CCP_ASSERT(ordinal>0);
		const DBORDINAL i = ordinal-1;
		
		DBSTATUS status = a.GetStatus_ByIndex(index);
		if( status == DBSTATUS_S_ISNULL	 ) {
			SetBit(d, d.mSNull+i);
			continue;
		}
		if(status != DBSTATUS_S_OK ) {
			CString msg;
			if (status == DBSTATUS_S_TRUNCATED) {
				msg.Format("column %d truncated.", ordinal);
				Warn(PYDBWARN(msg));
			} else {
				msg.Format("Unexpected status of column %d: %d", ordinal, status);
				PYDBERROR(msg);
			}
		}
		
		DBTYPE dbtype = a.GetColumnType_ByIndex(index);
		if (dbtype & (DBTYPE_ARRAY | DBTYPE_VECTOR)) {
			CString msg;
			msg.Format("Unexpected type of column %d: %d", ordinal, dbtype);
			PYDBERROR(msg);
		}

		//store stats on received data
		DBLENGTH dblen = a.GetLength_ByIndex(index);
		if (dblen != ~0)
			recvLen += dblen;

		bool byref = !!(dbtype & DBTYPE_BYREF);
		switch (dbtype&0xff)
		{
		case DBTYPE_I1:
		case DBTYPE_UI1: {
			__int8 b;
			a.GetValue_ByIndex(index, &b);
			SetData(d, b, i);
			break; }
		case DBTYPE_I2:
		case DBTYPE_UI2: {
			__int16 b;
			a.GetValue_ByIndex(index, &b);
			SetData(d, b, i);
			break; }
		case DBTYPE_I4:
		case DBTYPE_UI4:
		case DBTYPE_R4: {
			__int32 b;
			a.GetValue_ByIndex(index, &b);
			SetData(d, b, i);
			break; }
		case DBTYPE_I8:
		case DBTYPE_UI8:
		case DBTYPE_R8:
		case DBTYPE_CY:
		case DBTYPE_FILETIME: {
			__int64 b;
			a.GetValue_ByIndex(index, &b);
			SetData(d, b, i);
			break; }
		case DBTYPE_BOOL: {
			VARIANT_BOOL b;
			a.GetValue_ByIndex(index, &b);
			bool bb = !!b;
			SetData(d, bb, i);
			break; }
		case DBTYPE_STR: {
			char * s = byref ? *(char**)a.GetValue(i+1) : (char*)a.GetValue(i+1);

			//Convert string to wide string so when retrieved the data comes through to Python3 as string, not bytes.
			PyObject* tmp = PyUnicode_DecodeASCII( s, strlen( s ), nullptr );
			if( !tmp )
			{
				CString msg;
				msg.Format( "Conversion to unicode failed on column %d(%s)", i, (const char*)CW2A( a.GetColumnName( i + 1 ) ) );
				return DelayedException::New( msg );
			}
			wchar_t* sw = PyUnicode_AsWideCharString( tmp, nullptr );
			if( !sw )
			{
				CString msg;
				msg.Format( "Failed to retrieve wide string from unicode object on column %d(%s)", i, (const char*)CW2A( a.GetColumnName( i + 1 ) ) );
				return DelayedException::New( msg );
			}
			StringStoreElem* elem;
			DelayedException* e = stringStore.Insert( elem, sw, wcslen( sw ) );
			Py_DecRef( tmp );
			if (e)
				return e;
			SetData(d, elem, i);
			break;}
		case DBTYPE_WSTR:
		case DBTYPE_BSTR: {
			wchar_t *s = byref ? *(wchar_t**)a.GetValue(i+1) : (wchar_t*)a.GetValue(i+1);
			StringStoreElem *elem;
			DelayedException *e = stringStore.Insert(elem, s, wcslen(s));
			if (e)
				return e;
			SetData(d, elem, i);
			break;}
		case DBTYPE_BYTES: {
			DBLENGTH len;
			if(!a.GetLength(i+1, &len))
				continue;
			char *src = (char*)a.GetValue(i+1);
			if (byref && src)
				src = *(char**)src;
			if (!src)
				len = 0;
			StringStoreElem *elem;
			DelayedException *e = stringStore.Insert(elem, src, len);
			if (e)
				return e;
			SetData(d, elem, i);
			break;}
		case DBTYPE_DBTIMESTAMP: //sql "datetime", "smalldatetime", "datetime2"
		case DBTYPE_DBDATE:		 //sql "date"
		{
			DBLENGTH len;
			if(!a.GetLength(i+1, &len))
				continue;
			HRESULT	hr;
			DBLENGTH dstlen;
			DBSTATUS srcstatus = status;
			DBSTATUS dststatus;
			FILETIME ftime;
			
			hr = d.mNSession->mConv->DataConvert(
				dbtype&0xff,
				DBTYPE_FILETIME,
				len,
				&dstlen,
				a.GetValue(i+1),
				&ftime,
				sizeof (ftime),
				srcstatus,
				&dststatus,
				0, 0,
				DBDATACONVERT_DEFAULT
				);
			if (FAILED(hr)) {
				CString msg;
				msg.Format("Data conversion failed on column %d(%s)", i, (const char*)CW2A(a.GetColumnName(i+1)));
				return DelayedException::New(hr, msg);
			}
			SetData(d, ftime, i);
			break; }
		case DBTYPE_DBTIME2: {
			//The time.  Convert to FILETIME units (1e-7s)
			const DBTIME2 &t2 = *reinterpret_cast<const DBTIME2*>(a.GetValue(i+1));
			unsigned __int64 time = 3600*t2.hour + 60*t2.minute + t2.second;
			time *= 10000000;
			time += t2.fraction / 100; // (ns to 1e-7s);
			SetData(d, time, i);
			break; }
		default: {
			CString msg;
			msg.Format("Unexpected type of column %d(%s): %d", i, (const char*)CW2A(a.GetColumnName(i+1)), dbtype);
			PYDBERROR(msg); }
		}
	}
	return 0;
}


PyObject *Row::ToPython(const RowDescriptor &rd, PyObject *pyrd, ToPythonCtxt &ctxt)
{
	// Raise any warning (or exception if turns out to be one
	if (mWarning.get()) {
		if (!mWarning->Raise())
			return 0;
	}

	BluePy data(PyCapsule_New(&mData, "DBRow", 0));
	if (!data)
		return 0;
	//storing the method name here saves us loads of time in large rowsets
	static PyObject *method = 0;
	if (!method) {
		method = PyUnicode_InternFromString( "DBRow" );
		if (!method)
			return 0;
	}
	//convert any pointers to StringStore elements to the proper Python pointers.
	//this is a once only operation, and so the row cannot be converted to Python more than once.
	for(int i = 0; i<rd.mNObjects; i++) {
		StringStoreElem *ptr;
		GetObject(rd, i, ptr);
		if (ptr) {
			PyObject *pystr = ptr->GetPython();
			SetObject(rd, i, pystr);
		}
	}

	return PyObject_CallMethodObjArgs(ctxt.mBlue, method, pyrd, data, 0);
}


DelayedException *TmpRowset::Get(DBLENGTH &recvLen, rowset_t &rs)
{
	DelayedException *de = mRD.Init(rs);
	if (de)
		return de;
	
	mRows.clear();
	HRESULT hr = rs.MoveFirst();
	if (FAILED(hr))
		return DelayedException::New(hr, "MoveFirst failed");
	int got = 0;
	recvLen = 0;
	const int numCols = (int)rs.GetColumnCount(); //expensive, pays to take this outside loop
	while(hr != DB_S_ENDOFROWSET) {
		Row *rp;
		DBLENGTH len;
		de = Row::NewRow(&rp, mAllocator, len, mRD, rs, numCols, mStringStore);
		if (de)
			return de;
		recvLen += len;
		got++;
		mRows.push_back(rp);
		hr = rs.MoveNext();
		if (FAILED(hr))
			return DelayedException::New(hr, "MoveFirst failed");
	}
	return 0;
}


TmpRowset::~TmpRowset()
{
	for(rows_i i = mRows.begin(); i != mRows.end(); i++)
		if (*i)
			Row::DeleteRow(*i, mRD);
}


//returns a tuple: (rowdscriptor, [row, row, ...])
PyObject *TmpRowset::ToPython(ToPythonCtxt &ctxt)
{
	TTIMER1("DB::NSession::ToPython::RowSet");
	BluePyTuple rs(2);
	if (!rs) return 0;
	BluePy pyrd(mRD.ToPython(ctxt.mBlue));
	rs.Set(0, pyrd); //row descriptor
	int len = (int)mRows.size();
	BluePyList list(len);
	if (!list) return 0;
	rs.Set(1, list);
	
	rows_i ri = mRows.begin();
	for(int i = 0; i<len; i++, ri++) {
		BluePy pyrow((*ri)->ToPython(mRD, pyrd, ctxt));
		if (!pyrow || !list.Set(i, pyrow))
			return 0;
		//destroy the old data as we go along.
		Row::DeleteRow(*ri, mRD);
		*ri = 0;
		if (ctxt.mBeNiceEvery && ((++ctxt.mRowCounter) % ctxt.mBeNiceEvery) == 0 && ctxt.mBeNice) {
			//be nice.  But first, count allocated python mem
			size_t now = ctxt.GetMem();
			ctxt.mTotalPyBytes += now - ctxt.mLastPyBytes;
			if (!ctxt.BeNice())
				return 0; //Probably tasklet got killed
			ctxt.mLastPyBytes = ctxt.GetMem();
		}
	}

	return rs.Detach();
}


//The first result has already been bound
DelayedException *TmpRowsetList::Get(DBLENGTH &recvLen, NSession *sess, command_t &rs, DBROWCOUNT rc)
{	
	//note that the implicit GetNextResult in the rc.Open command may have failed.  This failure is (erroneously) not handled
	//with a hresult, but we must check the m_spRowset to see if it zero.  In this case, we must get the errorRecords, and
	//try again.

	HRESULT hr = S_OK;
	//get result value from the parameters
	if (!rs.GetParam(1, &mProcResult)) { //Get return value
		mProcResult=0;
		recvLen = 0;
	} else
		recvLen = sizeof(mProcResult);

	int got = 0;
	do {
		//handle the failure mode of GetNextResult at the start of each loop point
		//(Command->Open() had an implicit GetNextResult)
		if (!rs.m_spRowset) {
			hr = rs.GetNextResult(&rc, true);
			if (hr == DB_S_NORESULT || !rs.m_spRowset) //yes, sometimes we get the null m_spRowset. This API is broken.
				break;
			if (FAILED(hr))
				return DelayedException::New(hr, "GetNextResult() failed (1)");
		}
			
		TmpRowset *trs = new TmpRowset(sess, mAllocator, mStringStore);
		if (!trs)
			return DelayedException::NoMem("Couldn't allocate memory for row");
		DBLENGTH len;
		DelayedException *e = trs->Get(len, rs);
		trs->mRC = rc; //rowcount
		if (e) {
			delete trs;
			return e;
		}
		recvLen += len;
		mRowsets.push_back(trs);
		got++;
		//get the next stuff for the next round of the loop
		hr = rs.GetNextResult(&rc, true);
		if (hr == DB_S_NORESULT)
			break;
	} while(hr == S_OK);
	if (FAILED(hr))
		return DelayedException::New(hr, "GetNextResult() failed");
	return 0;
}


TmpRowsetList::~TmpRowsetList()
{
	for(rowsets_i i = mRowsets.begin(); i != mRowsets.end(); i++)
		delete *i;
}


PyObject *TmpRowsetList::ToPython(ToPythonCtxt &ctxt)
{
	int len = (int)mRowsets.size();
	if (!len)
		return PyLong_FromLong( mProcResult );

	ctxt.mLastPyBytes = ctxt.GetMem();
	BluePyList list(len);
	if (!list)
		return 0;

	if (len) {
		BluePy blueModule(PyImport_Import(BluePyStr("blue")));
		if (!blueModule)
			return 0;
		
		for(int i = 0; i<len; i++) {
			TmpRowset * &trs = mRowsets[i];
			if (trs) {
				BluePy rs(trs->ToPython(ctxt));
				if (!rs || !list.Set(i, rs))
					return 0;
				delete trs;
				trs = 0;
			}
		}
	}
	ctxt.mTotalPyBytes += ctxt.GetMem() - ctxt.mLastPyBytes;
	return list.Detach();
}
