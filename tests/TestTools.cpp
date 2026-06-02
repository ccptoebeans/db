// Copyright © 2023 CCP ehf.
#include "StdAfx.h"
#include "TestTools.h"
#include "MockSqlCommand.h"
#include "RowDescriptor.h"

TestTools::TestTools( IRoot* lockobj /*= nullptr */ )
{
	// Init utilities to initialise DbExc_RuntimeError
	Utilities::InitUtilities( nullptr );
}

PyObject* TestTools::TestParameter( int db_type, PyObject* value )
{
	// Construct context
	BluePy blue = BluePy( PyImport_Import( BluePyStr( "blue" ) ) );
	if( !blue )
		return nullptr;
	BluePy getMem = BluePy( PyImport_Import( BluePyStr( "sys" ) ) );
	if( !getMem )
		return false;
	getMem = BluePy( PyObject_GetAttrString( getMem, "getpymalloced" ) );
	if( !getMem )
		PyErr_Clear(); // not supported, continue
	ToPythonCtxt ctxt( blue, 0, 0, getMem );

	// Initialise required structures
	SimplePoolAllocator allocator;
	Row* r;
	DBLENGTH receiveLength;
	Store store(allocator);
	int numCols = 1;
	void* blob;
	size_t data_length;

	// Create MockNSession
	// Creates an empty NSession which creates a valid conversion member
	MockNSession mockNSession;

	if(!mockNSession.isValid())
	{
		PyErr_SetString( PyExc_RuntimeError, "Failed to create MockNSession" );
		return nullptr;
	}

	// Create a MockSqlCommand which will process pyobject input via MockAccessor
	// based on db_type
	MockSqlCommand test(&mockNSession);
	if(!test.SetPyParamTest( db_type, value, blob, data_length ))
	{
		return nullptr;
	}

	// Create a new MockAccessor and prep the input data processed in previous step
	MockAccessor accessor;
	accessor.SetParamType( 0, db_type );
	accessor.SetParamLength( 0, data_length );
	char* dest = (char*)accessor.GetParam( 0 );
	memcpy( dest, blob, data_length );
	free( blob );

	// Create a row descriptor for testing database retrieval
	RowDescriptor rd(&mockNSession);
	rd.Init( accessor );

	// Create Row which will retrieve out input data back from MockAccessor set previously
	Row::NewRow( &r, allocator, receiveLength, rd, accessor, numCols, store );

	// Convert Row to python, code path ends up in Blue extension ( PyRowSet.cpp )
	PyObject* retval = r->ToPython( rd, rd.ToPython( ctxt.mBlue ), ctxt );

	// Return DBRow object containing one value representing retrieved input data
	return retval;

}
