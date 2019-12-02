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


virtual const char *GetSql() {return "";}
};


class CProcParamsInfo8 : public CProcParamsInfo
{
public:
	const char *GetSql() {return Sql8;}

static const char Sql8[];
};

class CProcParamsInfo9 : public CProcParamsInfo
{
public:

	const char *GetSql() {return Sql9;}

static const char Sql9[];
};

const char CProcParamsInfo8::Sql8[] = "\
select SCHEMA_NAME = '', PROCEDURE_NAME 		= o.name,--convert(nvarchar(134),o.name +';'+ ltrim(str(c.number,5))),\n\
	PARAMETER_NAME 		= c.name,\n\
	ORDINAL_POSITION 	= convert(smallint, c.colid),\n\
	PARAMETER_TYPE 		= convert(smallint, 1+c.isoutparam),\n\
	IS_NULLABLE		= convert(bit,ColumnProperty(c.id,c.name,'AllowsNull')),\n\
	DATA_TYPE		= d.oledb_data_type,\n\
	CHARACTER_MAXIMUM_LENGTH= convert(int,\n\
					case \n\
					when d.oledb_data_type = 129 /*DBTYPE_STR*/ \n\
						or d.oledb_data_type = 128 /*DBTYPE_BYTES*/\n\
					then coalesce(d.column_size,c.length)\n\
					when d.oledb_data_type = 130 /*DBTYPE_WSTR*/\n\
					then coalesce(d.column_size,c.length/2)\n\
					else null \n\
					end),\n\
	NUMERIC_PRECISION	= convert(smallint,\n\
					case when d.oledb_data_type = 131 /*DBTYPE_NUMERIC*/ then c.prec\n\
						when (d.fixed_prec_scale =1  or d.oledb_data_type =5 or d.oledb_data_type =4)\n\
						then d.data_precision else null end),\n\
	NUMERIC_SCALE		= convert(smallint, \n\
					case when d.oledb_data_type = 131 /*DBTYPE_NUMERIC*/ then c.scale else null end)\n\
\n\
  from sysobjects o\n\
    left join syscolumns c on c.id = o.id\n\
      left join master.dbo.spt_provider_types d on c.xtype = d.ss_dtype\n\
 where o.type = 'P' and o.status > 0\n\
 order by PROCEDURE_NAME, ORDINAL_POSITION\n\
";

typedef CCommand<CAccessor<CProcParamsInfo8>, CBulkRowset> CProcParamsCommand8;


const char CProcParamsInfo9::Sql9[] = "\
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
typedef CCommand<CAccessor<CProcParamsInfo9>, CBulkRowset> CProcParamsCommand9;



//another subclass, binding much less data than the CColumnsInfo
class CColsInfo : public CColumnsInfo
{
public:
	BEGIN_COLUMN_MAP(CColsInfo)
	COLUMN_ENTRY(1, m_szTableSchema)
	COLUMN_ENTRY(2, m_szTableName)
	COLUMN_ENTRY(3, m_szColumnName)
	COLUMN_ENTRY(4, m_nOrdinalPosition)
	COLUMN_ENTRY(5, m_bIsNullable)
	COLUMN_ENTRY(6, m_nDataType)
	COLUMN_ENTRY(7, m_nMaxLength)
	END_COLUMN_MAP()

	static const char Sql[];
};

