//
// db.cpp
//
// Defines the entry point for the DLL application.
//
// J�rundur Sveinn Matth�asson
// (c) CCP march 2001
//


#include "StdAfx.h"
#include "Utils.h"
#include "NSession.h"

#include <blue/include/Blue.h>

const char* g_moduleName = "_db";

// reduce CRT link 
extern "C" void _setargv(){}
extern "C" void _setenvp(){}


//--------------------------------------------------------------------
// BlueClientStart
//--------------------------------------------------------------------
void BlueClientStart(HINSTANCE instance)
{
	CCP_LOG( "DB Lib starting" );
	
	// Init Python related
	PyObject* module = Py_InitModule( CCP_STRINGIZE( CCP_CONCATENATE( _db , CCP_BUILD_FLAVOR ) ), NULL );
	PyObject* dict = PyModule_GetDict(module);
	Utilities::InitUtilities(dict);

#define INSERT(name, object) \
    if (PyDict_SetItemString(dict, name, (PyObject*)object) < 0)\
        return

	INSERT("NSession", NSession::GetType());
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
extern "C" void __declspec(dllexport)
CCP_CONCATENATE( init_db, CCP_BUILD_FLAVOR )()
{
	// Init Blue related
	BlueClientStart(gInstance);
	CoInitialize(0);
}
