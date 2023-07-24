/* 
	*************************************************************************

	MockSqlCommand.h

	Author:    James Hawk
	Created:   July. 2023

	Description:   

		Allows direct entry point to SetPyParam for testing input parameter
		processing.

	(c) CCP 2023

	*************************************************************************
*/

#pragma once
#ifndef _MOCKSQLCOMMAND_H_
#define _MOCKSQLCOMMAND_H_

#include "msoledbsql.h"

#include "SqlCommand.h"

class MockNSession;

class MockSqlCommand : public SQLCommand
{
public:
	MockSqlCommand( NSession* nSession);

	~MockSqlCommand();

	bool SetPyParamTest( DBTYPE parameter_type, PyObject* val, void*& blob, size_t& data_length );

private:
	unsigned long long m_buffer_length;
	DBTYPE m_current_target_type;
	PyObject* m_value;
	static const wchar_t* s_dummy_name;
};

#endif