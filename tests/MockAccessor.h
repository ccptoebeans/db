// Copyright © 2023 CCP ehf.
/* 
	*************************************************************************

	MockAccessor.h

	Description:   

		Overrides database access functions so they can be mocked
		Designed to work using single value for simplicity.

	*************************************************************************
*/

#pragma once
#ifndef _MOCKOURACCESSOR_H_
#define _MOCKOURACCESSOR_H_

#include "TmpRowset.h"

#include <map>

class MockAccessor : public OurAccessor
{
public:

	MockAccessor();

	~MockAccessor();

	DBORDINAL GetOrdinal_ByIndex( int index )
	{
		return 1;
	}

	LPOLESTR GetColumnName_ByIndex( int index )
	{
		return (LPOLESTR)s_dummy_column_name;
	}

	DBTYPE GetColumnType_ByIndex( int index )
	{
		return m_data_type;
	}

	DBSTATUS GetStatus_ByIndex( int index )
	{
		return DBSTATUS_S_OK;
	}

	DBLENGTH GetLength_ByIndex( int index )
	{
		return m_data_length;
	}

	bool GetLength(
		DBORDINAL nColumn,
		DBLENGTH* pLength ) const
	{
		*pLength = m_param_length;

		return true;
	}

	void* GetValue( _In_ DBORDINAL nColumn ) const
	{
		return m_dummy_buffer;
	}

	template <class ctype>
	void GetValue_ByIndex( int index, ctype* pData )
	{
		memcpy( pData, m_dummy_buffer, sizeof( ctype ) );
	}

	template <class ctype>
	bool GetParam(
		DBORDINAL nParam,
		ctype* pData ) const
	{
		return false;
	}

	virtual void* GetParam( DBORDINAL nParam )
	{
		return m_dummy_buffer;
	};

	virtual bool GetParamType(
		DBORDINAL nParam,
		DBTYPE* pType )
	{
		*pType = m_data_type;
		return true;
	};

	virtual bool SetParamLength(
		DBORDINAL nParam,
		DBLENGTH length )
	{
		m_param_length = length;
		return false;
	};

	virtual bool SetParamStatus(
		DBORDINAL nParam,
		DBSTATUS status )
	{
		return false;
	};

	DBSTATUS* GetParamStatus( DBORDINAL nParam )
	{
		return &m_param_status;
	}
	
	template <class ctype>
	void SetParam( DBORDINAL nparam, ctype* p )
	{
		memcpy( m_dummy_buffer, p, sizeof(ctype) );
	}

	virtual LPOLESTR GetParamName( DBORDINAL nParam )
	{
		return (wchar_t*)s_dummy_column_name;
	};

	virtual bool SetParamString(
		DBORDINAL nParam,
		const CHAR* pString,
		DBSTATUS status = DBSTATUS_S_OK )
	{
		size_t size = strlen( pString ) * sizeof( CHAR );
		memcpy( m_dummy_buffer, pString, size);
		return true;
	};

	virtual bool SetParamString(
		DBORDINAL nParam,
		const WCHAR* pString,
		DBSTATUS status = DBSTATUS_S_OK )
	{
		size_t size = wcslen( pString ) * sizeof( WCHAR );
		memcpy( m_dummy_buffer, pString, size );
		return true;
	};

	virtual bool GetParamSize(
		DBORDINAL nParam,
		DBLENGTH* pLength ) const
	{
		*pLength = m_buffer_length;
		return true;
	};

	DBORDINAL GetColumnCount() const
	{
		return 1;
	}

	LPOLESTR GetColumnName( _In_ DBORDINAL nColumn ) const
	{
		return (wchar_t*)s_dummy_column_name;
	}

	bool GetColumnType(
		DBORDINAL nColumn,
		DBTYPE* pType ) const throw()
	{
		*pType = m_data_type;
		return true;
	}

	void SetParamType(
		DBORDINAL nParam,
		DBTYPE pType )
	{
		m_data_type = pType;
	};

private:
	DBTYPE m_data_type;
	DBLENGTH m_data_length;
	static const wchar_t* s_dummy_column_name;
	unsigned long long m_buffer_length;
	char* m_dummy_buffer;
	unsigned long long m_param_length;
	DBSTATUS m_param_status;
};

#endif