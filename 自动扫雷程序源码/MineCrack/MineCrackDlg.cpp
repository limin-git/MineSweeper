// MineCrackDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MineCrack.h"
#include "MineCrackDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMineCrackDlg dialog

CMineCrackDlg::CMineCrackDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMineCrackDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMineCrackDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_nWidth = m_nHeight =0;
	m_nMineNum = 0;
	m_nMineCurNum = 0;

	m_bWin98 = FALSE;
	m_bStart = FALSE;
	m_map = NULL;
}

void CMineCrackDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMineCrackDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMineCrackDlg, CDialog)
	//{{AFX_MSG_MAP(CMineCrackDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTSTART, OnBtstart)
	ON_BN_CLICKED(IDC_BTSWEEPMINE, OnBtsweepmine)
	ON_BN_CLICKED(IDC_ABOUT, OnAbout)
	ON_WM_COPYDATA()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMineCrackDlg message handlers

BOOL CMineCrackDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	srand((unsigned)time(NULL));
	rand();

	DWORD dwBuild;
	
	DWORD dwVersion = GetVersion();
	
	// Get major and minor version numbers of Windows
	DWORD dwWindowsMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));
	DWORD dwWindowsMinorVersion =  (DWORD)(HIBYTE(LOWORD(dwVersion)));
	// Get build numbers for Windows NT or Win32s
	if (dwVersion < 0x80000000)                // Windows NT
		dwBuild = (DWORD)(HIWORD(dwVersion));
	else if (dwWindowsMajorVersion < 4)        // Win32s
		dwBuild = (DWORD)(HIWORD(dwVersion) & ~0x8000);
	else         // Windows 95 and Windows 98 -- No build numbers provided
		dwBuild =  0;
	
	CString str;
	WORD lan = (WORD)::GetSystemDefaultLangID();
	if (lan == 0x0804) 	
		winMineClassName = "扫雷";
	else if (lan == 0x0409)	
		winMineClassName = "Minesweeper";
	
	if (dwWindowsMajorVersion==5 && dwWindowsMinorVersion==2)
	{
		str.Format("Windows 2003  %d.%d.%d, %s", dwWindowsMajorVersion,
			dwWindowsMinorVersion, dwBuild, (lan == 0x0804) ? "简体中文版" : "English(USA) Edition");
		m_bWin98 = TRUE;
	} 
	else if(dwWindowsMajorVersion==5 && dwWindowsMinorVersion==1) 
	{
		str.Format("Windows XP  %d.%d.%d, %s",dwWindowsMajorVersion,
			dwWindowsMinorVersion,dwBuild, (lan == 0x0804) ? "简体中文版" : "English(USA) Edition");
		m_bWin98 = FALSE;
	} 
	else if(dwWindowsMajorVersion==5 && dwWindowsMinorVersion==0) 
	{
		str.Format("Windows 2000  %d.%d.%d, %s",dwWindowsMajorVersion,
			dwWindowsMinorVersion,dwBuild, (lan == 0x0804) ? "简体中文版" : "English(USA) Edition");
		m_bWin98 = FALSE;
	} 
	else if(dwWindowsMajorVersion==4 && dwWindowsMinorVersion==10) 
	{
		str.Format("Windows 98  %d.%d.%d, %s",dwWindowsMajorVersion,
			dwWindowsMinorVersion,dwBuild, (lan == 0x0804) ? "简体中文版" : "English(USA) Edition");
		m_bWin98 = TRUE;
	}
	else 
	{
		str.Format("Unknow Windows %d.%d.%d, %s",dwWindowsMajorVersion,
			dwWindowsMinorVersion,dwBuild, (lan == 0x0804) ? "简体中文版" : "English(USA) Edition");
		m_bWin98 = TRUE;
	}

	( (CStatic *)GetDlgItem(IDC_STCSHOW) ) -> SetWindowText(str);

