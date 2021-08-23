#include "StdAfx.h"
#include "Utils.h"
#include <stackless_api.h>
#include <blue/include/IBlueOS.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <atlstr.h>

PyObject* Utilities::DbExc_RuntimeError = NULL;
PyObject* Utilities::DbExc_WindowsError = NULL;
PyObject* Utilities::DbExc_SQLError = NULL;
PyObject* Utilities::DbExc_ConnectionError = NULL;

static CcpLogChannel_t s_chUtils = CCP_LOG_DEFINE_CHANNEL( "Utils" );

//--------------------------------------------------------------------
// InsertNewException - helper func
//--------------------------------------------------------------------
static PyObject* InsertNewException(const char* name, PyObject* parent, PyObject* dict)
{
	PyObject* newexc = PyErr_NewException((char*)name, parent, NULL);

	if (newexc == NULL)
	{
		newexc = parent;
		Py_INCREF(newexc);
	}
	else
	{
		PyDict_SetItemString(dict, (char*)name, newexc);
	}

	return newexc;
}


//--------------------------------------------------------------------
// InitUtilities
//--------------------------------------------------------------------
bool Utilities::InitUtilities(PyObject* dict)
{	
	DbExc_RuntimeError = PyExc_RuntimeError;
	Py_INCREF(DbExc_RuntimeError);
	DbExc_WindowsError = PyExc_WindowsError;
	Py_INCREF(DbExc_WindowsError);

	// SQLError is declared in /lib/ccp_exceptions.py
	DbExc_SQLError = PyDict_GetItemString(PyEval_GetBuiltins(), "SQLError");
	if (DbExc_SQLError == NULL)
	{
		// dang it!
		DbExc_SQLError = DbExc_RuntimeError;
		CCP_LOGERR_CH( s_chUtils, "SQLError not found in Python builtins. Defaulting to RuntimeError");
	}
	Py_INCREF(DbExc_SQLError);

	// ConnectionError is declared in /lib/ccp_exceptions.py
	DbExc_ConnectionError = PyDict_GetItemString(PyEval_GetBuiltins(), "ConnectionError");
	if (DbExc_ConnectionError == NULL)
	{
		// dang it!
		DbExc_ConnectionError = DbExc_RuntimeError;
		CCP_LOGERR_CH( s_chUtils, "ConnectionError not found in Python builtins. Defaulting to RuntimeError");
	}
	Py_INCREF(DbExc_ConnectionError);

	return true;
}


//--------------------------------------------------------------------
// UninitUtilities
//--------------------------------------------------------------------
bool Utilities::UninitUtilities()
{
	Py_XDECREF(DbExc_RuntimeError);

	return true;
}


