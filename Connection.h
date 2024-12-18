/* 
	*************************************************************************

	Connection.h

	Author:    Matthias Gudmundsson
	Created:   Jul. 2002
	OS:        Win32
	Project:   EVE Server Database Access

	Description:   

		Database connection object.


	Dependencies:

		Blue, Python

	(c) CCP 2002

	*************************************************************************
*/

#ifndef _CONNECTION_H_
#define _CONNECTION_H_

class Connection : 
	public IRoot
{
public:
	EXPOSE_TO_BLUE();

	// mSchema is a Tuple of two dicts. First dict has key as stored proc name and
	// value is list of parameter columns info.
	// Second dict has key as table name and value is list of columns info
	PyObject* mSchema;

	Connection();
	~Connection();

	PyObject* GetSchema(CSession& session, bool refresh = false);

private:
	static PyObject *EnumerateProcParams(CSession &s);
};

TYPEDEF_BLUECLASS(Connection);
	

//A class for the limited proc parameter info that we need.
#include <atldbsch.h>
#include <set>
#include <string>
class CMiniProcParamsInfo
{
public:
	CMiniProcParamsInfo(const CProcedureParameterInfo &other) :
		m_szParameterName(StringStore(other.m_szParameterName)), 
		m_nOrdinalPosition(other.m_nOrdinalPosition),
		m_nType(other.m_nType),
		m_bIsNullable(!!other.m_bIsNullable),
		m_nDataType(other.m_nDataType),
		m_nMaxLength(other.m_nMaxLength),
		m_nPrecision(other.m_nPrecision),
		m_nScale(other.m_nScale) {}
	
	const char *m_szParameterName;
	ULONG m_nMaxLength;
	USHORT m_nOrdinalPosition;
	USHORT m_nType;
	USHORT m_nDataType;
	USHORT m_nPrecision;
	SHORT m_nScale;
	bool m_bIsNullable;

	//use a static string store, to reuse the considerable amount of common parameter names
	static const char *StringStore(const char *input){
		typedef std::set<std::string> store_t;
		static store_t store;
		std::pair<store_t::iterator, bool> r = store.insert(input);
		return r.first->c_str();
	}

	static void Destructor(PyObject *obj) {
		CMiniProcParamsInfo *self = reinterpret_cast<CMiniProcParamsInfo*>(
			PyCapsule_GetPointer(obj, "CMiniProcParamsInfo"));
		delete self;
	}
};

#endif
