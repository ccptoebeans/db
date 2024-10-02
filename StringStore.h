/* 
	*************************************************************************

	StringStore.h

	Author:    Kristjan Valur Jonsson
	Created:   Dec. 2006
	OS:        Win32
	Project:   EVE Server Database Access

	Description:   

		This guy provides functionality to reuse string and data objects from
		the database.  It is a singleton storage for byte data.
		When getting the rowset, all strings are entered into this, and it
		returns a pointer to a StringStore element, possibly shared.
		
	Dependencies:

		Python

	(c) CCP 2006

	*************************************************************************
*/

#ifndef STRINGSTORE_H
#define STRINGSTORE_H

#include <hash_set>
#include "DelayedException.h"
#include "BlockAllocator.h"

enum StringStoreElementType
{
	BYTESTRING,
	STRING,
	UNICODE
};

class StringStoreElem
{
	friend class StringStore;
public:
	StringStoreElem(char *data, size_t elems, bool isBytes);
	StringStoreElem(wchar_t *data, size_t elems);
	StringStoreElem(const StringStoreElem &o);
	~StringStoreElem();

	//copy constructor
	StringStoreElem &operator =(const StringStoreElem &o);

protected:
	DelayedException *Claim(SimplePoolAllocator &allocator); //copy the data into the element
	
public:
	bool operator < (const StringStoreElem &rhs) const;
	operator size_t ()  const {return mHash;}
	PyObject *GetPython(); //return a borrowed ref to the already converted python object.

private:
	DelayedException *MakeCopy();
	void Hash();
	bool ToPython(); //convert the string to python.  Once only.
	
	//store either a char or wchar data and length dude, or a pyobject
	union {
		struct {
			union {
				char *mCData;
				wchar_t *mWData;
			};
			size_t mElements;
			size_t mHash; //cashed hash
			StringStoreElementType mElementType;
		};
		PyObject *mObject;
	};
	bool mPython; //data is python (mObject is valid)
};


//And the hash set itself.
class StringStore
{
public:
	StringStore(SimplePoolAllocator &allocator);
	DelayedException *Insert(StringStoreElem* &res, char *data, size_t elems, bool isBytes);
	DelayedException *Insert(StringStoreElem* &res, wchar_t *data, size_t elems);
	size_t GetMemSaved() const;

private:
	typedef stdext::hash_compare<StringStoreElem, std::less<StringStoreElem> > traits_t;
	typedef StlPoolAllocator<StringStoreElem> allocator_t;

	SimplePoolAllocator &mAllocator;
	typedef stdext::hash_set<StringStoreElem, traits_t, allocator_t> StringStoreSet_t;
	typedef StringStoreSet_t::iterator StringStoreSet_i;

	StringStoreSet_t mSet;
	size_t mMemSaved; //how much memory (without overhead) saved by the reuse
};


#endif //STRINGSTORE_H