/*
	CFile fl;
	if ( fl.Open("Sign.bmp", CFile::modeRead) )
	{
		BITMAPFILEHEADER bitmapfileheader;
		BITMAPINFOHEADER bitmapinfoheader;
		fl.Read((void *)&bitmapfileheader, sizeof(BITMAPFILEHEADER));
		ASSERT(bitmapfileheader.bfType == 0x4d42);	//BM
		fl.Read((void *)&bitmapinfoheader, sizeof(BITMAPINFOHEADER));
		ASSERT(bitmapinfoheader.biWidth == 16);		//位图宽16像素
		ASSERT(bitmapinfoheader.biHeight == 256);	//位图高256像素
		ASSERT(bitmapinfoheader.biBitCount == 4);	//位图为16色
		BYTE * bmpData = new BYTE[bitmapinfoheader.biSizeImage];
		fl.Seek(bitmapfileheader.bfOffBits, CFile::begin);
		fl.Read(bmpData, bitmapinfoheader.biSizeImage);
		
		for( int i=0; i<16; i++)
			for( int j=0; j<8; j++)
				m_ColorSample[i][j] = * (bmpData + i*256/2 + 8*8 + j);
		delete bmpData;
		fl.Close();
	}
	else
	{
		MessageBox("Sign.bmp 文件不存在！");
		CDialog::EndDialog(0);
		return FALSE;
	}

*/

	m_ColorSample[ 0][ 0] = 0x78; 
	m_ColorSample[ 0][ 1] = 0x88;
	m_ColorSample[ 0][ 2] = 0x88;
	m_ColorSample[ 0][ 3] = 0x88;
	m_ColorSample[ 0][ 4] = 0x88;
	m_ColorSample[ 0][ 5] = 0x88;
	m_ColorSample[ 0][ 6] = 0x88;
	m_ColorSample[ 0][ 7] = 0x88;
	m_ColorSample[ 1][ 0] = 0x78;
	m_ColorSample[ 1][ 1] = 0x88;
	m_ColorSample[ 1][ 2] = 0x88;
	m_ColorSample[ 1][ 3] = 0x8c;
	m_ColorSample[ 1][ 4] = 0xcc;
	m_ColorSample[ 1][ 5] = 0x88;
	m_ColorSample[ 1][ 6] = 0x88;
	m_ColorSample[ 1][ 7] = 0x88;
	m_ColorSample[ 2][ 0] = 0x78;
	m_ColorSample[ 2][ 1] = 0x88;
	m_ColorSample[ 2][ 2] = 0x88;
	m_ColorSample[ 2][ 3] = 0x88;
	m_ColorSample[ 2][ 4] = 0x22;
	m_ColorSample[ 2][ 5] = 0x22;
	m_ColorSample[ 2][ 6] = 0x88;
	m_ColorSample[ 2][ 7] = 0x88;
	m_ColorSample[ 3][ 0] = 0x78;
	m_ColorSample[ 3][ 1] = 0x88;
	m_ColorSample[ 3][ 2] = 0x88;
	m_ColorSample[ 3][ 3] = 0x99;
	m_ColorSample[ 3][ 4] = 0x99;
	m_ColorSample[ 3][ 5] = 0x99;
	m_ColorSample[ 3][ 6] = 0x88;
	m_ColorSample[ 3][ 7] = 0x88;
	m_ColorSample[ 4][ 0] = 0x78;
	m_ColorSample[ 4][ 1] = 0x84;
	m_ColorSample[ 4][ 2] = 0x44;
	m_ColorSample[ 4][ 3] = 0x44;
	m_ColorSample[ 4][ 4] = 0x44;
	m_ColorSample[ 4][ 5] = 0x44;
	m_ColorSample[ 4][ 6] = 0x48;
	m_ColorSample[ 4][ 7] = 0x88;
	m_ColorSample[ 5][ 0] = 0x78;
	m_ColorSample[ 5][ 1] = 0x81;
	m_ColorSample[ 5][ 2] = 0x11;
	m_ColorSample[ 5][ 3] = 0x11;
	m_ColorSample[ 5][ 4] = 0x11;
	m_ColorSample[ 5][ 5] = 0x11;
	m_ColorSample[ 5][ 6] = 0x88;
	m_ColorSample[ 5][ 7] = 0x88;
	m_ColorSample[ 6][ 0] = 0x78;
	m_ColorSample[ 6][ 1] = 0x86;
	m_ColorSample[ 6][ 2] = 0x66;
	m_ColorSample[ 6][ 3] = 0x66;
	m_ColorSample[ 6][ 4] = 0x66;
	m_ColorSample[ 6][ 5] = 0x66;
	m_ColorSample[ 6][ 6] = 0x88;
	m_ColorSample[ 6][ 7] = 0x88;
	m_ColorSample[ 7][ 0] = 0x78;
	m_ColorSample[ 7][ 1] = 0x88;
	m_ColorSample[ 7][ 2] = 0x88;
	m_ColorSample[ 7][ 3] = 0x88;
	m_ColorSample[ 7][ 4] = 0x80;
	m_ColorSample[ 7][ 5] = 0x00;
	m_ColorSample[ 7][ 6] = 0x88;
	m_ColorSample[ 7][ 7] = 0x88;
	m_ColorSample[ 8][ 0] = 0x78;
	m_ColorSample[ 8][ 1] = 0x88;
	m_ColorSample[ 8][ 2] = 0x77;
	m_ColorSample[ 8][ 3] = 0x77;
	m_ColorSample[ 8][ 4] = 0x77;
	m_ColorSample[ 8][ 5] = 0x77;
	m_ColorSample[ 8][ 6] = 0x88;
	m_ColorSample[ 8][ 7] = 0x88;
	m_ColorSample[ 9][ 0] = 0x78;
	m_ColorSample[ 9][ 1] = 0x88;
	m_ColorSample[ 9][ 2] = 0x88;
	m_ColorSample[ 9][ 3] = 0x88;
	m_ColorSample[ 9][ 4] = 0x88;
	m_ColorSample[ 9][ 5] = 0x00;
	m_ColorSample[ 9][ 6] = 0x88;
	m_ColorSample[ 9][ 7] = 0x88;
	m_ColorSample[10][ 0] = 0x78;
	m_ColorSample[10][ 1] = 0x88;
	m_ColorSample[10][ 2] = 0x00;
	m_ColorSample[10][ 3] = 0xff;
	m_ColorSample[10][ 4] = 0x00;
	m_ColorSample[10][ 5] = 0x00;
	m_ColorSample[10][ 6] = 0x08;
	m_ColorSample[10][ 7] = 0x88;
	m_ColorSample[11][ 0] = 0x78;
	m_ColorSample[11][ 1] = 0x88;
	m_ColorSample[11][ 2] = 0x00;
	m_ColorSample[11][ 3] = 0x99;
	m_ColorSample[11][ 4] = 0x09;
	m_ColorSample[11][ 5] = 0x90;
	m_ColorSample[11][ 6] = 0x08;
	m_ColorSample[11][ 7] = 0x88;
	m_ColorSample[12][ 0] = 0x79;
	m_ColorSample[12][ 1] = 0x99;
	m_ColorSample[12][ 2] = 0x00;
	m_ColorSample[12][ 3] = 0xff;
	m_ColorSample[12][ 4] = 0x00;
	m_ColorSample[12][ 5] = 0x00;
	m_ColorSample[12][ 6] = 0x09;
	m_ColorSample[12][ 7] = 0x99;
	m_ColorSample[13][ 0] = 0xff;
	m_ColorSample[13][ 1] = 0x88;
	m_ColorSample[13][ 2] = 0x88;
	m_ColorSample[13][ 3] = 0x88;
	m_ColorSample[13][ 4] = 0x00;
	m_ColorSample[13][ 5] = 0x88;
	m_ColorSample[13][ 6] = 0x88;
	m_ColorSample[13][ 7] = 0x77;
	m_ColorSample[14][ 0] = 0xff;
	m_ColorSample[14][ 1] = 0x88;
	m_ColorSample[14][ 2] = 0x88;
	m_ColorSample[14][ 3] = 0x89;
	m_ColorSample[14][ 4] = 0x98;
	m_ColorSample[14][ 5] = 0x88;
	m_ColorSample[14][ 6] = 0x88;
	m_ColorSample[14][ 7] = 0x77;
	m_ColorSample[15][ 0] = 0xff;
	m_ColorSample[15][ 1] = 0x88;
	m_ColorSample[15][ 2] = 0x88;
	m_ColorSample[15][ 3] = 0x88;
	m_ColorSample[15][ 4] = 0x88;
	m_ColorSample[15][ 5] = 0x88;
	m_ColorSample[15][ 6] = 0x88;
	m_ColorSample[15][ 7] = 0x77;

	StartGame();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMineCrackDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMineCrackDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMineCrackDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CMineCrackDlg::StartGame()
{
	hWinMineWnd = ::FindWindow(winMineClassName,NULL);
	
	if (hWinMineWnd == NULL) 
	{
		TCHAR strSysPath[ MAX_PATH ] = { '0' };
		GetWindowsDirectory( strSysPath, MAX_PATH );
		lstrcat( strSysPath, TEXT( "\\System32\\WinMine.exe" ) );
		
		PROCESS_INFORMATION ProcessInfo;
		STARTUPINFO	StartupInfo;
		ZeroMemory( & ProcessInfo, sizeof( PROCESS_INFORMATION ) );
		ZeroMemory( & StartupInfo, sizeof( STARTUPINFO ) );
		StartupInfo.cb = sizeof( StartupInfo );
		
		if ( CreateProcess( strSysPath,
			NULL, NULL, NULL, FALSE, 0, NULL, NULL, 
			&StartupInfo, &ProcessInfo ) )
		{
			WaitForInputIdle( ProcessInfo.hProcess, INFINITE );
			hWinMineWnd = ::FindWindow(winMineClassName,NULL);
			ASSERT( hWinMineWnd );
		}
	}	
	
	if(::IsIconic(hWinMineWnd))	::ShowWindow(hWinMineWnd,SW_SHOWNORMAL);
	::BringWindowToTop(hWinMineWnd);
	::SetActiveWindow(hWinMineWnd);
	
	CRect rectWinMinelocal, rectMelocal;
	::GetWindowRect(hWinMineWnd, &rectWinMinelocal);
	::GetWindowRect(CWnd::m_hWnd, &rectMelocal);
	
	if ( rectWinMine != rectWinMinelocal || rectMe != rectMelocal )
	{
		int cxScreen = ::GetSystemMetrics(SM_CXSCREEN);
		int cyScreen = ::GetSystemMetrics(SM_CYSCREEN);
		::MoveWindow(
			hWinMineWnd, 
			(cxScreen-(rectWinMinelocal.right-rectWinMinelocal.left)-(rectMelocal.right-rectMelocal.left))/2,
			(cyScreen-(rectWinMinelocal.bottom-rectWinMinelocal.top))/2,
			rectWinMinelocal.right-rectWinMinelocal.left,
			rectWinMinelocal.bottom-rectWinMinelocal.top,
			TRUE);
		::MoveWindow(
			CWnd::m_hWnd,
			(cxScreen+(rectWinMinelocal.right-rectWinMinelocal.left)-(rectMelocal.right-rectMelocal.left))/2,
			(cyScreen-(rectWinMinelocal.bottom-rectWinMinelocal.top))/2,
			rectMelocal.right-rectMelocal.left,
			rectMelocal.bottom-rectMelocal.top,
			TRUE);
		
		::GetWindowRect(hWinMineWnd, &rectWinMine);
		::GetWindowRect(CWnd::m_hWnd, &rectMe);
		
		CRect tmpRect;
		::GetClientRect(hWinMineWnd, &tmpRect);
		if(m_bWin98)
		{
			m_nWidth=(tmpRect.right-tmpRect.left-24)/16;
			m_nHeight=(tmpRect.bottom-tmpRect.top-67)/16;
		}
		else
		{
			m_nWidth=(tmpRect.right-tmpRect.left-20)/16;
			m_nHeight=(tmpRect.bottom-tmpRect.top-63)/16;
		}
		if ( m_nWidth == 30 && m_nHeight == 16 )
			m_nMineNum = 99;
		else if ( m_nWidth == 16 && m_nHeight == 16 )
			m_nMineNum = 40;
		else if ( m_nWidth == 9 && m_nHeight == 9 )
			m_nMineNum = 10;
		else{
			MessageBox("本程序不支持自定义雷区！", NULL, MB_OK | MB_ICONINFORMATION);
			return;
		}

	}
	
	if (m_map) {
		delete m_map;
		m_map = NULL;
	}
	m_map = new BYTE[ m_nWidth*m_nHeight ];
	ZeroMemory(m_map, m_nWidth*m_nHeight);
	
	::PostMessage(hWinMineWnd,WM_KEYDOWN,VK_F2,0);
	m_nMineCurNum = 0;
	m_bStart = TRUE;

	m_hook.StartMouseHook( GetSafeHwnd() );
}

