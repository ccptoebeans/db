#pragma once

#include "StdAfx.h"
#include "TaskletBlockingIO.h"
#include <BluePyCpp.h>

const void* COOKIE = "DB::RequestContext";

RequestContext::RequestContext() :
	mRegistered( false ), mLoop( nullptr )
{
}

RequestContext::~RequestContext()
{
	if( mRegistered )
	{
		BeOS->UnregisterForTicks( this, const_cast<void*>( COOKIE ) );
		mRegistered = false;
	}
	if( mLoop )
	{
		uv_loop_close( mLoop );
		mLoop = nullptr;
	}
}

void RequestContext::Init()
{
	if( !mLoop )
	{
		mLoop = new uv_loop_t;
		uv_loop_init( mLoop );
	}
	if( !mRegistered )
	{
		BeOS->RegisterForTicks( this, const_cast<void*>( COOKIE ) );
		mRegistered = true;
	}
}

void AfterWorkCB( uv_work_t* work, int status )
{
	auto* request = static_cast<TaskletBlockingRequest*>( work->data );
	if( status == UV_ECANCELED )
	{
		request->MarkCancelled();
	}
	request->Complete();
}

void WorkCB( uv_work_t* work )
{
	auto* request = static_cast<TaskletBlockingRequest*>( work->data );
	request->ThreadFunc();
}

void RequestContext::Schedule( TaskletBlockingRequest* request )
{
	if( !mLoop )
	{
		Init();
	}
	auto* work = new uv_work_t;
	work->data = request;
	uv_queue_work( mLoop, work, WorkCB, AfterWorkCB );
}

void RequestContext::OnTick( Be::Time realTime, Be::Time simTime, void* cookie )
{
	int result = uv_run( mLoop, UV_RUN_NOWAIT );
	if( result < 0 )
	{
		CCP_LOGERR( "TaskletBlockingRequest::OnTick Error ticking UV loop %d", result );
	}
}

TaskletBlockingRequest::TaskletBlockingRequest() :
	mState( TaskletBlockingRequest::PENDING )
{
	mChannel = PyChannel_New( nullptr );
	if( !mChannel )
	{
		PyErr_WriteUnraisable( PyUnicode_FromString( "TaskletBlockingRequest::Request Failed to create channel" ) );
		mState = TaskletBlockingRequest::FAILED;
	}
	PyChannel_SetPreference( mChannel, 1 );
}


TaskletBlockingRequest::~TaskletBlockingRequest()
{
	Py_XDECREF( mChannel );
	mChannel = nullptr;
}

bool TaskletBlockingRequest::ExecuteAndWait()
{
	g_taskletBlockingRequestContext.Schedule( this );
	auto sentinel = PyChannel_Receive( mChannel );
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

void TaskletBlockingRequest::MarkCancelled()
{
	mState = TaskletBlockingRequest::CANCELED;
	Ccp::PyGilEnsure gil;
	if( PyChannel_Send( mChannel, Py_None ) == -1 )
	{
		CCP_LOGWARN( "TaskletBlockingRequest::MarkCancelled Failed to send sentinel" );
	}
}

void TaskletBlockingRequest::Complete()
{
	mState = TaskletBlockingRequest::DONE;
	Ccp::PyGilEnsure gil;
	if( PyChannel_Send( mChannel, Py_None ) == -1 )
	{
		CCP_LOGWARN( "TaskletBlockingRequest::Complete Failed to send sentinel" );
	}
}
