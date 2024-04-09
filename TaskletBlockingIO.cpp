#pragma once

#include "StdAfx.h"
#include "TaskletBlockingIO.h"
#include <BluePyCpp.h>

const void* COOKIE = "DB::IOWorkerContext";

IOWorkerContext::IOWorkerContext() :
	mRegistered( false ), mLoop( nullptr )
{
}

IOWorkerContext::~IOWorkerContext()
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

void IOWorkerContext::Init()
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
	auto* request = static_cast<IOWorker*>( work->data );
	if( status == UV_ECANCELED )
	{
		request->MarkCancelled();
	}
	request->Complete();
}

void WorkCB( uv_work_t* work )
{
	auto* request = static_cast<IOWorker*>( work->data );
	request->ThreadFunc();
}

void IOWorkerContext::Schedule( IOWorker* request )
{
	if( !mLoop )
	{
		Init();
	}
	auto* work = new uv_work_t;
	work->data = request;
	uv_queue_work( mLoop, work, WorkCB, AfterWorkCB );
}

void IOWorkerContext::OnTick( Be::Time realTime, Be::Time simTime, void* cookie )
{
	int result = uv_run( mLoop, UV_RUN_NOWAIT );
	if( result < 0 )
	{
		CCP_LOGERR( "IOWorker::OnTick Error ticking UV loop %d", result );
	}
}

IOWorker::IOWorker() :
	mState( IOWorker::PENDING )
{
	mChannel = PyChannel_New( nullptr );
	if( !mChannel )
	{
		PyErr_WriteUnraisable( PyUnicode_FromString( "IOWorker::Request Failed to create channel" ) );
		mState = IOWorker::FAILED;
	}
	PyChannel_SetPreference( mChannel, 1 );
}


IOWorker::~IOWorker()
{
	Py_XDECREF( mChannel );
	mChannel = nullptr;
}

bool IOWorker::ExecuteAndWait()
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

void IOWorker::MarkCancelled()
{
	mState = IOWorker::CANCELED;
	Ccp::PyGilEnsure gil;
	if( PyChannel_Send( mChannel, Py_None ) == -1 )
	{
		CCP_LOGWARN( "IOWorker::MarkCancelled Failed to send sentinel" );
	}
}

void IOWorker::Complete()
{
	mState = IOWorker::DONE;
	Ccp::PyGilEnsure gil;
	if( PyChannel_Send( mChannel, Py_None ) == -1 )
	{
		CCP_LOGWARN( "IOWorker::Complete Failed to send sentinel" );
	}
}
