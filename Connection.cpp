#include "StdAfx.h"
#include "Connection.h"
#include "Utils.h"
#include <atldbsch.h>
#include <vector>
#include <map>
#include <string>

static CcpLogChannel_t s_chConn = CCP_LOG_DEFINE_CHANNEL( "Connection" );




class CProcParamsInfo : public CProcedureParameterInfo
{
public:
	// data elements inherited from parent
BEGIN_COLUMN_MAP(CProcParamsInfo)
	COLUMN_ENTRY(1, m_szSchema)
	COLUMN_ENTRY(2, m_szName)
	COLUMN_ENTRY(3, m_szParameterName)
	COLUMN_ENTRY(4, m_nOrdinalPosition)
	COLUMN_ENTRY(5, m_nType)
	COLUMN_ENTRY(6, m_bIsNullable)
	COLUMN_ENTRY(7, m_nDataType)
	COLUMN_ENTRY(8, m_nMaxLength)
	COLUMN_ENTRY(9, m_nPrecision)
	COLUMN_ENTRY(10, m_nScale)
END_COLUMN_MAP()

static CProcParamsInfo Retval(const char *procName)
{
	//set up a default return value (always an int)
	CProcParamsInfo r;
	strcpy_s(r.m_szName, procName);
	strcpy_s(r.m_szParameterName, "@RETURN_VALUE");
	r.m_nOrdinalPosition = 0;
	r.m_nType = 4;
	r.m_nDataType = 3; //int
	r.m_nPrecision = 10;
	return r;
}

static const char Sql[];
};


const char CProcParamsInfo::Sql[] = "\
SELECT SCHEMA_NAME = s.[name],\n\
       PROCEDURE_NAME = pr.name,\n\
       PARAMETER_NAME = pa.name,\n\
       ORDINAL_POSITION = CONVERT(smallint, pa.parameter_id),\n\
       PARAMETER_TYPE = CONVERT(smallint, 1 + pa.is_output),\n\
       IS_NULLABLE = CONVERT(bit, 1),\n\
       DATA_TYPE = CASE t.name\n\
         WHEN 'char' THEN 129 WHEN 'varchar' THEN 129 WHEN 'text' THEN 129\n\
         WHEN 'smalldatetime' THEN 135 WHEN 'datetime' THEN 135\n\
         WHEN 'real' THEN 4 WHEN 'float' THEN 5 WHEN 'money' THEN 6\n\
         WHEN 'image' THEN 128 WHEN 'varbinary' THEN 128\n\
         WHEN 'bit' THEN 11 WHEN 'tinyint' THEN 17 WHEN 'smallint' THEN 2 WHEN 'int' THEN 3 WHEN 'bigint' THEN 20\n\
         WHEN 'nchar' THEN 130 WHEN 'nvarchar' THEN 130 WHEN 'ntext' THEN 130\n\
         WHEN 'date' THEN 133 WHEN 'time' THEN 145 when 'datetime2' THEN 135\n\
         ELSE NULL END,\n\
       CHARACTER_MAXIMUM_LENGTH = CONVERT(int, CASE\n\
         WHEN t.name IN ('char', 'nchar') THEN CONVERT(int, pa.max_length)\n\
         WHEN t.name = 'varchar' THEN\n\
           CASE WHEN pa.max_length = -1 THEN 2147483647 ELSE CONVERT(int, pa.max_length) END\n\
         WHEN t.name = 'nvarchar' THEN\n\
           CASE WHEN pa.max_length = -1 THEN 1073741823 ELSE CONVERT(int, pa.max_length) END\n\
         WHEN t.name = 'ntext' THEN 1073741823\n\
         WHEN t.name IN ('text', 'image') THEN 2147483647\n\
		 WHEN t.name = 'varbinary' THEN\n\
           CASE WHEN pa.max_length = -1 THEN 2147483647 ELSE CONVERT(int, pa.max_length) END\n\
         ELSE NULL END),\n\
       NUMERIC_PRECISION = CONVERT(smallint, pa.precision),\n\
       NUMERIC_SCALE = CONVERT(smallint, NULL)\n\
    FROM sys.procedures pr\n\
	  INNER JOIN sys.schemas s ON s.schema_id = pr.schema_id\n\
      LEFT JOIN sys.parameters pa ON pa.object_id = pr.object_id\n\
        LEFT JOIN sys.types t ON t.user_type_id = pa.user_type_id\n\
  WHERE pr.is_ms_shipped = 0\n\
  ORDER BY s.[name], pr.[name], pa.parameter_id\n\
 ";
typedef CCommand<CAccessor<CProcParamsInfo>, CBulkRowset> CProcParamsCommand;


//////////////////////////////////////////////////////////////////////
//
// Public member functions
//
//////////////////////////////////////////////////////////////////////


//--------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------

Connection::Connection()
{
	mSchema = NULL;
}


//--------------------------------------------------------------------
// Destructor
//--------------------------------------------------------------------

Connection::~Connection()
{
	Py_XDECREF(mSchema);
}


