#include "MockNSession.h"

extern "C" const CLSID CLSID_DataConvert;

MockNSession::MockNSession()
{
	HRESULT hr = mConv.CoCreateInstance( CLSID_DataConvert, NULL, CLSCTX_INPROC_SERVER );
}

MockNSession::~MockNSession()
{

}

CComPtr<IDataConvert> MockNSession::converter()
{
	return mConv;
}