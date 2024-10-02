#include "Row.h"

#include <msdadc.h>
#include "NSession.h"

void Row::DeleteRow( Row* r, RowDescriptor const& d )
{
	r->~Row();
}


DelayedException* Row::NewRow( Row** res, SimplePoolAllocator& ba, DBLENGTH& recvLen, RowDescriptor const& d, ACCESSOR& a, int numCols, StringStore& stringStore )
{
	int len = offsetof( Row, mData ) + d.mTotalLen;
	void* data = ba.malloc( len );
	*res = 0;
	if( !data )
		return DelayedException::NoMem( "couldn't allocate %d bytes for Row", len );
	Row* row = new( data ) Row();
	DelayedException* exc = row->Init( recvLen, d, a, numCols, stringStore );
	if( exc )
	{
		row->~Row();
		return exc;
	}
	*res = row;
	return 0;
}

DelayedException* Row::Init( DBLENGTH& recvLen, RowDescriptor const& d, ACCESSOR& a, int nc, StringStore& stringStore )
{
	memset( &mData, 0, d.mTotalLen );
	recvLen = 0;
	for( int index = 0; index < nc; index++ )
	{
		const DBORDINAL ordinal = a.GetOrdinal_ByIndex( index );
		CCP_ASSERT( ordinal > 0 );
		const DBORDINAL i = ordinal - 1;
		DBSTATUS status = a.GetStatus_ByIndex( index );
		if( status == DBSTATUS_S_ISNULL )
		{
			SetBit( d, d.mSNull + i );
			continue;
		}
		if( status != DBSTATUS_S_OK )
		{
			CString msg;
			if( status == DBSTATUS_S_TRUNCATED )
			{
				msg.Format( "column %d truncated.", ordinal );
				Warn( PYDBWARN( msg ) );
			}
			else
			{
				msg.Format( "Unexpected status of column %d: %d", ordinal, status );
				PYDBERROR( msg );
			}
		}
		DBTYPE dbtype = a.GetColumnType_ByIndex( index );
		if( dbtype & ( DBTYPE_ARRAY | DBTYPE_VECTOR ) )
		{
			CString msg;
			msg.Format( "Unexpected type of column %d: %d", ordinal, dbtype );
			PYDBERROR( msg );
		}

		//store stats on received data
		DBLENGTH dblen = a.GetLength_ByIndex( index );
		if( dblen != ~0 )
			recvLen += dblen;

		

		bool byref = !!( dbtype & DBTYPE_BYREF );
		switch( dbtype & 0xff )
		{
		case DBTYPE_I1:
		case DBTYPE_UI1: {
			__int8 b;
			a.GetValue_ByIndex( index, &b );
			SetData( d, b, i );
			break;
		}
		case DBTYPE_I2:
		case DBTYPE_UI2: {
			__int16 b;
			a.GetValue_ByIndex( index, &b );
			SetData( d, b, i );
			break;
		}
		case DBTYPE_UI4: {
			unsigned long b;
			a.GetValue_ByIndex( index, &b );
			SetData( d, b, i );
			break;
		}
		case DBTYPE_I4:
		case DBTYPE_R4: {
			__int32 b;
			a.GetValue_ByIndex( index, &b );
			SetData( d, b, i );
			break;
		}
		case DBTYPE_I8:
		case DBTYPE_UI8:
		case DBTYPE_R8:
		case DBTYPE_CY:
		case DBTYPE_FILETIME: {
			__int64 b;
			a.GetValue_ByIndex( index, &b );
			SetData( d, b, i );
			break;
		}
		case DBTYPE_BOOL: {
			VARIANT_BOOL b;
			a.GetValue_ByIndex( index, &b );
			bool bb = !!b;
			SetData( d, bb, i );
			break;
		}
		case DBTYPE_STR: {
			char* s = byref ? *(char**)a.GetValue( i + 1 ) : (char*)a.GetValue( i + 1 );

			//Convert string to wide string so when retrieved the data comes through to Python3 as string, not bytes.
			StringStoreElem* elem;
			DelayedException* e = stringStore.Insert( elem, s, strlen( s ), false );
			if( e )
				return e;
			SetData( d, elem, i );
			break;
		}
		case DBTYPE_WSTR:
		case DBTYPE_BSTR: {
			wchar_t* s = byref ? *(wchar_t**)a.GetValue( i + 1 ) : (wchar_t*)a.GetValue( i + 1 );
			StringStoreElem* elem;
			DelayedException* e = stringStore.Insert( elem, s, wcslen( s ));
			if( e )
				return e;
			SetData( d, elem, i );
			break;
		}
		case DBTYPE_BYTES: {
			DBLENGTH len;
			if( !a.GetLength( i + 1, &len ) )
				continue;
			char* src = (char*)a.GetValue( i + 1 );
			if( byref && src )
				src = *(char**)src;
			if( !src )
				len = 0;
			StringStoreElem* elem;
			DelayedException* e = stringStore.Insert( elem, src, len, true );
			if( e )
				return e;
			SetData( d, elem, i );
			break;
		}
		case DBTYPE_DBTIMESTAMP: //sql "datetime", "smalldatetime", "datetime2"
		case DBTYPE_DBDATE: //sql "date"
		{
			DBLENGTH len;
			if( !a.GetLength( i + 1, &len ) )
				continue;
			HRESULT hr;
			DBLENGTH dstlen;
			DBSTATUS srcstatus = status;
			DBSTATUS dststatus;
			FILETIME ftime;
			
			hr = d.mNSession->mConv->DataConvert(
				dbtype & 0xff,
				DBTYPE_FILETIME,
				len,
				&dstlen,
				a.GetValue( i + 1 ),
				&ftime,
				sizeof( ftime ),
				srcstatus,
				&dststatus,
				0,
				0,
				DBDATACONVERT_DEFAULT );
			if( FAILED( hr ) )
			{
				CString msg;
				msg.Format( "Data conversion failed on column %d(%s)", i, (const char*)CW2A( a.GetColumnName( i + 1 ) ) );
				return DelayedException::New( hr, msg );
			}
			SetData( d, ftime, i );
			break;
		}
		case DBTYPE_DBTIME2: {
			//The time.  Convert to FILETIME units (1e-7s)
			const DBTIME2& t2 = *reinterpret_cast<const DBTIME2*>( a.GetValue( i + 1 ) );
			unsigned __int64 time = 3600 * t2.hour + 60 * t2.minute + t2.second;
			time *= 10000000;
			time += t2.fraction / 100; // (ns to 1e-7s);
			SetData( d, time, i );
			break;
		}
		default: {
			CString msg;
			msg.Format( "Unexpected type of column %d(%s): %d", i, (const char*)CW2A( a.GetColumnName( i + 1 ) ), dbtype );
			PYDBERROR( msg );
		}
		}
	}
	return 0;
}


PyObject* Row::ToPython( const RowDescriptor& rd, PyObject* pyrd, ToPythonCtxt& ctxt )
{
	// Raise any warning (or exception if turns out to be one
	if( mWarning.get() )
	{
		if( !mWarning->Raise() )
			return 0;
	}

	BluePy data( PyCapsule_New( &mData, "DBRow", 0 ) );
	if( !data )
		return 0;
	//storing the method name here saves us loads of time in large rowsets
	static PyObject* method = 0;
	if( !method )
	{
		method = PyUnicode_InternFromString( "DBRow" );
		if( !method )
			return 0;
	}
	//convert any pointers to StringStore elements to the proper Python pointers.
	//this is a once only operation, and so the row cannot be converted to Python more than once.
	for( int i = 0; i < rd.mNObjects; i++ )
	{
		StringStoreElem* ptr;
		GetObject( rd, i, ptr );
		if( ptr )
		{
			PyObject* pystr = ptr->GetPython();
			SetObject( rd, i, pystr );
		}
	}

	return PyObject_CallMethodObjArgs( ctxt.mBlue, method, pyrd, data, 0 );
}