//--------------------------------------------------------------------
// DBTypeName
//--------------------------------------------------------------------
const char* Utilities::DBTypeName(DBTYPE type)
{
	const char* name;
	static char retval[200];
	
	switch(type & 0xFF)
	{
	case DBTYPE_EMPTY:			name = "DBTYPE_EMPTY"; break;
	case DBTYPE_NULL:			name = "DBTYPE_NULL"; break;
	case DBTYPE_I2:				name = "DBTYPE_I2"; break;
	case DBTYPE_I4:				name = "DBTYPE_I4"; break;
	case DBTYPE_R4:				name = "DBTYPE_R4"; break;
	case DBTYPE_R8:				name = "DBTYPE_R8"; break;
	case DBTYPE_CY:				name = "DBTYPE_CY"; break;
	case DBTYPE_DATE:			name = "DBTYPE_DATE"; break;
	case DBTYPE_BSTR:			name = "DBTYPE_BSTR"; break;
	case DBTYPE_IDISPATCH:		name = "DBTYPE_IDISPATCH"; break;
	case DBTYPE_ERROR:			name = "DBTYPE_ERROR"; break;	
	case DBTYPE_BOOL:			name = "DBTYPE_BOOL"; break;
	case DBTYPE_VARIANT:		name = "DBTYPE_VARIANT"; break;
	case DBTYPE_IUNKNOWN:		name = "DBTYPE_IUNKNOWN"; break;
	case DBTYPE_DECIMAL:		name = "DBTYPE_DECIMAL:	"; break;
	case DBTYPE_UI1:			name = "DBTYPE_UI1"; break;
	case DBTYPE_I1:				name = "DBTYPE_I1"; break;
	case DBTYPE_UI2:			name = "DBTYPE_UI2"; break;
	case DBTYPE_UI4:			name = "DBTYPE_UI4"; break;
	case DBTYPE_I8:				name = "DBTYPE_I8"; break;
	case DBTYPE_UI8:			name = "DBTYPE_UI8"; break;
	case DBTYPE_GUID:			name = "DBTYPE_GUID"; break;	
	case DBTYPE_BYTES:			name = "DBTYPE_BYTES"; break;
	case DBTYPE_STR:			name = "DBTYPE_STR"; break;
	case DBTYPE_WSTR:			name = "DBTYPE_WSTR"; break;
	case DBTYPE_NUMERIC:		name = "DBTYPE_NUMERIC"; break;
	case DBTYPE_UDT:			name = "DBTYPE_UDT"; break;	
	case DBTYPE_DBDATE:			name = "DBTYPE_DBDATE"; break;	
	case DBTYPE_DBTIME:			name = "DBTYPE_DBTIME"; break;	
	case DBTYPE_DBTIMESTAMP:	name = "DBTYPE_DBTIMESTAMP"; break;
	case DBTYPE_HCHAPTER:		name = "DBTYPE_HCHAPTER"; break;	
	case DBTYPE_FILETIME:		name = "DBTYPE_FILETIME"; break;
	case DBTYPE_PROPVARIANT:	name = "DBTYPE_PROPVARIANT"; break;
	case DBTYPE_VARNUMERIC:		name = "DBTYPE_VARNUMERIC"; break;
	case DBTYPE_DBTIME2:		name = "DBTYPE_DBTIME2"; break;
	default:					
		sprintf_s(retval, "Unexpected DB type %d", type);
		return retval;
	}

	strcpy_s(retval, name);

	if (type & DBTYPE_ARRAY)
		strcat_s(retval, " | DBTYPE_ARRAY");
	if (type & DBTYPE_BYREF)
		strcat_s(retval, " | DBTYPE_BYREF");
	if (type & DBTYPE_VECTOR)
		strcat_s(retval, " | DBTYPE_VECTOR");
	if (type & DBTYPE_RESERVED)
		strcat_s(retval, " | DBTYPE_RESERVED");

	return retval;
}


