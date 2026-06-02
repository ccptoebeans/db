// Copyright © 2001 CCP ehf.
// Defines the entry point for the DLL application.

#include "StdAfx.h"
#include "Utils.h"
#include "NSession.h"

#include <Blue.h>

const char* g_moduleName = "_db";

// reduce CRT link 
extern "C" void _setargv(){}
extern "C" void _setenvp(){}


//--------------------------------------------------------------------
// BlueClientStart
//--------------------------------------------------------------------
static struct PyModuleDef ModuleDef = {
	PyModuleDef_HEAD_INIT,
	CCP_STRINGIZE( CCP_CONCATENATE( _db, CCP_BUILD_FLAVOR ) ),
	"",
	-1,
	NULL
};

PyObject* BlueClientStart( HINSTANCE instance )
{
	CCP_LOG( "DB Lib starting" );
	
	// Init Python related
	PyObject* module = PyModule_Create( &ModuleDef );
	PyObject* dict = PyModule_GetDict(module);
	Utilities::InitUtilities(dict);

#define INSERT(name, object) \
    if (PyDict_SetItemString(dict, name, (PyObject*)object) < 0)\
        return nullptr;

	INSERT("NSession", NSession::GetType());
	return module;
}

static HINSTANCE gInstance = NULL;

BOOL APIENTRY DllMain(HINSTANCE instance, DWORD  reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		gInstance = instance;
		BeClasses->RegisterClasses( BlueRegistration::GetClassRegs() );
	}
	else if (reason == DLL_PROCESS_DETACH)
	{
		BeClasses->UnregisterClasses( BlueRegistration::GetClassRegs() );
	}
	else if (reason == DLL_THREAD_ATTACH)
	{
	}

    return TRUE;
}


//--------------------------------------------------------------------
// initdb - python dll module entry function
//--------------------------------------------------------------------
extern "C" __declspec( dllexport ) PyObject* 
CCP_CONCATENATE( PyInit__db, CCP_BUILD_FLAVOR )()
{
	auto* context = GetTaskletBlockingRequestContext();
	if( !context->Init() )
	{
		return nullptr;
	}
	// Init Blue related
	PyObject* module = BlueClientStart( gInstance );
	CoInitialize(0);
	return module;
}
