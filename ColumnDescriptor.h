/* 
	*************************************************************************

	ColumnDescriptor.h

	Author:    Kristjan Valur Jonsson
	Created:   Feb. 2005
	OS:        Win32
	Project:   EVE Server Database Access

	Split From:	TmpRowset.h
	by:			James Hawk
	Date:		July. 2023

	Description:   

		Stores information relating to a column
		
	Dependencies:

		Python

	(c) CCP 2023

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