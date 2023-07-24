/* 
	*************************************************************************

	TestTools.h

	Author:    James Hawk
	Created:   July. 2023

	Description:   

		Provides functions to test Input and output processing with a
		mocked db.

		Can be used to excersise all dbtypes against various python inputs.
		

	(c) CCP 2023

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

	TestTools( IRoot* lockobj = nullptr );

	PyObject* TestParameter( int db_type, PyObject* value );
	
};

TYPEDEF_BLUECLASS( TestTools );

#endif