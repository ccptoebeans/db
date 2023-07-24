// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once
#ifndef _DB_STDAFX_H_
#define _DB_STDAFX_H_


// Insert your headers here
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#if (_MSC_VER < 1400 && !_DLL)
// Not using c++ exceptions
#define _HAS_EXCEPTIONS 0
#include <exception>
using std::exception;
#else
#define _HAS_EXCEPTIONS 1
#endif

#include <windows.h>


// TODO: reference additional headers your program requires here
#include <BlueExposure.h>
#include <IBlueOS.h>
#include <IBluePython.h>
#include <ITaskletTimer.h>

#include "msoledbsql.h"

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include <atldbcli.h>
#include <oledberr.h>

#ifdef TEST
	#include "MockAccessor.h"
#endif


#endif