//--------------------------------------------------------------------
// GetSchema
//--------------------------------------------------------------------
PyObject* Connection::GetSchema(CSession& session, bool refresh)
{
	if (mSchema && !refresh)
	{
		Py_INCREF(mSchema);
		return mSchema;
	}

	CCP_LOG_CH( s_chConn, "Checking SQL Server version");
	CCommand<CDynamicStringAccessorA> pp;
	CHECKERR(pp.Open(session, "SELECT SERVERPROPERTY('productversion')", 0, 0, DBGUID_SQL), "Cannot query server version");
	CHECKERR(pp.MoveFirst(), "error iteration version result");
	std::string version(pp.GetString(1));

	PySys_WriteStdout("SQL server version %s\n", version.c_str());
	CCP_LOG_CH( s_chConn, "SQL server version %s", version.c_str());

	// SQL server versions for reference:
	// 9.=2005, 10.=2008 and 200R2, 11.=2012, 12.=2014, 13.=2016, 14.=2017, 15.=2019, 16.=2022, 17.=2025
	// This test should never exceed three versions, e.g. only test for the "last", "current" and "next" one.
	if (strncmp(version.c_str(), "15.", 3) && strncmp(version.c_str(), "16.", 3) && strncmp(version.c_str(), "17.", 3)) {
		std::string err("Invalid SQL Server version: ");
		err += version;
		PyErr_SetString(PyExc_RuntimeError, err.c_str());
		return nullptr;
	}

	CCP_LOG_CH(s_chConn, "Stored proc schema ...");
	PyObject* procParams = EnumerateProcParams(session);
	if (!procParams) {
		return nullptr;
	}
	CCP_LOG_CH( s_chConn, "... done");

	PyObject *schema = PyTuple_New(2);
	if (!schema) {
		Py_DECREF(procParams);
		return nullptr;
	}

	PyObject *colInfo = Py_None;
	Py_INCREF(colInfo);

	PyTuple_SET_ITEM(schema, 0, procParams);
	PyTuple_SET_ITEM(schema, 1, colInfo);

	Py_XDECREF(mSchema);
	mSchema = schema;
	
	Py_INCREF(mSchema);
	return mSchema;
}


PyObject *Connection::EnumerateProcParams(CSession &session)
{
	// Enumerate all stored procs parameters
	PySys_WriteStdout("DB library populating stored proc. schema");
	
	CProcParamsCommand pp;
	CHECKERR(pp.Open(session, pp.Sql, 0, 0, DBGUID_SQL), "Cannot open procedure parameters schema");
	
	PyObject* procParams = PyDict_New();
	PyObject* schemas = PyDict_New();
	
	HRESULT hr;
	std::string currName("");
	std::string currSchema("");
	std::vector<CProcParamsInfo> currParams;
	int count = 0;
	for (hr = pp.MoveFirst(); ; hr = pp.MoveNext())
	{
		CHECKERR(hr, "Error iterating over procedure parameter schema");

		std::string rowName = pp.m_szSchema + std::string(".") + std::string(pp.m_szName);
		std::string schemaName = pp.m_szSchema;

		if (schemaName.length() && currSchema != schemaName) {
			PyObject* schemaO = PyUnicode_FromString( pp.m_szSchema );
			PyDict_SetItem(schemas, schemaO, Py_None);
			Py_DECREF(schemaO);
			currSchema = schemaName;
		}

		if (hr == DB_S_ENDOFROWSET || currName != rowName) {
			//must flush the parameters we have gathered, if any
			size_t s = currParams.size();
			if (s) {
				PyObject *params = PyList_New(s);
				for(size_t i = 0; i<s; i++) {
					const CProcParamsInfo &p = currParams[i];
					CMiniProcParamsInfo* info = new CMiniProcParamsInfo(p);
					PyObject *cobject = PyCapsule_New(info, "CMiniProcParamsInfo", 
						CMiniProcParamsInfo::Destructor);

					//TODO: Get rid of this python gunk and query it on demand.
					PyObject *param = PyList_New(7);
					PyList_SET_ITEM( param, 0, PyUnicode_InternFromString( p.m_szParameterName ) );
					PyList_SET_ITEM( param, 1, PyLong_FromLong( p.m_nType ) );
					PyList_SET_ITEM( param, 2, PyLong_FromLong( p.m_bIsNullable ? 1 : 0 ) );
					PyList_SET_ITEM( param, 3, PyLong_FromLong( p.m_nDataType ) );
					PyList_SET_ITEM( param, 4, PyLong_FromLong( p.m_nMaxLength ) );
					PyList_SET_ITEM( param, 5, PyLong_FromLong( p.m_nPrecision ) );
					PyList_SET_ITEM(param, 6, cobject);

					PyList_SET_ITEM(params, i, param);
				}
				PyObject* keyO = PyUnicode_FromString( currName.c_str() );
				if (!keyO) return 0;
				if (PyDict_SetItem(procParams, keyO, params)) return 0;
				Py_DECREF(keyO);
				Py_DECREF(params);
			}

			if (hr == DB_S_ENDOFROWSET)
				break;
			currParams.clear();
			currParams.push_back(CProcParamsInfo::Retval(rowName.c_str()));
			currName = rowName;
		}
		if (pp.m_szParameterName[0])
		{
			currParams.push_back(pp);
		}
		if (++count % 100 == 0)
			PySys_WriteStdout(".");
	}
	PyDict_SetItemString(procParams, "__schemas", schemas);
	Py_DECREF(schemas);
	PySys_WriteStdout("done!\r\n");
	return procParams;
}
