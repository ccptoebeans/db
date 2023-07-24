#include "StdAfx.h"
#include "TestTools.h"

BLUE_DEFINE( TestTools );

const Be::ClassInfo* TestTools::ExposeToBlue()
{
    EXPOSURE_BEGIN( TestTools, "" )
		MAP_INTERFACE( TestTools )

		MAP_METHOD_AND_WRAP(
			"TestParameter",
			TestParameter,
			"" )

    EXPOSURE_END()
}