void CMineCrackDlg::OnBtstart() 
{
	// TODO: Add your control notification handler code here
	StartGame();
}

BOOL CMineCrackDlg::ScanTheMap()
{
	int i, j, ret;
	CPoint pt; 
	BOOL finish = FALSE;

	for (i=0; i<m_nHeight; i++)
	{
		for(j=0; j<m_nWidth; j++)
		{
			if (m_map[ i*m_nWidth + j ] != 0) continue;
			pt = CPoint( 12 + j*BMPWIDTH, 55 + i*BMPHEIGHT + 7 );	//坐标从上至下；
			ret = GetColorSample(pt);
			switch(ret) 
			{
			case 0:	m_map[ i*m_nWidth + j ] = NOMINE; break;
			case 1:	m_map[ i*m_nWidth + j ] = NEIGHBOR_1; break;
			case 2:	m_map[ i*m_nWidth + j ] = NEIGHBOR_2; break;
			case 3:	m_map[ i*m_nWidth + j ] = NEIGHBOR_3; break;
			case 4:	m_map[ i*m_nWidth + j ] = NEIGHBOR_4; break;
			case 5:	m_map[ i*m_nWidth + j ] = NEIGHBOR_5; break;
			case 6:	m_map[ i*m_nWidth + j ] = NEIGHBOR_6; break;
			case 7:	m_map[ i*m_nWidth + j ] = NEIGHBOR_7; break;
			case 8:	m_map[ i*m_nWidth + j ] = NEIGHBOR_8; break;
			case 9:	m_map[ i*m_nWidth + j ] = NOMINE; break;
			case 10: finish = TRUE; break;
			case 11: finish = TRUE; break;
			case 12: finish = TRUE; break;
			case 13: m_map[ i*m_nWidth + j ] = UNKNOW; break;
			case 14: m_map[ i*m_nWidth + j ] = ISMINE; break;	
			case 15: m_map[ i*m_nWidth + j ] = UNKNOW; break;
			}
		}
	}
	if ( finish ) return FALSE;
	else return TRUE;
}

