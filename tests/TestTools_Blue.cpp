#include "StdAfx.h"
#include "TestTools.h"

Be::VarChooser DbTypeGroupTypeChooser[] = {
	{ "BOOL", BeCast( TestTools::BOOL ), "Boolean value" },
	{ "I1", BeCast( TestTools::I1 ), "One-byte signed integer" },
	{ "UI1", BeCast( TestTools::UI1 ), "One-byte, unsigned integer" },
	{ "I2", BeCast( TestTools::I2 ), "Two-byte signed integer" },
	{ "UI2", BeCast( TestTools::UI2 ), "Two-byte unsigned integer" },
	{ "I4", BeCast( TestTools::I4 ), "Four-byte signed integer" },
	{ "UI4", BeCast( TestTools::UI4 ), "Four-byte unsigned integer" },
	{ "I8", BeCast( TestTools::I8 ), "Eight-byte signed integer" },
	{ "FILETIME", BeCast( TestTools::FILETIME ), "64 bit value - Test max storage capacity" },
	{ "UI8", BeCast( TestTools::UI8 ), "Eight-byte, unsigned integer" },
	{ "R4", BeCast( TestTools::R4 ), "Single-precision floating-point value" },
	{ "R8", BeCast( TestTools::R8 ), "Double-precision floating-point value" },
	{ "CY", BeCast( TestTools::CY ), "Currency is a fixed-point number with four digits to the right of the decimal point" },
	{ "STR", BeCast( TestTools::STR ), "Null-terminated ANSI/DBCS character string" },
	{ "BSTR", BeCast( TestTools::BSTR ), "Pointer to a BSTR, as in Automation: Typedef WCHAR * BSTR" },
	{ "WSTR", BeCast( TestTools::WSTR ), "Null-terminated Unicode character string" },
	{ "BYTES", BeCast( TestTools::BYTES ), "A binary data value. That is, an array of bytes" },
	{ "IUNKNOWN", BeCast( TestTools::IUNKNOWN ), "Pointer to an IUnknown interface on a COM object" },
	{ "DBTIMESTAMP", BeCast( TestTools::DBTIMESTAMP ), "Takes a DBTYPE_FILETIME and converts to DBTYPE_DBTIMESTAMP to store in db" },
	{ "DBDATE", BeCast( TestTools::DBDATE ), "Takes a DBTYPE_FILETIME and converts to DBTYPE_DBDATE to store in db" },
	{ "DBTIME2", BeCast( TestTools::DBTIME2 ), "Takes a DBTYPE_FILETIME and converts to DBTYPE_DBTIME2 to store in db" },
	{ "INVALID", BeCast( TestTools::INVALID ), "Invalid Type" },
	{ 0 }
};

BLUE_REGISTER_ENUM_EX( "dbType", TestTools::DbType, DbTypeGroupTypeChooser, ENUM_REG_ENUM_OBJECT_ON_MODULE );


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