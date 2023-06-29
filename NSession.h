/* 
	*************************************************************************

	Session.h

	Author:    Kristjan Valur Jonsson
	Created:   feb. 2005
	OS:        Win32
	Project:   EVE Server Database Access

	Description:   

		A new database session object, which generates new style rowsets


	Dependencies:

		Python

	(c) CCP 2005

	*************************************************************************
*/

#ifndef _NSESSION_H_
#define _NSESSION_H_

#include "Connection.h"
#include "PyTemplates.h"
#include <stackless_api.h>

#include "SessionPool.h"
#include "tmprowset.h"

#include <atlstr.h>
#include <msdadc.h>	// for IDataConvert

#include <stacklessio.h>


extern ITaskletTimer *ttimer;
#define AUTOTASKLET0(c) AutoTasklet _at(ttimer, (c))
#define AUTOTASKLETC(c, cond) AutoTasklet _at(ttimer, (c), (cond))
#define AUTOTASKLET1(c) AUTOTASKLETC(c, mTimerDetail>=1)
#define AUTOTASKLET2(c) AUTOTASKLETC(c, mTimerDetail>=2)
#define AUTOTASKLET3(c) AUTOTASKLETC(c, mTimerDetail>=3)


BLUE_DECLARE( Connection );

class NSession;

class SQLCommand:
	public CCommand<OurAccessor, CBulkRowset, CMultipleResults>
{
public:
	SQLCommand(NSession *s) : mNSession(s) {}
	bool Prepare(size_t &paramLen, CSession *session, PyObject *schema, PyObject *args);
	PyObject *Raise(const char *msg, HRESULT hr=S_OK, const CDBErrorInfo *errorInfo=0, ULONG nErrors=0);
	
private:
	bool SelectProcedure(CString &csSQL, const char *procname, PyObject *schema);

	//These two are reimplementations from the ATL, that bind parameters using cached
	//scheme data, rather than invoke a server roundtrip to inquire about the parameters.
	HRESULT GetParameterInfo (DB_UPARAMS *pcParams, DBPARAMINFO **prgParamInfo, OLECHAR **ppNamesBuffer, 
			PyObject *procParamSchema);
	HRESULT BindParameters(HACCESSOR* pHAccessor, ICommand* pCommand, PyObject *procParamScema,
			void** ppParameterBuffer, bool fBindLength = false, bool fBindStatus = false, PyObject *params = 0) throw();
	SSIZE_T GetStringSize(DBPARAMINFO *info, PyObject *params);
	bool SetDefaultParams();
	bool SetParamsFromList(size_t &paramLen, PyObject *plist);
	bool SetParamsFromDict(size_t &paramLen, PyObject *pdict);
	bool SetPyParam(size_t &len, DBORDINAL param, PyObject *val);
	template<class T>
	bool SetPyParamInt(size_t &len, DBORDINAL col, PyObject *value);

	NSession * const mNSession;
};


