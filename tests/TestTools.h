// Copyright © 2023 CCP ehf.
/* 
	*************************************************************************

	TestTools.h

	Description:   

		Provides functions to test Input and output processing with a
		mocked db.

		Can be used to excersise all dbtypes against various python inputs.

	*************************************************************************
*/

#pragma once
#ifndef TestTools_h
#define TestTools_h

#include "BlueExposure.h"
#include "msoledbsql.h"
#include "MockNSession.h"

BLUE_DECLARE( TestTools );

BLUE_CLASS( TestTools ) : public IRoot
{
public:
	EXPOSE_TO_BLUE();

	enum DbType
	{
		BOOL = DBTYPE_BOOL,
		I1 = DBTYPE_I1,
		UI1 = DBTYPE_UI1,
		I2 = DBTYPE_I2,
		UI2 = DBTYPE_UI2,
		I4 = DBTYPE_I4,
		UI4 = DBTYPE_UI4,
		I8 = DBTYPE_I8,
		FILETIME = DBTYPE_FILETIME,
		UI8 = DBTYPE_UI8,
		R4 = DBTYPE_R4,
		R8 = DBTYPE_R8,
		CY = DBTYPE_CY,
		STR = DBTYPE_STR,
		BSTR = DBTYPE_BSTR,
		WSTR = DBTYPE_WSTR,
		BYTES = DBTYPE_BYTES,
		IUNKNOWN = DBTYPE_IUNKNOWN,
		DBTIMESTAMP = DBTYPE_DBTIMESTAMP,
		DBDATE = DBTYPE_DBDATE,
		DBTIME2 = DBTYPE_DBTIME2,
		INVALID = -1

	};

	TestTools( IRoot* lockobj = nullptr );

	PyObject* TestParameter( int db_type, PyObject* value );
	
};

TYPEDEF_BLUECLASS( TestTools );

#endif