BOOL CMineCrackDlg::ScanTheMap98()
{
	return FALSE;
}

int CMineCrackDlg::GetColorSample(CPoint pt)
{
	BYTE ptrColor[8];
	COLORREF colorref1,colorref2;
	int i;
	
	HDC	hDC = ::GetDC(NULL);

	memset(ptrColor, 0, 8);
	::ClientToScreen(hWinMineWnd,&pt);

	for(i=0; i<8; i++)
	{
		colorref1=::GetPixel(hDC, pt.x+i*2, pt.y);
		colorref2=::GetPixel(hDC, pt.x+i*2+1, pt.y);
		BYTE tmp1 = Color256To16(colorref1);
		BYTE tmp2 = Color256To16(colorref2);
		ptrColor[i] = tmp1<<4 | tmp2;
	}

	::ReleaseDC(NULL,hDC);	

	for(i=0; i<16; i++)
		if ( m_ColorSample[i][0] == ptrColor[0] && 
			 m_ColorSample[i][1] == ptrColor[1] && 
			 m_ColorSample[i][2] == ptrColor[2] && 
			 m_ColorSample[i][3] == ptrColor[3] &&
			 m_ColorSample[i][4] == ptrColor[4] && 
			 m_ColorSample[i][5] == ptrColor[5] && 
			 m_ColorSample[i][6] == ptrColor[6] && 
			 m_ColorSample[i][7] == ptrColor[7] ) 
		{
			return i; //返回匹配值
		}
		
		return -1;	//没有匹配
		
}

