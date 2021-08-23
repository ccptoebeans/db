/* 
	*************************************************************************

	DelayedException.h

	Author:    Kristjan Valur Jonsson
	Created:   Dec. 2006
	OS:        Win32
	Project:   EVE Server Database Access

	Description:   

		Classes for working on the thread.  No python stuff can happen here,
		so we have to have special support to set up errors that will be
		raised when python resumes.

	Dependencies:

		Python

	(c) CCP 2006

	*************************************************************************
*/

#ifndef DELAYEDEXCEPTION_H
#define DELAYEDEXCEPTION_H


//A delayed exception class.  Use this to create python exceptions without actually
//creating them, since we cannot touch the python runtime in our worker thread.
//We have special static methods for handling the case when a new exception
//cannot be allocated, due to out of memory situations.
class DelayedException
{
private:
	DelayedException(const char *msg, HRESULT hr, PyObject *cls);

public:
	static DelayedException *New(HRESULT hr, const char *msg);
	static DelayedException *New(PyObject *cls, const char *msg);
	static DelayedException *New(const char *msg);
	static DelayedException *NoMem(const char *fmt=0, ...);
	static DelayedException *Dummy() {return reinterpret_cast<DelayedException*>(-1);}

	void operator delete(void *p) throw();

	bool Raise();

	// is this an error we should drop the session because of?
	bool IsSessionFatal() const;
	
public:
	PyObject * const mCls;
	const HRESULT mHR;
	const std::string mMsg;
	ULONG mNErrors;
	CDBErrorInfo mErrorInfo;
	std::shared_ptr<DelayedException> mNext;
};
typedef std::shared_ptr<DelayedException> DelayedException_ptr;

#define PYERROR(c, m) return DelayedException::New((c), (m))
#define PYDBERROR(m) PYERROR(Utilities::DbExc_RuntimeError, (m))
#define PYWARN(c, m) DelayedException::New((c), (m))
#define PYDBWARN(m) PYWARN(PyExc_UserWarning, (m))


#endif // DELAYEDEXCEPTION_H