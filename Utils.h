/* 
	*************************************************************************

	Utils.h

	Author:    Matthias Gudmundsson
	Created:   Jan. 2003
	OS:        Win32
	Project:   EVE Server Database Access

	Description:   

		Utility functions and stuff that doesn't fit into any other file.


	Dependencies:

		Python

	(c) CCP 2003

	*************************************************************************
*/

#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>
#include <vector>

class Session;



namespace Utilities 
{
	bool InitUtilities(PyObject* dbModuleDict);
	bool UninitUtilities();
	
	const char* DBTypeName(DBTYPE type);
	size_t GetSizeofDBType(DBTYPE bType);
	
	// Error facility
	extern PyObject* DbExc_RuntimeError;	// inherited from exceptions.RuntimeError
	extern PyObject* DbExc_WindowsError;	// inherited from exceptions.WindowsError
	extern PyObject* DbExc_SQLError;		// imported from /lib/ccp_exceptions.py
	extern PyObject* DbExc_ConnectionError;	// imported from /lib/ccp_exceptions.py

	PyObject* SetErr32(int err, const char* format, ...);
	PyObject* SetErrBlue(const char* format, ...);

	void FormatErrorRec(std::vector<std::wstring> &result, ULONG ne, const CDBErrorInfo &ei);
	PyObject* ErrorRecToPy(const CDBErrorInfo *ei, ULONG ne);
	bool ParamErrorsToPy(HRESULT hr, ATL::CDynamicParameterAccessor &accessor, PyObject **paramErrors, PyObject **columnErrors);

	bool IsSoftError(HRESULT hr);
	PyObject* ErrorClass(HRESULT hr);

}


#define CHECKERR(_hr, _why) \
	do {if (FAILED(_hr)){\
		return Utilities::SetErr32(_hr, _why);} \
	}while (0)

#define CHECKERR1(_hr, _why, _arg1) \
	do {if (FAILED(_hr)){\
		return Utilities::SetErr32(_hr, _why, _arg1);} \
	}while (0)

#define CHECKERR2(_hr, _why, _arg1, _arg2) \
	do {if (FAILED(_hr)){\
		return Utilities::SetErr32(_hr, _why, _arg1, _arg2);} \
	}while (0)

#endif