typedef CCommand<CAccessor<CColsInfo>, CBulkRowset> CColsCommand;
const char CColsInfo::Sql[] = "\
select	SCHEMA_NAME = '', TABLE_NAME		= o.name,\n\
		COLUMN_NAME		= c.name,\n\
		ORDINAL_POSITION 	= c.colid,\n\
		IS_NULLABLE		= convert(bit,ColumnProperty(c.id,c.name,'AllowsNull')),\n\
		DATA_TYPE		= d.oledb_data_type,\n\
		CHARACTER_MAXIMUM_LENGTH= convert(int,\n\
						case \n\
						when d.oledb_data_type = 129 /*DBTYPE_STR*/ \n\
							or d.oledb_data_type = 128 /*DBTYPE_BYTES*/\n\
						then coalesce(d.column_size,c.length)\n\
						when d.oledb_data_type = 130 /*DBTYPE_WSTR*/\n\
						then coalesce(d.column_size,c.length/2)\n\
						else null \n\
						end)\n\
  from syscolumns c\n\
    inner join sysobjects o on o.id = c.id\n\
      inner join master.dbo.spt_provider_types d on c.xtype = d.ss_dtype\n\
 where o.type in ('U', 'V')\n\
 order by o.name, c.colid";



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

	CCP_LOG_CH( s_chConn, "Getting DB Schema");
	PyObject *versionO = GetServerVersionString(session);
	if (!versionO) return 0;
	PySys_WriteStdout("SQL server version %s\n", PyString_AsString(versionO));
	CCP_LOG_CH( s_chConn, "SQL server version %s", PyString_AsString(versionO));

	std::string version = PyString_AsString(versionO);
	Py_DECREF(versionO);

	//two ways of getting procParams
	CCP_LOG_CH( s_chConn, "Stored proc schema ...");
	PyObject *procParams;
	if (!strncmp(version.c_str(), "8.", 2))
		//SQL server 2000
		procParams = ProcParamsNew8(session);
	else if (!strncmp(version.c_str(), "9.", 2) || !strncmp(version.c_str(), "10.", 3) ||
             !strncmp(version.c_str(), "11.", 3) || !strncmp(version.c_str(), "12.", 3) ||
             !strncmp(version.c_str(), "13.", 3) || !strncmp(version.c_str(), "14.", 3) || 
             !strncmp(version.c_str(), "15.", 3) )
		//SQL server versions:
		//9.=2005, 10.=2008 and 200R2, 11.=2012, 12.=2014, 13.=2016, 14.=2017, 15.=2019
		procParams = ProcParamsNew9(session);
	else
		//any others
		procParams = ProcParams(session);
	if (!procParams)
		return 0;
	CCP_LOG_CH( s_chConn, "... done");

	PyObject *colInfo = Py_None; Py_INCREF(colInfo);
	if (!colInfo) {
		Py_DECREF(procParams);
		return 0;
	}
	PyObject *schema = PyTuple_New(2);
	if (!schema) {
		Py_DECREF(procParams);
		Py_DECREF(colInfo);
		return 0;
	}
	PyTuple_SET_ITEM(schema, 0, procParams);
	PyTuple_SET_ITEM(schema, 1, colInfo);

	Py_XDECREF(mSchema);
	mSchema = schema;
	
	Py_INCREF(mSchema);
	return mSchema;
}


PyObject *Connection::GetServerVersionString(CSession &session)
{
	CCommand<CDynamicStringAccessorA> pp;
	CHECKERR( pp.Open(session, "SELECT SERVERPROPERTY('productversion')", 0, 0, DBGUID_SQL), "Cannot query server version");
	CHECKERR( pp.MoveFirst(), "error iteration version result");
	return PyString_FromString(pp.GetString(1));
}


PyObject *Connection::ProcParams(CSession &session)
{
	// Enumerate all stored procs parameters
	PySys_WriteStdout("DB library populating stored proc. schema");

	CProcedureParameters pp;
	CHECKERR(pp.Open(session), "Cannot open procedure parameters schema");
	
	PyObject* procParams = PyDict_New();
	if (!procParams) return 0;

	HRESULT hr;
	std::string currName;
	typedef std::map<size_t, PyObject *> currParams_t;
	currParams_t currParams;
	int count = 0;
	for (hr = pp.MoveFirst(), currName = pp.m_szName; ; hr = pp.MoveNext())
	{
		CHECKERR(hr, "Error iterating over procedure parameter schema");
		std::string rowName;
		if (hr != DB_S_ENDOFROWSET) {
			rowName = pp.m_szName;
			// Strip the silly ";1 or ;0" from the proc name
			size_t where = rowName.find(';');
			if (where >= 0)
				rowName.resize(where);
		}
		
		if (hr == DB_S_ENDOFROWSET || currName != rowName) {
			//must flush the parameters we have gathered, if any
			if (!currName.empty()) {
				const char *key = currName.c_str();
				PyObject *params = PyList_New(currParams.size());
				for(currParams_t::iterator it = currParams.begin(); it != currParams.end(); it++) {
					size_t i = it->first;
					CCP_ASSERT(i<currParams.size());
					PyList_SET_ITEM(params, i, it->second);
				}
				PyObject *keyO = PyString_InternFromString(key);
				if (!keyO) return 0;
				if (PyDict_SetItem(procParams, keyO, params)) return 0;
				Py_DECREF(keyO);
				Py_DECREF(params);
			}
			currParams.clear();
			if (hr == DB_S_ENDOFROWSET)
				break;
			currName = rowName;
		}
		
		//create parameter record
		PyObject *param = PyList_New(7);
		PyList_SET_ITEM(param, 0, PyString_InternFromString(pp.m_szParameterName));
		PyList_SET_ITEM(param, 1, PyInt_FromLong(pp.m_nType));
		PyList_SET_ITEM(param, 2, PyInt_FromLong(pp.m_bIsNullable ? 1 : 0));
		PyList_SET_ITEM(param, 3, PyInt_FromLong(pp.m_nDataType));
		PyList_SET_ITEM(param, 4, PyInt_FromLong(pp.m_nMaxLength));
		PyList_SET_ITEM(param, 5, PyInt_FromLong(pp.m_nPrecision));
		CMiniProcParamsInfo* info = new CMiniProcParamsInfo(pp);
		PyList_SET_ITEM(param, 6, PyCapsule_New(info,
			"CMiniProcParamsInfo", 
			CMiniProcParamsInfo::Destructor));


		//add to the list for this proc
		CCP_ASSERT(currParams.find(pp.m_nOrdinalPosition) == currParams.end());
		currParams.insert(currParams_t::value_type(pp.m_nOrdinalPosition, param));

		if (++count % 100 == 0)
			PySys_WriteStdout(".");
	}
	PySys_WriteStdout("done!\r\n");
	CCP_ASSERT(currParams.empty());
	return procParams;
}


