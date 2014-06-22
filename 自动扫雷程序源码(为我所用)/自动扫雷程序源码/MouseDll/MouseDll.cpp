// MouseDll.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <afxdllx.h>
#include "MouseHook.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma data_seg(".myData")
HHOOK glhMouseHook = NULL;			//安装的鼠标勾子句柄
HHOOK glhKeyBoardHook = NULL;		//安装键盘勾子句柄
HINSTANCE glhInstance = NULL;		//DLL实例句柄
HWND glMainWin = NULL;
TCHAR winMineClassName[] = "扫雷";
#pragma data_seg()
#pragma comment(linker, "/SECTION:.myData,rws")

struct MsgMouse
{
	CPoint pt;
	int KeyValue;
};

LRESULT WINAPI MouseProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI KeyBoardProc(int nCode, WPARAM wParam, LPARAM lParam);

static AFX_EXTENSION_MODULE MouseDllDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("MOUSEDLL.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(MouseDllDLL, hInstance))
			return 0;

		glhInstance = hInstance;
		
		new CDynLinkLibrary(MouseDllDLL);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("MOUSEDLL.DLL Terminating!\n");
		// Terminate the library before destructors are called
		AfxTermExtensionModule(MouseDllDLL);
	}
	return 1;   // ok
}

//////////////////////////////////////////////////////////////////////
// CMouseHook Class
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMouseHook::CMouseHook()
{

}

CMouseHook::~CMouseHook()
{
	StopMouseHook();
	StopKeyBoardHook();
}

BOOL CMouseHook::StartMouseHook(HWND hwnd)
{
	if (glhMouseHook) return TRUE;

	BOOL bResult = FALSE;
	glhMouseHook = SetWindowsHookEx(WH_MOUSE, MouseProc, glhInstance, 0);
	if (glhMouseHook != NULL) 
	{
		glMainWin = hwnd;
		bResult = TRUE;
	}
	return bResult;
}

BOOL CMouseHook::StopMouseHook()
{
	BOOL bResult = FALSE;
	if (glhMouseHook) bResult = UnhookWindowsHookEx(glhMouseHook);
	if (bResult) 
	{
		glhMouseHook = NULL;
		glMainWin = NULL;
	}
	return bResult;
}

LRESULT WINAPI MouseProc(int nCode,WPARAM wParam,LPARAM lParam)
{
	LPMOUSEHOOKSTRUCT pMouseHook=(MOUSEHOOKSTRUCT FAR *)lParam;
	
	HWND hWinMineWnd = ::FindWindow(winMineClassName,NULL);
	if (nCode < HC_ACTION || hWinMineWnd != pMouseHook->hwnd)
		return CallNextHookEx(glhMouseHook,nCode,wParam,lParam); 
	if ( GetForegroundWindow() != hWinMineWnd ) 
		return CallNextHookEx(glhMouseHook,nCode,wParam,lParam); 
	
	if ( wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN ||
		wParam == WM_LBUTTONUP || wParam == WM_RBUTTONUP ) 
	{
		MsgMouse Msg;
		Msg.pt = pMouseHook->pt;
		Msg.KeyValue = wParam;
		COPYDATASTRUCT cs;
		cs.dwData = 0;			
		cs.cbData = sizeof(MsgMouse);	
		cs.lpData = &Msg;		
		SendMessage(glMainWin, WM_COPYDATA, (WPARAM)pMouseHook->hwnd, (LPARAM)&cs);
	}
	
	return CallNextHookEx(glhMouseHook,nCode,wParam,lParam);		
}

BOOL CMouseHook::StartKeyBoardHook()
{
	BOOL bResult = FALSE;
	glhKeyBoardHook = SetWindowsHookEx(WH_MOUSE, KeyBoardProc, glhInstance, 0);
	if (glhKeyBoardHook != NULL) bResult = TRUE;
	return bResult;
}

BOOL CMouseHook::StopKeyBoardHook()
{
	BOOL bResult = FALSE;
	if (glhKeyBoardHook) bResult = UnhookWindowsHookEx(glhKeyBoardHook);
	if (bResult) glhKeyBoardHook = NULL;
	return bResult;
}

LRESULT WINAPI KeyBoardProc(int nCode,WPARAM wParam,LPARAM lParam)
{
	
	return CallNextHookEx(glhKeyBoardHook,nCode,wParam,lParam);		
}