// not really a session in the OLEDB sense, more like a "connection"
class NSession : 
	public PyXObject2<NSession>
{
	typedef PyXObject2<NSession> Parent;
	typedef CCommand<CDynamicParameterAccessor, CArrayRowset, CMultipleResults> command_t;

public:
	PYTHON_CLASS((char*)"db.NSession");

	NSession();
	~NSession();

	//initialization
	static PyObject *_New(PyTypeObject*type, PyObject *args, PyObject *kw);
	bool Init(PyObject *args);

	PYTHON_METHODS_BEGIN()
		METHOD_VARARGS(Execute, "Executes a stored proc and returns rowset(s) or integer value.")
		METHOD_VARARGS(GetSchema, "get the database schema")
		METHOD_NOARGS(GetSessionStatus, "Get status of sessions")
		METHOD_NOARGS(GetSessionSettings, "Get settings for session pool")
		METHOD_O(SetSessionSettings, "Set settings for session pool")
	PYTHON_METHODS_END()
	PYTHON_GETSET_BEGIN()
		PYTHON_GETINT64( (char*)"totalBytesSentParam", mTotalBytesSentParam, (char*)"Total bytes sent in out params.")
		PYTHON_GETINT64( (char*)"totalBytesReceived", mTotalBytesReceived, (char*)"Total bytes received")
		PYTHON_GETINT64( (char*)"lastPyBytes", mLastPyBytes, (char*)"python bytes allocated in last request")
	PYTHON_GETSET_END()
	PYTHON_MEMBERS_BEGIN()
        PYTHON_MEMBER( (char*)"blobSizeLimit", T_INT, mBlobSizeLimit, 0)
		PYTHON_MEMBER( (char*)"lastWallclockTime", T_DOUBLE, mLastWallclockTime, READONLY)
		PYTHON_MEMBER( (char*)"lastKernelTime", T_DOUBLE, mLastKernelTime, READONLY)
		PYTHON_MEMBER( (char*)"lastUserTime", T_DOUBLE, mLastUserTime, READONLY)
		PYTHON_MEMBER( (char*)"beNiceEvery", T_INT, mBeNiceEvery, 0)
		PYTHON_MEMBER( (char*)"allowSync", T_INT, mAllowSync, 0)
		PYTHON_MEMBER( (char*)"timerDetail", T_INT, mTimerDetail, 0)
		PYTHON_MEMBER( (char*)"lastStringReuse", T_INT, mLastStringReuse, READONLY)
	PYTHON_MEMBERS_END()

	static bool InitType(PyTypeObject *type) 
	{
		type->tp_weaklistoffset = offsetof(NSession, mWeakrefList);
		return true;
	}

	CComPtr<IDataConvert> mConv; //for data conversion
	BluePy mBlue; //shortcut to blue module
	BluePy mYield; //shortcut to blye.pyos.Synchro.Yield
	BluePy mBeNice; //shortcut to blue.pyos.BeNice
	BluePy mGetMem; //shortcut to sys.getpymalloced
	int mBeNiceEvery; //line interval to call benice.  0 = off
	int mAllowSync; //if true, synchronous calls on noblock tasklets are allowd
	int mTimerDetail; //detail of tasklet timers. 0 is default, up this to increase.

private:
	PyObject *mWeakrefList; //weak references here
	PyObject *Execute(PyObject *args); //The main man
	PyObject *GetSchema(PyObject *args); //returns a new ref
	PyObject *GetSessionStatus();
	PyObject *GetSessionSettings();
	PyObject *SetSessionSettings(PyObject *settings);
	
	PyObject *GetSchemaB(bool refresh); //returns our cached schema.
	
	struct Request : public IOWorker
	{
		Request(NSession *ns) : mNSession(ns), mCommand(ns), mSessionKeeper(ns->mSessionPool) {
			mSession = 0;
		}
		~Request() {
			if (mException.get()) {
				DiscardSession();
			} else
				ReleaseSession();
		}

		// override virtual from IOEvent.
		// Release the command, which can happen without the GIL held.
		// The session can only be released with the GIL since it may
		// involve stackless pumping.
		void PreDelete() 
		{
			mCommand.Close();
			mCommand.ReleaseCommand();
		}

		void ThreadFunc(); //the worker function
		void Execute(); //when we do a direct execute

		PyObject *Raise();
		bool GetSession(int timerDetail); //call on main thread
		void ReleaseSession();
		void DiscardSession(); //use this if an error occurred (from worker), since session state may be borked.
		
		double mWallclockTime;
		double mKernelTime;
		double mUserTime;
		class NSession * const mNSession;  //the nSession object
		SessionKeeper mSessionKeeper;
		ATL::CSession *mSession; //the db session to run on
		TmpRowsetList mResult;
		SQLCommand mCommand;
		DelayedException_ptr mException;
		size_t mBytesReceived; //length of data
	};
	
	bool Prepare(Request &rq, PyObject *args);
	PyObject *ExecuteBlock(Request &rq);
	PyObject *ExecuteDirectly(Request &rq);
	
private:
	
	CDataSource mDataSource; //each session has its own datasource object.  suboptimal.
	SessionPoolPtr mSessionPool;
	ConnectionPtr mConnection; //must keep reference to connection, to keep it alive.
	BluePy mSchema;		 //the database schema., from the connection

	int mBlobSizeLimit;
	
	
	//Stats:
	double mLastWallclockTime;
	double mLastKernelTime;
	double mLastUserTime;
	__int64 mLastPyBytes;
	__int64 mTotalBytesSentParam;
	__int64 mTotalBytesReceived;
	int		mLastStringReuse;
};


//A simple SequentialStream wrapper around a pythonbuffer thing.
//It borrows the reference to its python object:  It is run in a thread
//and addrefing and decrefing is therefore not safe.  However, the caller
//owns a reference so this is ok.
class PythonBuff : public ISequentialStream
{
public:
	PythonBuff(PyObject *);
	~PythonBuff();
	bool Valid() const
	{
		return m_buff != nullptr;
	}
	size_t GetLength() const
	{
		return m_size;
	}

	HRESULT WINAPI Read(void* pv, ULONG cb, ULONG* got);
	HRESULT WINAPI Write(const void* pv, ULONG cb, ULONG* written) {return E_NOTIMPL;}
	
	HRESULT	WINAPI QueryInterface(REFIID riid, void** ppv);
	
	ULONG WINAPI AddRef() {return ++m_refcount;}

	ULONG WINAPI Release()
	{
		if( --m_refcount == 0 )
			delete this;
		return m_refcount;
	}

private:
	char* m_buff;
	size_t m_size;
	size_t m_pos;
	int m_refcount;
};



#endif //defined _NSESSION_H_