PyObject *Connection::ProcParamsNew8(CSession &session)
{
	// Enumerate all stored procs parameters
	PySys_WriteStdout("DB library populating stored proc. schema");
	
	CProcParamsCommand8 pp;
	CHECKERR(pp.Open(session, pp.Sql8, 0, 0, DBGUID_SQL), "Cannot open procedure parameters schema");
	
	PyObject* procParams = PyDict_New();
	
	HRESULT hr;
	std::string currName;
	std::vector<CProcParamsInfo> currParams;
	int count = 0;
	for (hr = pp.MoveFirst(); ; hr = pp.MoveNext())
	{
		CHECKERR(hr, "Error iterating over procedure parameter schema");
		const char *rowName = pp.m_szName;
		
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
					PyList_SET_ITEM(param, 0, PyString_InternFromString(p.m_szParameterName));
					PyList_SET_ITEM(param, 1, PyInt_FromLong(p.m_nType));
					PyList_SET_ITEM(param, 2, PyInt_FromLong(p.m_bIsNullable ? 1 : 0));
					PyList_SET_ITEM(param, 3, PyInt_FromLong(p.m_nDataType));
					PyList_SET_ITEM(param, 4, PyInt_FromLong(p.m_nMaxLength));
					PyList_SET_ITEM(param, 5, PyInt_FromLong(p.m_nPrecision));
					PyList_SET_ITEM(param, 6, cobject);

					PyList_SET_ITEM(params, i, param);
				}
				PyObject *keyO = PyString_InternFromString(currParams[0].m_szName);
				if (!keyO) return 0;
				if (PyDict_SetItem(procParams, keyO, params)) return 0;
				Py_DECREF(keyO);
				Py_DECREF(params);
			}

			if (hr == DB_S_ENDOFROWSET)
				break;
			currParams.clear();
			currParams.push_back(CProcParamsInfo::Retval(rowName));
			currName = rowName;
		}
		if (pp.m_szParameterName[0])
			currParams.push_back(pp);
		if (++count % 100 == 0)
			PySys_WriteStdout(".");
	}
	PySys_WriteStdout("done!\r\n");
	return procParams;
}

//same but for sqlserver2005 and later versions.  Later we can factor common code out and change CProcParamsCommand
PyObject *Connection::ProcParamsNew9(CSession &session)
{
	// Enumerate all stored procs parameters
	PySys_WriteStdout("DB library populating stored proc. schema");
	
	CProcParamsCommand9 pp;
	CHECKERR(pp.Open(session, pp.Sql9, 0, 0, DBGUID_SQL), "Cannot open procedure parameters schema");
	
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
			PyObject* schemaO = PyString_FromString(pp.m_szSchema);
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
					PyList_SET_ITEM(param, 0, PyString_InternFromString(p.m_szParameterName));
					PyList_SET_ITEM(param, 1, PyInt_FromLong(p.m_nType));
					PyList_SET_ITEM(param, 2, PyInt_FromLong(p.m_bIsNullable ? 1 : 0));
					PyList_SET_ITEM(param, 3, PyInt_FromLong(p.m_nDataType));
					PyList_SET_ITEM(param, 4, PyInt_FromLong(p.m_nMaxLength));
					PyList_SET_ITEM(param, 5, PyInt_FromLong(p.m_nPrecision));
					PyList_SET_ITEM(param, 6, cobject);

					PyList_SET_ITEM(params, i, param);
				}
				
				PyObject *keyO = PyString_FromString(currName.c_str());
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


