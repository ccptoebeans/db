// Copyright © 2023 CCP ehf.
/* 
	*************************************************************************

	Row.h

	Project:   EVE Server Database Access

	Description:   

		db Row manipulation functions
		
	Dependencies:

		Python

	*************************************************************************
*/

#pragma once
#ifndef _ROW_H_
#define _ROW_H_

#include "DelayedException.h"
#include "BlockAllocator.h"
#include "RowDescriptor.h"
#include "StringStore.h"
#include "Accessor.h"

class Row
{
public:
	static DelayedException* NewRow( Row** res, SimplePoolAllocator& ba, DBLENGTH& recvLen, RowDescriptor const& d, ACCESSOR& a, int numCols, Store& store );
	static void DeleteRow( Row* r, RowDescriptor const& d );
	PyObject* ToPython( RowDescriptor const& d, PyObject* pyrd, class ToPythonCtxt& ctxt );

private:
	DelayedException* Init( DBLENGTH& recvData, RowDescriptor const& d, ACCESSOR& a, int numCols, Store& store );

	void* GetDataPtr( const RowDescriptor& d, DBORDINAL i ) const
	{
		return (void*)( (char*)&mData + d.mColumnList[i].mOffset );
	}
	int GetDataSize( const RowDescriptor& d, DBORDINAL i ) const
	{
		return d.mColumnList[i].mSize;
	}

	char* GetBitPtr( DBORDINAL& bit, const RowDescriptor& d, DBORDINAL bitoffset ) const
	{
		bit = bitoffset % 8;
		DBORDINAL byte = bitoffset / 8;
		CCP_ASSERT( (int)byte < d.mDataLen );
		return (char*)&mData + byte;
	}

	void SetBit( const RowDescriptor& d, DBORDINAL bitoffset )
	{
		DBORDINAL bit;
		char* ptr = GetBitPtr( bit, d, bitoffset );
		*ptr |= 1 << bit;
	}
	void ClrBit( const RowDescriptor& d, DBORDINAL bitoffset )
	{
		DBORDINAL bit;
		char* ptr = GetBitPtr( bit, d, bitoffset );
		*ptr &= (char)~( 1 << bit );
	}
	bool GetBit( const RowDescriptor& d, DBORDINAL bitoffset ) const
	{
		DBORDINAL bit;
		char* ptr = GetBitPtr( bit, d, bitoffset );
		return ( ( *ptr ) & ( 1 << bit ) ) != 0;
	}

	template <class T>
	void GetData( const RowDescriptor& d, T& r, DBORDINAL i ) const
	{
		ASSERT( GetDataSize( d, i ) == sizeof( T ) );
		r = *( (T*)GetDataPtr( d, i ) );
	}
	template <class T>
	void SetData( const RowDescriptor& d, const T& r, DBORDINAL i )
	{
		CCP_ASSERT( GetDataSize( d, i ) == sizeof( T ) );
		*( (T*)GetDataPtr( d, i ) ) = r;
	}

	//special object accessor
	template <class T>
	void GetObject( const RowDescriptor& d, int i, T*& o )
	{
		o = ( (T**)&mData )[d.mSObjects + i];
	}
	template <class T>
	void SetObject( const RowDescriptor& d, int i, T* o )
	{
		( (T**)&mData )[d.mSObjects + i] = o;
	}

	//specializations for bool
	template <>
	void GetData<bool>( const RowDescriptor& d, bool& r, DBORDINAL i ) const
	{
		CCP_ASSERT( d.mColumnList[i].mType == DBTYPE_BOOL && d.mColumnList[i].mSize == -1 );
		r = GetBit( d, d.mColumnList[i].mOffset );
	}

	template <>
	void SetData<bool>( const RowDescriptor& d, const bool& r, DBORDINAL i )
	{
		CCP_ASSERT( d.mColumnList[i].mType == DBTYPE_BOOL && d.mColumnList[i].mSize == -1 );
		if( r )
			SetBit( d, d.mColumnList[i].mOffset );
		else
			ClrBit( d, d.mColumnList[i].mOffset );
	}

	//Link in a warning at the head of the list.
	void Warn( DelayedException* w )
	{
		w->mNext = mWarning;
		mWarning = DelayedException_ptr( w );
	}

public:
	DelayedException_ptr mWarning; // Any warnings raised

private:
	__int64 mData;
};

#endif