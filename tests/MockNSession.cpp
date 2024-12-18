#include "MockNSession.h"

extern "C" const CLSID CLSID_DataConvert;

MockNSession::MockNSession():
	m_valid(true)
{
	HRESULT hr = mConv.CoCreateInstance( CLSID_DataConvert, NULL, CLSCTX_INPROC_SERVER );
	if (FAILED(hr))
	{
		m_valid = false;
	}
}

MockNSession::~MockNSession()
{

}

bool MockNSession::isValid()
{
	return m_valid;
}