BYTE CMineCrackDlg::Color256To16(COLORREF colorref)
{
	BYTE ret;
	switch( colorref )
	{
	case RGB(  0,   0,   0): ret = 0x00; break;
	case RGB(128,   0,   0): ret = 0x01; break;
	case RGB(  0, 128,   0): ret = 0x02; break;
	case RGB(128, 128,   0): ret = 0x03; break;
	case RGB(  0,   0, 128): ret = 0x04; break;
	case RGB(128,   0, 128): ret = 0x05; break;
	case RGB(  0, 128, 128): ret = 0x06; break;
	case RGB(128, 128, 128): ret = 0x07; break;
	case RGB(192, 192, 192): ret = 0x08; break;
	case RGB(255,   0,   0): ret = 0x09; break;
	case RGB(  0, 255,   0): ret = 0x0A; break;
	case RGB(255, 255,   0): ret = 0x0B; break;
	case RGB(  0,   0, 255): ret = 0x0C; break;
	case RGB(255,   0, 255): ret = 0x0D; break;
	case RGB(  0, 255, 255): ret = 0x0E; break;
	case RGB(255, 255, 255): ret = 0x0F; break;
	}
	return ret;
}

BOOL CMineCrackDlg::MakeDecision(BOOL * bSign, BOOL * bStatus)
{
	int i, j, k, curMineCount = 0;
	
	for ( i=0; i<m_nHeight; i++ )
	{
		for ( j=0; j<m_nWidth; j++ )
		{
			if ( m_map[i*m_nWidth + j] == ISMINE ) { curMineCount++; continue; }
			if ( m_map[i*m_nWidth + j] < NEIGHBOR_1 || m_map[i*m_nWidth + j] > NEIGHBOR_8 ) continue;
			
			//计算数字方格(i,j)周围的8个方格中，有多少个未标记的方格；
			CArray < CPoint, CPoint > ArrayOfCenPt; 
			int isMineNumOfCenPt = 0; 
			
			LookAroundDot(i, j, &ArrayOfCenPt, &isMineNumOfCenPt);
			
			if ( ! ArrayOfCenPt.GetSize() ) continue;

			//如果方格(i,j)周围的未标记方格数 ＝ 方格(i,j)本身显示雷数值 － 周围已标记是雷的方格数
			if ( ArrayOfCenPt.GetSize() == m_map[i*m_nWidth + j] - isMineNumOfCenPt )
			{
				for ( k=0; k<ArrayOfCenPt.GetSize(); k++ )
				{
					//从数组中将点取出，给每个点标记“是雷”的标志
					::PostMessage( hWinMineWnd, WM_RBUTTONDOWN, MK_RBUTTON,
						MAKELPARAM(12+ArrayOfCenPt[k].y*BMPWIDTH+8, 55+ArrayOfCenPt[k].x*BMPHEIGHT+8) );
					m_map[ ArrayOfCenPt[k].x * m_nWidth + ArrayOfCenPt[k].y ] = ISMINE;
				}
				( *bSign ) = TRUE;
			}
			
			if ( m_map[i*m_nWidth + j] == isMineNumOfCenPt )
			{
				::PostMessage( hWinMineWnd, WM_LBUTTONDOWN, MK_LBUTTON | MK_RBUTTON,
					MAKELPARAM(12+j*BMPWIDTH+8, 55+i*BMPHEIGHT+8) );
				::PostMessage( hWinMineWnd, WM_RBUTTONUP, 0,
					MAKELPARAM(12+j*BMPWIDTH+8, 55+i*BMPHEIGHT+8) );
				( *bStatus ) = TRUE;
			}
		}	
	}

	if ( m_nMineNum == curMineCount )
	{
		for ( i=0; i<m_nHeight; i++ )
		{
			for ( j=0; j<m_nWidth; j++ )
			{
				if ( m_map[i*m_nWidth + j] == UNKNOW )
				{
					::PostMessage( hWinMineWnd, WM_LBUTTONDOWN, MK_LBUTTON,
						MAKELPARAM(12+j*BMPWIDTH+8, 55+i*BMPHEIGHT+8) );
					::PostMessage( hWinMineWnd, WM_LBUTTONUP, 0,
						MAKELPARAM(12+j*BMPWIDTH+8, 55+i*BMPHEIGHT+8) );
				}
			}
		}
		return TRUE;
	} else m_nMineCurNum = curMineCount;
	
	return FALSE;
}

