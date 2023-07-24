#include "MockSqlCommand.h"

const wchar_t* MockSqlCommand::s_dummy_name = L"DUMMY";

MockSqlCommand::MockSqlCommand( NSession* nSession ) :
	SQLCommand( nSession ),
	m_value(nullptr)
{
}

MockSqlCommand ::~MockSqlCommand()
{
}

bool MockSqlCommand::SetPyParamTest( DBTYPE parameter_type, PyObject* val, void*& blob, size_t& data_length )
{
	SetParamType( 0, parameter_type );

	size_t len;

	if(!SetPyParam( len, 0, val ))
	{
		return false;
	}

	data_length = len;
	
	blob = malloc( data_length );
	memcpy( blob, GetParam( 0 ), data_length );

	return true;
};