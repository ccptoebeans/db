#ifndef DB_TASKLETBLOCKINGIO_H
#define DB_TASKLETBLOCKINGIO_H

#include "StdAfx.h"

struct PyChannelObject; // Forward declare the PyChannleObject from <Scheduler.h>

class IOWorker : public std::enable_shared_from_this<IOWorker>
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
	bool Init();
	void Schedule( std::shared_ptr<IOWorker> request );

private:
	void OnTick( Be::Time realTime, Be::Time simTime, void* cookie ) override; // IBlueEvents::OnTick

	uv_loop_t* mLoop;
	bool mRegistered;
};

IOWorkerContext* GetTaskletBlockingRequestContext();

#endif //DB_TASKLETBLOCKINGIO_H