//在普通决策（bSign == FALSE && bStatus == FALSE）条件下调用高级决策
BOOL CMineCrackDlg::AdvanceDecision(BOOL * bSign, BOOL * bStatus)
{
	int i, j, i1, j1, k, curMineCount = 0;
	
	for ( i=0; i<m_nHeight; i++ )
	{
		for ( j=0; j<m_nWidth; j++ )
		{
			if ( m_map[i*m_nWidth + j] < NEIGHBOR_1 || m_map[i*m_nWidth + j] > NEIGHBOR_8 ) continue;

			//计算方格(i, j)周围3×3区域中有多少个雷区和未标示区
			CArray < CPoint, CPoint > ArrayOfCenPt; 
			int isMineNumOfCenPt = 0; 

			LookAroundDot(i, j, &ArrayOfCenPt, &isMineNumOfCenPt);
			
			if ( ArrayOfCenPt.GetSize() == 0 ) continue;  //周围没有未标示区
			
			ASSERT( ArrayOfCenPt.GetSize() != 1 );	//在高级决策下这种情况不存在

			if ( ArrayOfCenPt.GetSize() == 2 )	//如果周围只有两个未标示区，则无法进行高级决策
			{
				ASSERT( m_map[i*m_nWidth + j] == isMineNumOfCenPt + 1 );
				continue;
			}

/*……得到了方格(i, j)周围已标示雷区点的个数isMineNumOfCenPt和
	  未标示区点的数组ArrayOfCenPt后，对方格(i, j)周围5×5区域进行扫描……*/
			
			int xx1, xx2, yy1, yy2;
			
			if ( i-2 >= 0 ) xx1 = i-2;
			else if ( i-1 >= 0 ) xx1 = i-1;
			else xx1 = i;
			
			if ( i+2 <= m_nHeight-1 ) xx2 = i+2;
			else if ( i+1 <= m_nHeight-1 ) xx2 = i+1;
			else xx2 = i;
			
			if ( j-2 >= 0 ) yy1 = j-2;
			else if ( j-1 >= 0 ) yy1 = j-1;
			else yy1 = j;
			
			if ( j+2 <= m_nWidth-1 ) yy2 = j+2;
			else if ( j+1 <= m_nWidth-1 ) yy2 = j+1;
			else yy2 = j;
			
			for ( i1=xx1; i1<=xx2; i1++ )
			{
				for ( j1=yy1; j1<=yy2; j1++ )
				{
					//对方格(i, j)自身不考察
					if ( i1==i && j1==j ) continue;	
					//对方格(i, j)周围5×5区域中标示为非数字方块不考察
					if ( m_map[i1*m_nWidth + j1] < NEIGHBOR_1 || m_map[i1*m_nWidth + j1] > NEIGHBOR_8 ) continue;
					
					CArray < CPoint, CPoint > ArrayOfArdPt; 
					int isMineNumOfArdPt = 0; 
					LookAroundDot( i1, j1, &ArrayOfArdPt, &isMineNumOfArdPt);
					
					if ( !ArrayOfArdPt.GetSize() ) continue; //若该点周围没有未标示方块

					CArray < int ,int > indexArray;
					BOOL haveOther = FALSE;
					for (k=0; k<ArrayOfArdPt.GetSize(); k++)
					{
						int retindex = LookUpFromArray( &ArrayOfCenPt, ArrayOfArdPt.GetAt(k) );
						if ( retindex >= 0 ) indexArray.Add(retindex);
						else if ( retindex == -1 ) haveOther = TRUE;
					}

					if ( !haveOther ) 
						ASSERT( indexArray.GetSize() != 1 );	//等于1的情况在MakeDecision()函数中处理过了
					if ( indexArray.GetSize() < 2 ) continue;	//若没有或有一个共同的未标示方格
					
					//CommonMineNum：两个点之间公共的未标示区域中包括的雷数
					int CommonMineNum = m_map[i1*m_nWidth + j1] - isMineNumOfArdPt;
					
					int leaveUnknowDot = ArrayOfCenPt.GetSize() - indexArray.GetSize();

					if ( !leaveUnknowDot ) continue;

					if (m_map[i*m_nWidth + j] - isMineNumOfCenPt - CommonMineNum == leaveUnknowDot)
					{	//剩下的未标示方格全为雷，标记
						for ( k=0; k<ArrayOfCenPt.GetSize(); k++ )
						{
							int retindex = LookUpFromArray( &ArrayOfArdPt, ArrayOfCenPt.GetAt(k) );
							if ( retindex == -1 )
							{
								::PostMessage( hWinMineWnd, WM_RBUTTONDOWN, MK_RBUTTON,
									MAKELPARAM(12+ArrayOfCenPt[k].y*BMPWIDTH+8, 55+ArrayOfCenPt[k].x*BMPHEIGHT+8) );
								m_map[ ArrayOfCenPt[k].x * m_nWidth + ArrayOfCenPt[k].y ] = ISMINE;
							}
						}
						( *bSign ) = TRUE;
						return TRUE;
					}
						
					if (m_map[i*m_nWidth + j] - isMineNumOfCenPt == CommonMineNum && haveOther == FALSE )	
					{	//剩下的方格全不是雷，点开		ps：haveOther＝FALSE，必然事件；haveOther＝TRUE，最大可能事件
						for ( k=0; k<ArrayOfCenPt.GetSize(); k++ )
						{
							int retindex = LookUpFromArray( &ArrayOfArdPt, ArrayOfCenPt.GetAt(k) );
							if ( retindex == -1 )
							{
								::PostMessage( hWinMineWnd, WM_LBUTTONDOWN, MK_LBUTTON,
									MAKELPARAM(12+ArrayOfCenPt[k].y*BMPWIDTH+8, 55+ArrayOfCenPt[k].x*BMPHEIGHT+8) );
								::PostMessage( hWinMineWnd, WM_LBUTTONUP, 0,
									MAKELPARAM(12+ArrayOfCenPt[k].y*BMPWIDTH+8, 55+ArrayOfCenPt[k].x*BMPHEIGHT+8) );
							}
						}
						( *bStatus ) = TRUE;
						return TRUE;
					}
				}
			}
		}
	}

	return FALSE;
}

