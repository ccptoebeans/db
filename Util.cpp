#include "StdAfx.h"
#include <python/stackless_api.h>


//--------------------------------------------------------------------
// IsBlockTrapped - returns true if current tasklet is block-trapped.
//--------------------------------------------------------------------
bool IsBlockTrapped()
{
	PyObject* tasklet = PyStackless_GetCurrent();
	if (((PyTaskletObject*)tasklet)->flags.is_main)
		return true;
	PyObject* pytrapped = PyObject_GetAttrString(tasklet, "block_trap");
	Py_DECREF(tasklet);
	int trapped = PyObject_IsTrue(pytrapped);
	Py_DECREF(pytrapped);

	return trapped ? true : false;
}


//--------------------------------------------------------------------
// MayBlock - return true if the current tasklet is blockable
//--------------------------------------------------------------------
bool MayBlock(const char* where)
{
	PyTaskletObject* tasklet = (PyTaskletObject*)PyStackless_GetCurrent();
	PyTaskletFlagStruc* flags = 
		tasklet->flags.active ? &PyThreadState_GET()->st.flags : &tasklet->flags;

	if (flags->is_main)
	{
		PyErr_Format(JbExc_RuntimeError, "%s: Main tasklet cannot be blocked", where);
		return false;
	}
	else if (flags->block_trap)
	{
		PyErr_Format(JbExc_RuntimeError, "%s: Current tasklet is block_trap'd.", where);
		return false;
	}
	else
	{
		return true;
	}
}


//--------------------------------------------------------------------
// DBTypeName
//--------------------------------------------------------------------
const char* DBTypeName(DBTYPE type)
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
	//case DBTYPE_ARRAY:			name = "DBTYPE_ARRAY"; break;	
	//case DBTYPE_BYREF:			name = "DBTYPE_BYREF"; break;	
	case DBTYPE_I1:				name = "DBTYPE_I1"; break;
	case DBTYPE_UI2:			name = "DBTYPE_UI2"; break;
	case DBTYPE_UI4:			name = "DBTYPE_UI4"; break;
	case DBTYPE_I8:				name = "DBTYPE_I8"; break;
	case DBTYPE_UI8:			name = "DBTYPE_UI8"; break;
	case DBTYPE_GUID:			name = "DBTYPE_GUID"; break;	
	//case DBTYPE_VECTOR:			name = "DBTYPE_VECTOR"; break;	
	//case DBTYPE_RESERVED:		name = "DBTYPE_RESERVED"; break;
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
	default:					
		sprintf(retval, "Unexpected DB type %d", type);
		return retval;
	}

	strcpy(retval, name);

	if (type & DBTYPE_ARRAY)
		strcat(retval, " | DBTYPE_ARRAY");
	if (type & DBTYPE_BYREF)
		strcat(retval, " | DBTYPE_BYREF");
	if (type & DBTYPE_VECTOR)
		strcat(retval, " | DBTYPE_VECTOR");
	if (type & DBTYPE_RESERVED)
		strcat(retval, " | DBTYPE_RESERVED");

	return retval;
}


//--------------------------------------------------------------------
// GetVersionFromClsid
//--------------------------------------------------------------------

size_t GetSizeofDBType(DBTYPE bType)
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
		return sizeof(signed char);
		break;

	case DBTYPE_UI8:
		return sizeof(unsigned char);
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
// GetVersionFromClsid
//--------------------------------------------------------------------
bool GetVersionFromClsid(REFCLSID clsid, VS_FIXEDFILEINFO* verinfo)
{
	char* info = NULL;
	DWORD dummy;
	DWORD infosize;
	LPOLESTR guid;
	char key[512];
	HKEY hkey;
	DWORD valuetype = REG_SZ;
	char dllname[512];
	DWORD dllanemsize = sizeof (dllname);

	if (FAILED(StringFromCLSID(clsid, &guid)))
		goto bailout;
	
	sprintf(key, "\\CLSID\\%S\\InProcServer32", guid);
	CoTaskMemFree(guid);
	
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, key, 0, KEY_READ, &hkey) != ERROR_SUCCESS)
		goto bailout;		
	
	if (RegQueryValueEx(hkey, NULL, NULL, &valuetype, (BYTE*)dllname, &dllanemsize) != ERROR_SUCCESS)
	{
		RegCloseKey(hkey);
		goto bailout;
	}

	RegCloseKey(hkey);
	
	infosize = GetFileVersionInfoSize(
		dllname,
		&dummy
		);

	if (infosize == 0)
		goto bailout;

	info = new char[infosize];
	if (!GetFileVersionInfo(dllname, 0, infosize, info))
		goto bailout;

	UINT verlen;
	VS_FIXEDFILEINFO* ver;
	
	if (!VerQueryValue(info, "\\", (void**)&ver, &verlen))
		goto bailout;

	*verinfo = *ver;
	delete[] info;
	return true;

bailout:
	
	delete[] info;
	return false;
}