//--------------------------------------------------------------------
// GetSizeofDBType
//--------------------------------------------------------------------
size_t Utilities::GetSizeofDBType(DBTYPE bType)
{
	if( bType & DBTYPE_BYREF )
		return sizeof(void*);

	if( bType & DBTYPE_ARRAY )
		return sizeof(SAFEARRAY*);

	if( bType & DBTYPE_VECTOR )
		return sizeof(DBVECTOR);

	switch( bType )
	{
	case DBTYPE_I2:
		return sizeof(signed short);
		break;

	case DBTYPE_I4:
		return sizeof(signed int);
		break;

	case DBTYPE_R4:
		return sizeof(float);
		break;

	case DBTYPE_R8:
		return sizeof(double);
		break;

	case DBTYPE_CY:
		return sizeof(__int64);
		break;

	case DBTYPE_DATE:
		return sizeof(DATE);
		break;

	case DBTYPE_BSTR:
		return sizeof(BSTR*);
		break;

	case DBTYPE_IDISPATCH:
		return sizeof(IDispatch*);
		break;

	case DBTYPE_ERROR:
		return sizeof(SCODE);
		break;

	case DBTYPE_BOOL:
		return sizeof(VARIANT_BOOL);
		break;

	case DBTYPE_VARIANT:
		return sizeof(VARIANT);
		break;

	case DBTYPE_IUNKNOWN:
		return sizeof(IUnknown*);
		break;

	case DBTYPE_DECIMAL:
		return sizeof(DECIMAL);
		break;

	case DBTYPE_UI1:
		return sizeof(unsigned char);
		break;

	case DBTYPE_I1:
		return sizeof(signed char);
		break;

	case DBTYPE_UI2:
		return sizeof(unsigned short);
		break;

	case DBTYPE_UI4:
		return sizeof(unsigned int);
		break;

	case DBTYPE_I8:
		return sizeof(signed __int64);
		break;

	case DBTYPE_UI8:
		return sizeof(unsigned __int64);
		break;

	case DBTYPE_GUID:
		return sizeof(GUID);
		break;

	case DBTYPE_BYTES:
		return sizeof(BYTE);
		break;

	case DBTYPE_STR:
		return sizeof(char);
		break;

	case DBTYPE_WSTR:
		return sizeof(short);
		break;

	case DBTYPE_NUMERIC:
		return sizeof(DB_NUMERIC);
		break;

	case DBTYPE_DBDATE:
		return sizeof(DBDATE);
		break;

	case DBTYPE_DBTIME:
		return sizeof(DBTIME);
		break;

	case DBTYPE_DBTIMESTAMP:
		return sizeof(DBTIMESTAMP);
		break;

	default:
		return sizeof(__int64);
	}
}


//--------------------------------------------------------------------
// SetErr32
//--------------------------------------------------------------------
PyObject* Utilities::SetErr32(int err, const char* format, ...)
{
	CComPtr<IErrorInfo> errinfo;
	CComBSTR desc, src;
	
	if (!err)
		err = GetLastError();

	//retrieve DB error
	if (GetErrorInfo(0, &errinfo) == S_OK)
	{		
		errinfo->GetDescription(&desc);
		errinfo->GetSource(&src);
	}

	//compute given message
	char *msg = 0;
	if (format) {
		va_list args;
		va_start(args, format);
		size_t len = _vscprintf(format, args)+1;
		msg = new char[len];
		if (msg)
			vsprintf_s(msg, len, format, args);
		va_end(args);
	}

	format = "Msg: %s Desc: %S, Source: %S";
	size_t len2 = _scprintf( format, msg ? msg : "none", desc ? desc : CStringW( L"?" ), src ? src : CStringW( L"?" ) ) + 1;
	char *msg2 = new char[len2];
	if( msg2 )
		sprintf_s( msg2, len2, format, msg ? msg : "none", desc ? desc : CStringW( L"?" ), src ? src : CStringW( L"?" ) );
	PyErr_SetFromWindowsErrWithFilename(err,msg2?msg2:"none");

	delete[] msg2;
	delete [] msg;
	return NULL;
}


//--------------------------------------------------------------------
// SetErrBlue
//--------------------------------------------------------------------
PyObject* Utilities::SetErrBlue(const char* format, ...)
{
	char* err;
	BeOS->FormatError(&err);
	BeOS->SetError(BECLEAR);

	if (!format)
		format = "";
	va_list args;
	va_start(args, format);
	size_t len = _vscprintf(format, args)+1;
	if (err)
		len += strlen(err)+1;

	char *msg = new char[len];
	if (msg) {
		vsprintf_s(msg, len, format, args);
		if (err) {
			strcat_s(msg, len, " ");
			strcat_s(msg, len, err);
		}
	}

	CCP_FREE( err );

	PyErr_SetString(PyExc_RuntimeError, msg?msg:"?"); //we don't have PyExc_BlueError here.
	delete[] msg;
	return 0;
}


