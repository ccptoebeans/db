/* 
	*************************************************************************

	SessionPool.h

	Author:    Kristjan Valur Jonsson
	Created:   sept. 2009
	OS:        Win32
	Project:   EVE Server Database Access

	Description:   

		A class that doles out DB Session objects from a pool.  Has the
		concept of maxSessions (limiting the number of sessions.)
		It will block a tasklet until a session is available if there
		is no room to generate a new one.
		It maintans a list of free sessions and has a min-max watermark
		of the level of free sessions.  It can be useful to keep a minimum
		of two idle sessions to make sure that one is always available.


	Dependencies:

		Python

	(c) CCP 2005

	*************************************************************************
*/


#ifndef _SESSIONPOOL_H_
#define _SESSIONPOOL_H_

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <stacklessio.h>
#include <atldbcli.h>


/*
 * We use a session pool to keep sessions alive.  Each session keeps a connection
 * to the database.  When we create a command, we do it on an idle session and
 * that session's connection is used.
 * If we were to create a new command on a busy session, a new temporary DB connection
 * would be created behind the scenes with some overhead.
 */
class SessionPool;
typedef boost::shared_ptr<SessionPool> SessionPoolPtr;
class SessionPool : public boost::enable_shared_from_this<SessionPool>
{
public:
	SessionPool(const ATL::CDataSource &c);

	//We have init and fini methods, that can return pythone exceptions.
	//must be called on main thread.
	bool Init();
	void Fini();
	
	// This must be called on the main thread.
	// if it returns false, a python exception has been set
	bool GetSession(ATL::CSession* &s);

	// These can bec called on a worker thread
	void ReturnSession(ATL::CSession *s);
	void DiscardSession(ATL::CSession *s);

	// Finally, this must be called on the main before returning if the
	// GetSession was successful.  It _can_ be called before
	// ReturnSession or DiscardSession has been called.
	bool EndSession();

	//Informative Python interface functions:
	PyObject *GetStatus();
	PyObject *GetSettings();
	bool ApplySettings(PyObject *settings);
	
private:
	//push and pop from the freelist, updating mListSize
	void PushList(ATL::CSession *s);
	ATL::CSession *PopList();
	void FlushList();

	//Create a new session.  mSessionCount must have been
	//incremented prior.
	HRESULT NewSession(ATL::CSession* &s);
	
	//The same, but with tasklet-blocking boilerplate using
	//StacklessIO.  This function _will_ infrement mSessionCount
	//since it is called on the main thread.
	HRESULT TaskletBlockingNewSession(ATL::CSession * &s);
	
	//Delete a session. Will decrement mSessionCount.
	void DeleteSession(ATL::CSession*);

	//Create a new session and put on the idle queue.
	//mListSize and mSessionCount must have been incremented
	//prior to calling this.
	void NewIdleSessions_thread(int n);
	
	//The boilerplate function to call on the main thread.  Fires
	//off a worker thread after incrementing mListSize and mSessionCount
	void NewIdleSessions(int n);
	
	//Internal session getter.  Tasklet blocks if none is available.
	//Then either gets a free session or creates a new one.  Will
	//also stock up the idle list if it is growing empty.
	bool GetSession_int(ATL::CSession* &s);

	//Called when EndSession() is called, will release blocked tasklets
	//as apporpriate
	bool Pump();

	//This function will fill the idle list if needed.  Takes the total
	//session count, possibly adjusted with the new session about to
	//be created.
	//if fillAll is true, will fill to the top, otherwise, just add one
	void FillIdle(int nSessions, bool fillAll=false);

	//A function to trim the idle list if needed.
	void PruneIdle(bool all=false);

	//Can we clean now?
	bool CanClean(bool mark = false);
	//mark the clean moment
	void MarkClean();

	//A IORequest to create and return a new Session
	struct Request : public IOWorker
	{
		Request(SessionPoolPtr pool);
		~Request();
		void ThreadFunc();
		HRESULT GetResult(ATL::CSession * &s);
	
		SessionPoolPtr mPool;
		HRESULT mHr;
		ATL::CSession *mSession;
	}; 
	//A simple worker thread request to create a new idle session.
	//stacklessIO doesn't have throw-away IOWorker ops.
	struct IdleRequest
	{
		IdleRequest(SessionPoolPtr pool, int n): mPool(pool), mN(n) {}
		static DWORD WINAPI ThreadProc(LPVOID arg);
		SessionPoolPtr mPool;
		int mN;
	};

	//The data in the SLIST
	struct listEntry:
		public SLIST_ENTRY,
		public ATL::CSession
	{};


private:
	const ATL::CDataSource mDataSource;
	SLIST_HEADER mList;
	LONG mListSize;		//approx size of mList (this is roughly redundant, mListSize ~ mSessionCount-mSessionsInUse)
	LONG mSessionCount; //total number of sessions
	int mSessionsInUse;	//number of sessions in use (tasklets between StartSession and EndSession)
	BluePy mChannel; //throttling channel
	ULARGE_INTEGER mNextClean; //When to next perform cleanup
	volatile bool mAddingIdle;	//used to ensure that only a single "idle" job runs at a time.
	
public:
	int mMaxSessions; //maximum number of sessions or 0 for no max
	int mMinFreeSessions; // try to have at least this number of sessions free 
	int mMaxFreeSessions; // <= mMaxSessions, don't keep more than this number of free sessions, -1 for no max.
	int mTimerDetail;
	float mCleanEvery;// how often to do cleanup (in seconds)
};

//A holder class, to make sure we don't forget to call EndSession
class SessionKeeper
{
public:
	SessionKeeper(SessionPoolPtr pool) : mGot(false), mPool(pool), mSess(0)
	{}
	~SessionKeeper() {
		DiscardSession();
		if (mGot) {
			if (!mPool->EndSession())
				PyOS->PyError();
		}
	}
	bool GetSession(ATL::CSession* &s) {
		_ASSERT(!mGot); //single use only
		bool r = mPool->GetSession(s);
		if (r) {
			mGot = true;
			mSess = s;
		}
		return r;
	}
	void ReturnSession() {
		if (mSess)
			mPool->ReturnSession(mSess);
		mSess = 0;
	}
	void DiscardSession() {
		if (mSess)
			mPool->DiscardSession(mSess);
		mSess = 0;
	}
private:
	SessionKeeper(SessionKeeper const &other);
	SessionKeeper operator = (SessionKeeper const &other);

	SessionPoolPtr mPool;
	ATL::CSession *mSess;
	bool mGot;
};

#endif // _SESSIONPOOL_H_