void CMineCrackDlg::LookAroundDot(int i, int j, void * ptrarr, int * isMine)
{
	CArray < CPoint, CPoint > * ptrArray = ( CArray < CPoint, CPoint > * )ptrarr;
	
	int i1, j1;
	for ( i-1>=0 ? i1=i-1 : i1=i; i+1<=m_nHeight-1 ? i1<=i+1 : i1<=i; i1++ )	
	{
		for ( j-1>=0 ? j1=j-1 : j1=j; j+1<=m_nWidth-1 ? j1<=j+1 : j1<=j; j1++ )
		{
			if ( i1==i && j1==j ) 
			{
				ASSERT( m_map[i*m_nWidth + j] != 0 );
				continue;
			}
			if ( !m_map[i1*m_nWidth + j1] )	
			{
				CPoint pt = CPoint(i1, j1);
				ptrArray->Add(pt);
			} 
			else if ( m_map[i1*m_nWidth + j1] == ISMINE ) (* isMine)++;	
		}
	}
}

int CMineCrackDlg::LookUpFromArray(const void * ptrarr, CPoint pt)
{
	CArray < CPoint, CPoint > * ptrArray = ( CArray < CPoint, CPoint > * )ptrarr;

	for ( int i=0; i<ptrArray->GetSize(); i++ )
		if ( (* ptrArray)[i] == pt ) return i;
	
	return -1;
}

