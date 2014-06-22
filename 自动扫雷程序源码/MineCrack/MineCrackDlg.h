// MineCrackDlg.h : header file
//

#if !defined(AFX_MINECRACKDLG_H__9EA0C357_FEFF_49AC_8440_3A04B3A26647__INCLUDED_)
#define AFX_MINECRACKDLG_H__9EA0C357_FEFF_49AC_8440_3A04B3A26647__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>
#include "MouseHook.h"

/////////////////////////////////////////////////////////////////////////////
// CMineCrackDlg dialog
#define UNKNOW			0x00
#define NOMINE			0xFF
#define ISMINE			0x0F
#define NEIGHBOR_1		0x01
#define NEIGHBOR_2		0x02
#define NEIGHBOR_3		0x03
#define NEIGHBOR_4		0x04
#define NEIGHBOR_5		0x05
#define NEIGHBOR_6		0x06
#define NEIGHBOR_7		0x07
#define NEIGHBOR_8		0x08
#define BMPWIDTH		16
#define BMPHEIGHT		16

struct MsgMouse
{
	CPoint pt;
	int KeyValue;
};

class CMineCrackDlg : public CDialog
{
	
// Construction
public:
	CMineCrackDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CMineCrackDlg)
	enum { IDD = IDD_MINECRACK_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMineCrackDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	BOOL m_bWin98;

	CMouseHook m_hook;

	CString	winMineClassName;
	HWND hWinMineWnd;
	CRect rectWinMine;
	CRect rectMe;

	BOOL m_bStart;
	int	m_nWidth;
	int	m_nHeight;
	int	m_nMineNum;
	int	m_nMineCurNum;
	BYTE *m_map;
	BYTE m_ColorSample[16][8];

	void StartGame();
	BOOL ScanTheMap();
	BOOL ScanTheMap98();
	int	GetColorSample(CPoint pt);
	BYTE Color256To16(COLORREF colorref);
	BOOL MakeDecision(BOOL *, BOOL *);		//返回：1  成功；0：继续
	BOOL AdvanceDecision(BOOL *, BOOL *);
	int LookUpFromArray(const void *, CPoint);
	void LookAroundDot(int, int, void *, int *);
		
	// Generated message map functions
	//{{AFX_MSG(CMineCrackDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBtstart();
	afx_msg void OnBtsweepmine();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnAbout();
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MINECRACKDLG_H__9EA0C357_FEFF_49AC_8440_3A04B3A26647__INCLUDED_)





















