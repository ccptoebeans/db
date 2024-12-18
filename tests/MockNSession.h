/* 
	*************************************************************************

	MockNSession.h

	Author:    James Hawk
	Created:   July. 2023

	Description:   

		Offers and empty NSession with data converter access.
		Can be passed to processing functions which don't require
		valid database setup but do require data conversion.

	(c) CCP 2023

	*************************************************************************
*/

#pragma once
#ifndef _MOCKNSESSION_H_
#define _MOCKNSESSION_H_

#include "NSession.h"
#include <msdadc.h>	// for IDataConvert

class MockNSession : public NSession
{
public:
	MockNSession();
	~MockNSession();

	bool isValid();

private:

	bool m_valid;

};

#endif