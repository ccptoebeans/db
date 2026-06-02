// Copyright © 2012 CCP ehf.

#include "StdAfx.h"
#include "Connection.h"

BLUE_DEFINE( Connection );

const Be::ClassInfo* Connection::ExposeToBlue()
{
	EXPOSURE_BEGIN( Connection, "" )
		MAP_INTERFACE(Connection)

		MAP_ATTRIBUTE
		(    
			"schema",
			mSchema,
			"Table and stored proc. schema.",
			Be::READ
		)
	EXPOSURE_END()
}