void Utilities::FormatErrorRec(std::vector<std::wstring> &result, ULONG ne, const CDBErrorInfo &ei)
{
	for(ULONG i=0; i<ne; ++i) {

		//Get source and description
		CComBSTR descr, src;
		HRESULT hr = ei.GetAllErrorInfo(i, LOCALE_SYSTEM_DEFAULT, &descr, &src);
		if (!SUCCEEDED(hr)) {
			result.push_back(L"No error info");
			continue; //no use continuin.
		}

		// Get specific MS SQL Server error info
		CComPtr<ISQLServerErrorInfo> sqlservererr;
		hr = ei.GetCustomErrorObject(i, __uuidof(sqlservererr), (IUnknown**)&sqlservererr);
		if (SUCCEEDED(hr) && sqlservererr != NULL) {
			CComHeapPtr<SSERRORINFO> ssinfo;
			CComHeapPtr<OLECHAR> sserrstr;
			hr = sqlservererr->GetErrorInfo(&ssinfo, &sserrstr);
			if (SUCCEEDED(hr)) {
				CStringW msg;
				msg.Format(L"Source: %s, SQLServerErrorInfo: \"%s\" on %s, in %s:%d. Native:%d, state:%d, class:%d",
						   src.m_str,
						   ssinfo->pwszMessage,
						   ssinfo->pwszServer,
						   ssinfo->pwszProcedure,
						   ssinfo->wLineNumber, ssinfo->lNative, ssinfo->bState, ssinfo->bClass);
				result.push_back(std::wstring(msg));
				continue;
			}
		}

		//ok, more generic then:
		CComPtr<ISQLErrorInfo> sqlerr;
		hr = ei.GetCustomErrorObject(i, __uuidof(sqlerr), (IUnknown**)&sqlerr);
		if (SUCCEEDED(hr) && sqlerr != NULL) {
			CComBSTR ansierr;
			LONG nativeerr = 0;
			hr = sqlerr->GetSQLInfo(&ansierr, &nativeerr);
			if (SUCCEEDED(hr)) {
				CStringW msg;
				msg.Format( L"Source: %s, message: \"s\", sSQLErrorInfo: \"%s\" : %d",
							src.m_str,
							descr.m_str,
							ansierr ? ansierr : CStringW( L"<none>" ),
							nativeerr );
				result.push_back(std::wstring(msg));
				continue;
			}
		}
		
		//the most generic:
		CStringW msg;
		msg.Format( L"ErrorInfo: \"%s\" from \"%s\"",
					descr ? descr : CStringW( L"<none>" ),
					src ? src : CStringW( L"<none>" ) );
		result.push_back(std::wstring(msg));
	}
}


bool Utilities::ParamErrorsToPy(HRESULT hr, ATL::CDynamicParameterAccessor &accessor, PyObject **paramErrors, PyObject **columnErrors)
{
	//get column errors
	*paramErrors = *columnErrors = 0;
	BluePyList pe(0), ce(0);
	if (!pe || !ce) return false;
	if (hr == DB_E_ERRORSOCCURRED)
	{
		// Errors occurred while getting data for all columns or parameter
		// Warning conditions did not occur for any columns or parameter
		const char* DBSTATUSTEXT[] =
		{
			"DBSTATUS_S_OK",
			"DBSTATUS_E_BADACCESSOR",
			"DBSTATUS_E_CANTCONVERTVALUE",
			"DBSTATUS_S_ISNULL",
			"DBSTATUS_S_TRUNCATED",
			"DBSTATUS_E_SIGNMISMATCH",
			"DBSTATUS_E_DATAOVERFLOW",
			"DBSTATUS_E_CANTCREATE",
			"DBSTATUS_E_UNAVAILABLE",
			"DBSTATUS_E_PERMISSIONDENIED",
			"DBSTATUS_E_INTEGRITYVIOLATION",
			"DBSTATUS_E_SCHEMAVIOLATION",
			"DBSTATUS_E_BADSTATUS",
			"DBSTATUS_S_DEFAULT",
		};

		ULONG i;
		for (i = 1; i < accessor.GetParamCount(); i++)
		{
			DBSTATUS status;
			if (accessor.GetParamStatus(i, &status) && status != DBSTATUS_S_OK && status != DBSTATUS_E_UNAVAILABLE)
			{
				DBTYPE type = 0;
				accessor.GetParamType(i+1, &type); //weird how some functions have other param indices
				BluePy v(Py_BuildValue(
					"iuisis",
					i, accessor.GetParamName(i+1), status, DBSTATUSTEXT[status], type, Utilities::DBTypeName(type)
					));
				if (!v || !pe.Append(v))
					return false;
			}
		}

		for (i = 0; i < accessor.GetColumnCount(); i++)
		{
			DBSTATUS status;
			if (accessor.GetStatus(i, &status) && status != DBSTATUS_S_OK && status != DBSTATUS_E_UNAVAILABLE)
			{
				BluePy v(Py_BuildValue(
					"iuis",
					i, accessor.GetColumnName(i), status, DBSTATUSTEXT[status]
					));
				if (!v || !ce.Append(v))
					return false;
			}
		}
	}
	*paramErrors = pe.Detach();
	*columnErrors = ce.Detach();
	return true;
}

