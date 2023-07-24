#include "MockAccessor.h"

const wchar_t* MockAccessor::s_dummy_column_name = L"DUMMY";

MockAccessor::MockAccessor():
	m_data_type( DBTYPE_BOOL ),
	m_data_length( 0 ),
	m_buffer_length( 2048*1024 ), //2MB blob size limit - matches nvarchar(MAX)
	m_param_length( 0 ),
	m_param_status( DBSTATUS_S_OK )
{
	m_dummy_buffer = new char [m_buffer_length];
	memset( m_dummy_buffer, '\0', m_buffer_length );
}

MockAccessor::~MockAccessor()
{
	delete [] m_dummy_buffer;
}