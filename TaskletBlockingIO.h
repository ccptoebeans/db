#ifndef DB_TASKLETBLOCKINGIO_H
#define DB_TASKLETBLOCKINGIO_H

#include "StdAfx.h"

class IOWorker
{
public:
	IOWorker();
	~IOWorker();
	bool ExecuteAndWait();

	enum State
	{
		PENDING = 1,
		FAILED = 2,
		CANCELED = 3,
		DONE = 4
	};

	State mState;
	virtual void ThreadFunc() = 0;
	void MarkCancelled();
	void Complete();

private:
	PyChannelObject* mChannel;
};

class IOWorkerContext : public IBlueEvents
{
public:
	IOWorkerContext();
	~IOWorkerContext();
	void Init();
	void Schedule( IOWorker* request );

private:
	void OnTick( Be::Time realTime, Be::Time simTime, void* cookie ) override; // IBlueEvents::OnTick

	uv_loop_t* mLoop;
	bool mRegistered;
};

static IOWorkerContext g_taskletBlockingRequestContext;

#endif //DB_TASKLETBLOCKINGIO_H
