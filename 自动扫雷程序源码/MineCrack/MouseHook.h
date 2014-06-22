// MouseHook.h: interface for the CMouseHook class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_MOUSEHOOK_H_)
#define _MOUSEHOOK_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class AFX_EXT_CLASS CMouseHook : public CObject
{
public:
	BOOL StartMouseHook(HWND hwnd);
	BOOL StopMouseHook();
	BOOL StartKeyBoardHook();
	BOOL StopKeyBoardHook();
	CMouseHook();
	virtual ~CMouseHook();
};

#endif // !defined(_MOUSEHOOK_H_)
