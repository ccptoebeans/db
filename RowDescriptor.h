/* 
	*************************************************************************

	RowDescriptor.h

	Author:    Kristjan Valur Jonsson
	Created:   Feb. 2005
	OS:        Win32
	Project:   EVE Server Database Access

	Split From:	TmpRowset.h
	by:			James Hawk
	Date:		July. 2023

	Description:   

		Stores information relating to a row
		
	Dependencies:

		Python

	(c) CCP 2023

	*************************************************************************
*/

#pragma once
#ifndef _ROWDESCRIPTOR_H_
#define _ROWDESCRIPTOR_H_

#include "DelayedException.h"
#include "OurAccessor.h"
#include "ColumnDescriptor.h"
#include "Accessor.h"

#define TTIMER1 __noop
#define TTIMER2 __noop

//A row descriptor object that isn't python.  We can't always create python objects
class NSession;
class RowDescriptor
{
public:
	RowDescriptor( NSession* sess ) :
		mNSession( sess )
	{
	}
	DelayedException* Init( ACCESSOR& a );
	PyObject* ToPython( PyObject* blueModule );

	//members.  First a list of descriptors
	typedef std::vector<ColumnDescriptor> columnList_t;
	columnList_t mColumnList;

	//row data is laid out thus:
	//[binary columns , null flags, objects]
	//there is a null flag for every column, indexed by column number, also
	//for Object columns (even though we could use PyNone there) for simplicity.

	int mSNull; //start of null flags, in bits
	int mDataLen; //length of integer data and null flags only, in bytes
	int mSObjects; //offset to first object, in pointers.
	int mNObjects; //number of objects trailing data

	int mTotalLen; //total length of data (including object pointers)

	NSession* const mNSession; //convenient place to store this
};

#endif