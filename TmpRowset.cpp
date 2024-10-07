#include "StdAfx.h"
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
		de = Row::NewRow(&rp, mAllocator, len, mRD, rs, numCols, mStore);
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
			
		TmpRowset *trs = new TmpRowset(sess, mAllocator, mStore);
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