PyObject *Utilities::ErrorRecToPy(const CDBErrorInfo *errorInfo, ULONG nErrors)
{
	BluePyList errorRecords(0);
	for (ULONG i = 0; errorInfo != 0 && i < nErrors; i++)
	{
		LCID lcid = GetSystemDefaultLCID();
		CComBSTR desc;
		CComBSTR source;

		// Get error record
		HRESULT hr = errorInfo->GetAllErrorInfo(i, lcid, &desc, &source);
		if (FAILED(hr))
			continue;
		
		// Get specific MS SQL Server error info
		CComPtr<ISQLServerErrorInfo> sqlservererr;
		CComHeapPtr<SSERRORINFO> ssinfo;
		CComHeapPtr<OLECHAR> sserrstr;
		hr = errorInfo->GetCustomErrorObject(i, __uuidof(sqlservererr), (IUnknown**)&sqlservererr);
		if (SUCCEEDED(hr) && sqlservererr != NULL)
			hr = sqlservererr->GetErrorInfo(&ssinfo, &sserrstr);
		
		BluePy rec;
		if (ssinfo) {
			rec = BluePy(Py_BuildValue("u ul u uk BB",
				source.m_str,
				ssinfo->pwszMessage, ssinfo->lNative,
				ssinfo->pwszServer,
				ssinfo->pwszProcedure, ssinfo->wLineNumber,
				ssinfo->bState, ssinfo->bClass));
		} else {
			rec = BluePy(Py_BuildValue("uu", 
				source.m_str, desc.m_str));
		}
		errorRecords.Append(rec);
	}
	return errorRecords.Detach();
}


// The list of "soft" SQL errors
bool Utilities::IsSoftError(HRESULT hr)
{
	assert(FAILED(hr));
        // if this is an ITF error we know it's soft
	HRESULT facility = HRESULT_FACILITY(hr);
	if (facility == FACILITY_ITF) {
		return true;
	}

	// Nonni 06-27-01: this should not be necessary since we've already checked for ITF
	// but better safe than sorry. Refactor this out later after we've confirmed
	// clean logs for a few months
	const HRESULT ok[] = {
		DB_E_ERRORSINCOMMAND, //primarily, this is user exceptions
		DB_SEC_E_PERMISSIONDENIED,
		DB_E_PARAMNOTOPTIONAL,
		DB_E_CANCELED
	};
	
	for(size_t i=0; i<_countof(ok); ++i) {
		if (hr == ok[i]) {
			CCP_LOGWARN_CH( s_chUtils, "ITF check did not classify a soft error properly: %d", hr);
			return true;
		}
	}

	return false;
}

PyObject *Utilities::ErrorClass(HRESULT hr)
{
	if (IsSoftError(hr))
		return Utilities::DbExc_SQLError;
	return Utilities::DbExc_ConnectionError;
}