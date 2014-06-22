// MineCrack.h : main header file for the MINECRACK application
//

#if !defined(AFX_MINECRACK_H__D3A17A70_E172_498F_B2D9_E79B1F2D67C0__INCLUDED_)
#define AFX_MINECRACK_H__D3A17A70_E172_498F_B2D9_E79B1F2D67C0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CMineCrackApp:
// See MineCrack.cpp for the implementation of this class
//

class CMineCrackApp : public CWinApp
{
public:
	CMineCrackApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMineCrackApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CMineCrackApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MINECRACK_H__D3A17A70_E172_498F_B2D9_E79B1F2D67C0__INCLUDED_)
