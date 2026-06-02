// Copyright © 2023 CCP ehf.

/* 
	*************************************************************************

	OurAccessor.h

	Project:   EVE Server Database Access

	Description:   

		A subclass from CDynamicAccessor that provides methods to access the structure
		linearly, not through the ordinalID, which costs mucho runtime.  Code copied from atldbcli.h
		
	Dependencies:

		Python

	*************************************************************************
*/

#pragma once
#ifndef _OURACCESSOR_H_
#define _OURACCESSOR_H_


class OurAccessor : public CDynamicParameterAccessor
{
public:
	DBORDINAL GetOrdinal_ByIndex( int index )
	{
		const int nColumn = index;
		CCP_ASSERT( nColumn < (int)GetColumnCount() );
		return m_pColumnInfo[nColumn].iOrdinal;
	}
	LPOLESTR GetColumnName_ByIndex( int index )
	{
		const int nColumn = index;
		return m_pColumnInfo[nColumn].pwszName;
	}
	DBTYPE GetColumnType_ByIndex( int index )
	{
		const int nColumn = index;
		return m_pColumnInfo[nColumn].wType;
	}
	DBSTATUS GetStatus_ByIndex( int index )
	{
		const int nColumn = index;
		DBBYTEOFFSET nOffset = (DBBYTEOFFSET)(ULONG_PTR)m_pColumnInfo[nColumn].pTypeInfo;
		IncrementAndAlignOffset( nOffset, m_pColumnInfo[nColumn].ulColumnSize, __alignof( DBLENGTH ) );
		IncrementAndAlignOffset( nOffset, sizeof( DBLENGTH ), __alignof( DBSTATUS ) );
		return *(DBSTATUS*)( m_pBuffer + nOffset );
	}
	DBLENGTH GetLength_ByIndex( int index )
	{
		const int nColumn = index;
		DBBYTEOFFSET nOffset = (DBBYTEOFFSET)(ULONG_PTR)m_pColumnInfo[nColumn].pTypeInfo;
		IncrementAndAlignOffset( nOffset, m_pColumnInfo[nColumn].ulColumnSize, __alignof( DBLENGTH ) );
		return *(DBLENGTH*)( m_pBuffer + nOffset );
	}
	template <class ctype>
	void GetValue_ByIndex( int index, ctype* pData )
	{
		_GetValue( index, pData );
	}
};

#endif