void CMineCrackDlg::OnBtsweepmine() 
{
	// TODO: Add your control notification handler code here
	
	if ( ! m_bStart )
	{
		MessageBox("先点击开始按钮！", NULL, MB_OK | MB_ICONINFORMATION);
		return;
	}

	if(::IsIconic(hWinMineWnd))	::ShowWindow(hWinMineWnd,SW_SHOWNORMAL);
	::BringWindowToTop(hWinMineWnd);
	::SetActiveWindow(hWinMineWnd);
	
	BOOL DoScan = TRUE;
	while ( TRUE )
	{
		Sleep(20); 
		if ( DoScan )
		{
			if( !ScanTheMap() )
			{
				m_bStart = FALSE;
				ASSERT( m_map !=NULL );
				delete m_map;
				m_map = NULL;
				MessageBox("扫雷失败了，GAME OVER！", NULL, MB_OK | MB_ICONINFORMATION);
				break;
			}
		} 
		else DoScan = TRUE;
		
		BOOL bSign = FALSE, bStatus = FALSE;
		
		BOOL bSuccess = MakeDecision( & bSign, & bStatus );
		
		if ( bSuccess )
		{
			m_bStart = FALSE;
			ASSERT( m_map !=NULL );
			delete m_map;
			m_map = NULL;
			MessageBox("恭喜啊，扫雷成功了！", NULL, MB_OK | MB_ICONINFORMATION); 
			break;
		}
		
		if ( bSign == FALSE && bStatus == FALSE ) 
		{
			BOOL improve = AdvanceDecision( & bSign, & bStatus );
			if ( ! improve ) 
			{
				MessageBox("算法不能继续了，自己来吧！", NULL, MB_OK | MB_ICONINFORMATION);
				::BringWindowToTop(hWinMineWnd);
				::SetActiveWindow(hWinMineWnd);
				break;
			}
		}

		if ( bStatus ) continue;
		if ( bSign ) DoScan = FALSE;
	}
}

void CMineCrackDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}

void CMineCrackDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CMineCrackDlg::OnAbout() 
{
	// TODO: Add your control notification handler code here
	CAboutDlg dlg;
	dlg.DoModal();
}

BOOL CMineCrackDlg::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct) 
{
	// TODO: Add your message handler code here and/or call default
	static BOOL bDown = FALSE;
	MsgMouse * pMsg;
	pMsg = (MsgMouse *) (pCopyDataStruct->lpData);
	CPoint pt = pMsg->pt;
	::ScreenToClient(hWinMineWnd,&pt);
	
	if (pt.x>239 && pt.x<264 && pt.y>15 && pt.y<40  ) 
	{
		if ( pMsg->KeyValue == WM_LBUTTONDOWN ) bDown = TRUE;
		if ( pMsg->KeyValue == WM_LBUTTONUP && bDown )
		{
			StartGame();
			bDown = FALSE;
		}
	}
	if ( pMsg->KeyValue == WM_LBUTTONUP && bDown )  bDown = FALSE;

	return CDialog::OnCopyData(pWnd, pCopyDataStruct);
}


