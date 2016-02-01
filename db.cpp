//
// db.cpp
//
// Defines the entry point for the DLL application.
//
// Jörundur Sveinn Matthíasson
// (c) CCP march 2001
//


#include "StdAfx.h"
#include "Utils.h"
#include "NSession.h"

#include <blue/include/Blue.h>
#include <blue/include/Blue.cxx>

const char* g_moduleName = "db";

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
	PyObject* module = Py_InitModule("db", NULL);
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
extern "C" void __declspec(dllexport) initdb()
{
	// Init Blue related
	BlueClientStart(gInstance);
	CoInitialize(0);
}
