// Copyright © 2005 CCP ehf.

/* 
	*************************************************************************

	ColumnDescriptor.h

	Project:   EVE Server Database Access

	Description:   

		Stores information relating to a column
		
	Dependencies:

		Python

	*************************************************************************
*/

#pragma once

#ifndef _COLUMNDESCRIPTOR_H_
#define _COLUMNDESCRIPTOR_H_

#include "DelayedException.h"

#include <string>

struct ColumnDescriptor
{
	ColumnDescriptor( const char* name ) :
		mName( name ), mType( 0 ), mOffset( 0 ), mSize( 0 )
	{
	}
	static DelayedException* TypeTranslate( DBTYPE& type, int& size );

	std::string mName;
	int mOffset;
	DBTYPE mType;
	char mSize;
};

#endif