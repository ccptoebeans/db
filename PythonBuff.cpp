#include "PythonBuff.h"

//A simple Com wrapper for the python buffer
PythonBuff::PythonBuff( PyObject* obj ) :
	m_buff( nullptr ),
	m_size( 0 ),
	m_pos( 0 ),
	m_refcount( 0 )
{
	if( !PyObject_CheckBuffer( obj ) )
	{
		PyErr_SetString( PyExc_TypeError, "PythonbBuff object must have a buffer interface." );
		return;
	}

	Py_buffer view;

	if( PyObject_GetBuffer( obj, &view, 0 ) != 0 )
	{
		PyErr_SetString( PyExc_TypeError, "PythonbBuff object must have a buffer interface." );
		return;
	}

	if( !PyBuffer_IsContiguous( &view, 'A' ) )
	{
		PyErr_SetString( PyExc_TypeError, "PythonbBuff object buffer must be contiguous." );
	}

	m_size = view.len;
	m_buff = (char*)view.buf;

}

PythonBuff::~PythonBuff()
{}


HRESULT WINAPI PythonBuff::Read(void* pv, ULONG cb, ULONG* got)
{
	HRESULT hr = S_OK;
	if( cb > m_size - m_pos )
	{
		cb = (ULONG)( m_size - m_pos );
		hr = S_FALSE;
	}
	memcpy( pv, m_buff + m_pos, cb );
	m_pos += cb;
	if (got)
		*got = cb;
	return hr;
}


HRESULT	WINAPI PythonBuff::QueryInterface(REFIID riid, void** ppv)
{
	if (riid == IID_IUnknown)
		*ppv = (IUnknown*)this;
	else if (riid == IID_ISequentialStream)
		*ppv = (IUnknown*)(ISequentialStream*)this;
	else
		return E_NOINTERFACE;
	++m_refcount;
	return S_OK;
}
