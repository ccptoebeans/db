// Copyright © 2024 CCP ehf.
#pragma once

#include "StdAfx.h"
#include "TaskletBlockingIO.h"
#include <BluePyCpp.h>
#include <Scheduler.h>
#include <socketmodule.h>

const void* COOKIE = "DB::IOWorkerContext";

static IOWorkerContext g_taskletBlockingRequestContext;

IOWorkerContext* GetTaskletBlockingRequestContext()
{
	return &g_taskletBlockingRequestContext;
}

IOWorkerContext::IOWorkerContext() :
	mLoop( nullptr )
{
}

bool IOWorkerContext::Init()
{
	auto* socketAPI = reinterpret_cast<PySocketModule_APIObject*>( PySocketModule_ImportModuleAndAPI() );
	if ( !socketAPI )
	{
		CCP_LOGERR( "Failed acquiring carbon-io socket module" );
		return false;
	}
	mLoop = socketAPI->get_uv_loop();
	if( !mLoop )
	{
		CCP_LOGERR( "Failed to get uv_loop from carbon-io socket module" );
		return false;
	}
	return true;
}

typedef struct {
	uv_work_t work;
	std::shared_ptr<IOWorker> request;
} uv_work_req_t;

void AfterWorkCB( uv_work_t* work, int status )
{
	auto workRequest = reinterpret_cast<uv_work_req_t*>(work);
	auto request = workRequest->request;
	if( status == UV_ECANCELED )
	{
		request->MarkCancelled();
	}
	else
	{
		request->Complete();
	}
	request.reset();
	delete workRequest;
}

void WorkCB( uv_work_t* work )
{
	auto request = reinterpret_cast<uv_work_req_t*>(work)->request;
	request->ThreadFunc();
}

void IOWorkerContext::Schedule( std::shared_ptr<IOWorker> request )
{
	auto* work = new uv_work_req_t;
	work->request = request;
	uv_queue_work( mLoop, reinterpret_cast<uv_work_t*>(work), WorkCB, AfterWorkCB );
}

IOWorker::IOWorker() :
	mState( IOWorker::PENDING )
{
	mChannel = SchedulerAPI()->PyChannel_New( nullptr );
	if( !mChannel )
	{
		PyErr_WriteUnraisable( PyUnicode_FromString( "IOWorker::Request Failed to create channel" ) );
		mState = IOWorker::FAILED;
	}
	SchedulerAPI()->PyChannel_SetPreference( mChannel, 1 );
}


IOWorker::~IOWorker()
{
	Py_XDECREF( mChannel );
	mChannel = nullptr;
}

bool IOWorker::ExecuteAndWait()
{
	g_taskletBlockingRequestContext.Schedule( shared_from_this() );
	auto sentinel = SchedulerAPI()->PyChannel_Receive( mChannel );
	if( !sentinel )
	{
		return false;
	}
	else
	{
		Py_DECREF( sentinel );
	}
	return true;
}

void IOWorker::MarkCancelled()
{
	mState = IOWorker::CANCELED;
	Ccp::PyGilEnsure gil;
	if( SchedulerAPI()->PyChannel_GetBalance( mChannel ) >= 0 )
	{
		return;
	}
	if( SchedulerAPI()->PyChannel_Send( mChannel, Py_None ) == -1 )
	{
		CCP_LOGWARN( "IOWorker::MarkCancelled Failed to send sentinel" );
	}
}

void IOWorker::Complete()
{
	mState = IOWorker::DONE;
	Ccp::PyGilEnsure gil;
	if( SchedulerAPI()->PyChannel_GetBalance( mChannel ) >= 0 )
	{
		return;
	}
	if( SchedulerAPI()->PyChannel_Send( mChannel, Py_None ) == -1 )
	{
		CCP_LOGWARN( "IOWorker::Complete Failed to send sentinel" );
	}
}
