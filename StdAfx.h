// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#ifndef _DB_STDAFX_H_
#define _DB_STDAFX_H_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// Insert your headers here
#define WINVER 0x0500
#define _WIN32_WINNT 0x0500
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
#include "BlueExposure/include/BlueExposure.h"
#include <Blue/include/IBlueOS.h>
#include <blue/include/IBluePython.h>
#include <blue/include/ITaskletTimer.h>

//include SQL Native Client support for 2008
#define _SQLNCLI_OLEDB_
#include "sqlncli.h"

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include <atldbcli.h>
#include <oledberr.h>
#include <sqlncli.h>


#endif