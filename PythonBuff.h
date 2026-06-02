// Copyright © 2023 CCP ehf.

/* 
	*************************************************************************

	PythonBuff.h

	Project:   EVE Server Database Access

	Description:   

		A simple SequentialStream wrapper around a pythonbuffer thing.
		It borrows the reference to its python object:  It is run in a thread
		and addrefing and decrefing is therefore not safe.  However, the caller
		owns a reference so this is ok.
		
	Dependencies:

		Python

	*************************************************************************
*/

#pragma once
#ifndef _PYTHONBUFF_H_
#define _PYTHONBUFF_H_


class PythonBuff : public ISequentialStream
{
public:
	PythonBuff(PyObject *);
	~PythonBuff();
	bool Valid() const
	{
		return m_buff;
	}
	size_t GetLength() const
	{
		return m_size;
	}

	HRESULT WINAPI Read(void* pv, ULONG cb, ULONG* got);
	HRESULT WINAPI Write(const void* pv, ULONG cb, ULONG* written) {return E_NOTIMPL;}
	
	HRESULT	WINAPI QueryInterface(REFIID riid, void** ppv);
	
	ULONG WINAPI AddRef() {return ++m_refcount;}

	ULONG WINAPI Release()
	{
		if( --m_refcount == 0 )
			delete this;
		return m_refcount;
	}

private:
	char* m_buff;
	size_t m_size;
	size_t m_pos;
	int m_refcount;
};


#endif