PyObject *Connection::TableSchema(CSession &session)
{
	// Enumerate all table columns	
	PySys_WriteStdout("DB library populating table schema");
	CColumns tc;
	CHECKERR(tc.Open(session), "Cannot open table columns schema");
	std::vector<CColumnsInfo> columns;
	
	PyObject* colInfo = PyDict_New();

	int count = 0;
	char currName[130] = "";
	HRESULT hr;
	for (hr = tc.MoveFirst(), strcpy_s(currName, tc.m_szTableName); ; hr = tc.MoveNext())
	{
		if (hr == DB_S_ENDOFROWSET)
			break;
		
		CHECKERR(hr, "Error iterating over table columns schema");

		// Got a new proc?
		if (strcmp(currName, tc.m_szTableName) != 0)
		{
			// flush what we got so far
			PyObject* list = PyList_New(columns.size());

			for (size_t i = 0; i < columns.size(); i++)
			{
				// Create parameter record
				PyObject* fields = PyList_New(8);
				PyList_SET_ITEM(fields, 0, PyString_FromString(columns[i].m_szColumnName));
				PyList_SET_ITEM(fields, 1, PyInt_FromLong(columns[i].m_bColumnHasDefault ? 1 : 0));
				PyList_SET_ITEM(fields, 2, PyString_FromString(columns[i].m_szColumnDefault));
				PyList_SET_ITEM(fields, 3, PyInt_FromLong(columns[i].m_nColumnFlags));
				PyList_SET_ITEM(fields, 4, PyInt_FromLong(columns[i].m_bIsNullable ? 1 : 0));
				PyList_SET_ITEM(fields, 5, PyInt_FromLong(columns[i].m_nDataType));
				PyList_SET_ITEM(fields, 6, PyInt_FromLong(columns[i].m_nMaxLength));
				PyList_SET_ITEM(fields, 7, PyString_FromString(columns[i].m_szDescription));
				
				// Insert column record
				CCP_ASSERT(i+1 == columns[i].m_nOrdinalPosition);
				PyList_SET_ITEM(list, i, fields);
			}

			// Strip the silly ";1" from the proc name
			char* silly = strchr(currName, ';');
			if (silly)
				*silly = '\0';

			PyDict_SetItemString(colInfo, currName, list);
			Py_DECREF(list);
			columns.clear();
			strcpy_s(currName, tc.m_szTableName);
		}
		
		if (++count % 100 == 0)
			PySys_WriteStdout(".");

		columns.push_back(tc);
	}
	PySys_WriteStdout("done!\r\n");
	return colInfo;
}


PyObject *Connection::TableSchemaNew(CSession &session)
{
	// Enumerate all table columns	
	PySys_WriteStdout("DB library populating table schema");
	CColsCommand tc;
	CHECKERR(tc.Open(session, CColsCommand::Sql, 0, 0, DBGUID_SQL), "Cannot open table columns schema");
	
	std::vector<CColumnsInfo> columns;
	PyObject* colInfo = PyDict_New();
	int count = 0;
	std::string currName;
	HRESULT hr;
	for (hr = tc.MoveFirst(), currName = tc.m_szTableName; ; hr = tc.MoveNext())
	{
		CHECKERR(hr, "Error iterating over table columns schema");
		const char *rowName = tc.m_szTableName;

		if (hr == DB_S_ENDOFROWSET || currName != rowName) {
			//must flush the parameters we have gathered, if any
			size_t s = columns.size();
			if (s) {
				// flush what we got so far
				PyObject* list = PyList_New(s);

				for (size_t i = 0; i < s; i++) {
					// Create parameter record
					PyObject* fields = PyList_New(8);
					PyList_SET_ITEM(fields, 0, PyString_FromString(columns[i].m_szColumnName));
					PyList_SET_ITEM(fields, 1, PyInt_FromLong(columns[i].m_bColumnHasDefault ? 1 : 0));
					PyList_SET_ITEM(fields, 2, PyString_FromString(columns[i].m_szColumnDefault));
					PyList_SET_ITEM(fields, 3, PyInt_FromLong(columns[i].m_nColumnFlags));
					PyList_SET_ITEM(fields, 4, PyInt_FromLong(columns[i].m_bIsNullable ? 1 : 0));
					PyList_SET_ITEM(fields, 5, PyInt_FromLong(columns[i].m_nDataType));
					PyList_SET_ITEM(fields, 6, PyInt_FromLong(columns[i].m_nMaxLength));
					PyList_SET_ITEM(fields, 7, PyString_FromString(columns[i].m_szDescription));
					
					// Insert column record
					//ASSERT(i+1 == columns[i].m_nOrdinalPosition); //fails for some tables, like agtAgents
					PyList_SET_ITEM(list, i, fields);
				}
				PyDict_SetItemString(colInfo, currName.c_str(), list);
				Py_DECREF(list);
			}
			if (hr == DB_S_ENDOFROWSET)
				break;
			columns.clear();
			currName = rowName;
		}
		columns.push_back(tc);
		if (++count % 100 == 0)
			PySys_WriteStdout(".");
	}
	PySys_WriteStdout("done!\r\n");
	return colInfo;
}