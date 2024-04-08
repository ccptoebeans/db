#ifndef DB_TASKLETBLOCKINGIO_H
#define DB_TASKLETBLOCKINGIO_H

#include "StdAfx.h"

class TaskletBlockingRequest
{
public:
	TaskletBlockingRequest();
	~TaskletBlockingRequest();
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

class RequestContext : public IBlueEvents
{
public:
	RequestContext();
	~RequestContext();
	void Init();
	void Schedule( TaskletBlockingRequest* request );

private:
	void OnTick( Be::Time realTime, Be::Time simTime, void* cookie ) override; // IBlueEvents::OnTick

	uv_loop_t* mLoop;
	bool mRegistered;
};

static RequestContext g_taskletBlockingRequestContext;

#endif //DB_TASKLETBLOCKINGIO_H
