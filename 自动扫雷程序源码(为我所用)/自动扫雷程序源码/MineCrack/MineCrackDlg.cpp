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

#include <iostream>
#include <ctime>
#include <list>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <map>
#include <stack>
#include "Point.h"
#include "MyCombination.h"
#include "Assistant.h"
using namespace std;

int CONTINUE = 0;
int FAIL = 1;
int SUCCESS = 2;
int m_row, m_col, m_num;
int MASKED = 10;
int MARKED = 11;

// 算法:
// 1. 定义雷区, 初始化变量
// 2. 扫描雷区, 获取雷区信息
// 3. 用集合算法make_set_of_sub_mine_field() 将雷区内边界化成子雷区集合
// 4. 用夹逼算法converge() 进一步拆分子雷区集合
// 5. 搜索子雷区集合search_mine_and_nomine_to_set中的雷和非雷并加入到相应集合
// 6. 用make_command_list通过雷和非雷集合得到命令列表
// 7. 执行命令列表excute_the_command_list

typedef set< Point > S_O_P;			// 格的位置的集合(set of position)
typedef int N_O_M;					// 雷数(number of mines)
typedef pair< S_O_P, N_O_M > S_M_F;	// 子雷区(sub mine field)
set< Point > Map_Backup_Point;		// 候补位置集合, 以POSB升序排序
set< S_M_F > Set_Of_Sub_Mine_Field;	// 子雷区集合
S_O_P Set_Mine, Set_Nomine;			// 雷和非雷位置集

// 定义:
// 
// 格(pane):						雷区中的一个方格
// 空白格(blank pane)				周围没有地雷
// 未知格(unknown pane):			没有打开与没有标记的格
// 标记格(marked pane):				标记为雷的格
// 问题格(questioned pane):			标记为问号的格
// 数字格(numbered pane):			内容为数字的格
// 雷区外边界(outter bolderline) :	一系列相邻的周围有未知格的数字格
// 雷区内边界(inner bolderline):	一系列相信的周围有数字格的未知格

// 子雷区(sub mine field):	由1-8个与一个数字格相邻的未知格组成的区域

bool is_number(int b)
{
	return (1 <= b && b <= 8);
}
int cnt_masked_near(int **m, int x, int y) 
{	// field[x][y]这个位置周围有几个未知格
	for (int i = 0, c = 0; i < 8; ++i) {
		if (m[x + OFFSET[i].getx()][y + OFFSET[i].gety()] == MASKED) ++c;
	}
	return c;
}

int cnt_num_near(int **m, int x, int y)
{	// field[x][y]这个位置周围有几个未知格
	for (int i = 0, c = 0; i < 8; ++i) {
		if (is_number(m[x + OFFSET[i].getx()][y + OFFSET[i].gety()])) ++c;
	}
	return c;
}

int cnt_marked_near(int **m, int x, int y)
{	// field[x][y]这个位置周围有几个未知格
	for (int i = 0, c = 0; i < 8; ++i) {
		if (m[x + OFFSET[i].getx()][y + OFFSET[i].gety()] == MARKED) ++c;
	}
	return c;
}

void add_and_splite_the_set_of_sub_mine_field(set < S_M_F>& s, const S_M_F& smf)
{
	// 如果集合s中的子雷区与该子雷区f的位置集合有包含或被包含关系时, 
	// 求这两个位置集合的差集作为一个新的子雷区的位置集合
	// 删除含有较大位置集的子雷区
	// 将子雷区插入到子雷区集合中
	// 对新的子雷区重复以上操作
	set < S_M_F > s_del;			// 待删除的子雷区
	stack < S_M_F > s_new;			// 新产生的子雷区
	vector < Point > buff;
	bool Flag = true;				// 判断是否将f插入到子雷区集合
	set< S_M_F >::iterator _i;
	S_M_F f = smf;
	s_new.push(f);
	do {
		Flag = true;
		for ( _i = s.begin(); _i != s.end(); ++_i) {
			if (includes((*_i).first.begin(), (*_i).first.end(), f.first.begin(), f.first.end()) ) {
				set_difference(((*_i).first).begin(), ((*_i).first).end(), (f.first).begin(), (f.first).end(), back_inserter(buff));
				S_O_P t;
				t.insert(buff.begin(), buff.end());
				buff.clear();
				if (t.size() > 0) {
					s_new.push(S_M_F(t, (*_i).second - f.second));		// 新子雷区入栈
					s_del.insert(*_i);									// 收集待删除子雷区
				}
			}
			else if (includes(f.first.begin(), f.first.end(), (*_i).first.begin(), (*_i).first.end())) {
				set_difference((f.first).begin(), (f.first).end(), ((*_i).first).begin(), ((*_i).first).end(), back_inserter(buff));
				S_O_P t;
				t.insert(buff.begin(), buff.end());
				buff.clear();
				if (t.size() > 0) {
					s_new.push(S_M_F(t, f.second - (*_i).second));
					Flag = false;
				}
			}
		} 
		if (Flag) s.insert(f);
		_i = s_del.begin();
		for (; _i != s_del.end(); ++_i) {
			s.erase(*_i);
		}
		s_del.clear();
		f = s_new.top();
		s_new.pop();
	} while (!s_new.empty());
}

void make_set_of_sub_mine_field(int **m, set< S_M_F >& s)
{	// 将雷区内边界划分成子雷区集合

	S_O_P sop;
	Point p;
	for (int i = 1; i <= m_row; i++) {
		for (int j = 1; j <= m_col; j++) {
			if (is_number(m[i][j])) { // 在雷区外边界搜未知格
				sop.clear();
				for (int k = 0; k < 8; k++) {	// 查看四周
					p = Point(i, j) + OFFSET[k];
					if (m[p.getx()][p.gety()] == MASKED) { // 是未展开雷区的边界
						sop.insert(p);			// 加入位置集合
					}
				}
				if (sop.size() > 0) {			// 找到一个子雷区
					S_M_F f(sop, m[i][j] - cnt_marked_near(m, i, j));	// 构造子雷区
					set < S_M_F> ::iterator p = s.find(f);
					if (p == s.end()) {			// 如果子雷区集合中没有该子雷区就加入它
						add_and_splite_the_set_of_sub_mine_field(s, f);	// 加入并拆分子雷区集合
					}
				}
			}
		}
	}
}

void converge(set< S_M_F >& s)
{	// 用夹逼法搜索子雷区
	// 例如有一个子雷区: 位置集的大小为6, 雷数为2 则:
	//     Less_Than:	任意3, 4, 5 个位置集中最多含有2个雷 (共有6-2-1种组合)
	//     More_Than:	任意5个位置集中最少含有1个雷		(共有 2-1 种组合)
	vector< S_M_F > Buff_SMF(200);
	set < S_M_F > Set_Less_Than, Set_More_Than, Set_Field;
	vector <Point> t, v;
	set < S_M_F > ::iterator _i = s.begin();
	for (; _i != s.end(); ++_i) { 
		if ((*_i).first.size() > (*_i).second && (*_i).second > 0 && (*_i).first.size() > 2) {
			t.clear(); v.clear();
			set_to_vector((*_i).first, t);		// 将集合转化成向量以便利用组合算法
			S_O_P p;
			for (int i = (*_i).second + 1; i < t.size(); ++i) {			// 加入Set_Less_Than集
				v.clear();
				combi(t, i, v);			// 将所有的组合都放到向量v中
				for (int j = 0; j < v.size(); ) {
					p.clear();
					for (int k = 0; k < i; k++)
						p.insert(v[j++]);
					Set_Less_Than.insert(S_M_F(p, (*_i).second));
				}
			}
			int n;
			for (i = (*_i).second - 1, n = t.size() - 1; i > 0; --i) {	// 加入Set_More_Than集
				v.clear();
				combi(t, n, v);
				for (int j = 0; j < v.size(); ) {
					p.clear();
					for (int k = 0; k < n; k++)
						p.insert(v[j++]);
					Set_More_Than.insert(S_M_F(p, i));
				}
				--n;
			}
		}
	} 
	// 交集必然都是子雷区
	set_intersection(Set_Less_Than.begin(), Set_Less_Than.end(), Set_More_Than.begin(), Set_More_Than.end(), Buff_SMF.begin());
	vector< S_M_F >::iterator _i_v = Buff_SMF.begin();
	for (; _i_v != Buff_SMF.end(); ++_i_v) {
		if ((*_i_v).first.size() != 0) {
			Set_Field.insert(*_i_v);
		}
	}
	// 将交集Set_Field中的子雷区全部都加入并划分子雷区集合s
	_i = Set_Field.begin();
	for (; _i != Set_Field.end(); ++_i) {
		add_and_splite_the_set_of_sub_mine_field(s, *_i);
	}
}

Point search_for_min_possible_rand_point(int **m)
{	// 搜索是雷的可能性最小的随机位置
	double WHOLE_POSSIBLITY = (1.0 * m_num) / (m_row * m_col); // 全局是雷的可能性
	multimap < double, Point > Map_Backup_Point;
	// 遍历雷区获取候补位置集合
	for (int i = 1; i <= m_row; ++i) {
		for (int j = 1; j <= m_col; ++j) {
			if (m[i][j] == MASKED) { // 是未知格
				if (cnt_num_near(m, i, j) == 0) {
					Map_Backup_Point.insert(pair<double, Point>(WHOLE_POSSIBLITY, Point(i, j)));
					continue;
				}
				double posb = 0;	
				for (int k = 0; k < 8; ++k) { // 查看该位置是否属于子雷区, 即周围是否有数字格
					if (is_number(m[i + OFFSET[k].getx()][j + OFFSET[k].gety()])) {
						double c = cnt_masked_near(m, i + OFFSET[k].getx(), j + OFFSET[k].gety());
						double tp = 0; 
						if (0 < c) 
							tp = m[i + OFFSET[k].getx()][j + OFFSET[k].gety()] / c;
						if (posb < tp) posb = tp;
					}
				}
				if (0 < posb)  { // 在雷区边界上, 为最大概率
					Map_Backup_Point.insert(pair<double, Point>(posb, Point(i, j)));
				}
			}
		}
	}
	int number = Map_Backup_Point.count((*Map_Backup_Point.begin()).first);
	int r = rand() % number;
	multimap < double, Point > ::iterator p = Map_Backup_Point.begin();
	for (i = 0; i < r; ++i, ++p) NULL;
	return (*p).second;
}

void search_mine_and_nomine_to_set(int **m, const set < S_M_F >& s, S_O_P& Set_Mine, S_O_P& Set_Nomine)
{
	set < S_M_F > ::const_iterator _i = s.begin();
	for (; _i != s.end(); ++_i) {
		if ((*_i).first.size() == (*_i).second && (*_i).second != 0) { // 该子雷区中的所有位置都是雷
			S_O_P::const_iterator _i_p = (*_i).first.begin();
			for (; _i_p != (*_i).first.end(); ++_i_p) {
				Set_Mine.insert(*_i_p);
			}
		}
		else if ((*_i).first.size() != 0 && (*_i).second == 0) {		// 该子雷区中的所有位置都不是雷
			S_O_P::const_iterator _i_p = (*_i).first.begin();
			for (; _i_p != (*_i).first.end(); ++_i_p) {
				Set_Nomine.insert(*_i_p);
			}
		}
	}

	if (Set_Mine.size() == 0 && Set_Nomine.size() == 0)
		Set_Nomine.insert(search_for_min_possible_rand_point(m));
}

void make_command_list(list < pair < Point, int> >& List_Of_Command, S_O_P& SOP_Nomine, S_O_P& SOP_Mine)
{
	S_O_P ::iterator _i = SOP_Nomine.begin();
	for (; _i != SOP_Nomine.end(); ++_i) {
		List_Of_Command.push_back(pair < Point, int>(*_i, 0));
	}
	for (_i = SOP_Mine.begin(); _i != SOP_Mine.end(); ++_i) {
		List_Of_Command.push_back(pair < Point, int>(*_i, 1));
	}
}

void CMineCrackDlg::excute_the_command_list(const list< pair < Point, int> >& l)
{
	list < pair < Point, int> > ::const_iterator _i = l.begin();
	for (; _i != l.end(); ++_i) {
		switch ((*_i).second) {
		case 0:
			::PostMessage( hWinMineWnd, WM_LBUTTONDOWN, MK_LBUTTON,
				MAKELPARAM(12 + (*_i).first.getx() * BMPWIDTH + 8, 55 + (*_i).first.gety() * BMPHEIGHT + 8) );
			::PostMessage( hWinMineWnd, WM_LBUTTONUP, 0,
				MAKELPARAM(12 + (*_i).first.getx() * BMPWIDTH + 8, 55 + (*_i).first.gety() * BMPHEIGHT + 8) );
			break;
		case 1:
			::PostMessage( hWinMineWnd, WM_RBUTTONDOWN, MK_RBUTTON,
				MAKELPARAM(12 + (*_i).first.getx() * BMPWIDTH + 8, 55 + (*_i).first.gety() * BMPHEIGHT + 8) );
		}
	}
}

bool is_win(int **m)
{
	for (int i = 1; i <= m_row; ++i) {
		for (int j = 1; j <= m_col; ++j) {
			if (m[i][j] == MASKED) return false;
		}
	}
	return true;
}

/*
void excute_the_command_list(int ** m, const list< CMD >& l)
{
	list < CMD > ::const_iterator _i = l.begin();
	for (; _i != l.end(); ++_i) {
		switch ((*_i).second) {
		case LEFT:
			m.dig((*_i).first.getx(), (*_i).first.gety(), OPEN);
			break;
		case RIGHT:
			m.dig((*_i).first.getx(), (*_i).first.gety(), SIGN);
			break;
		}
	}
}
*/
/*

void main()
{
		// 定义变量
		srand(time((time_t*) NULL));
		set< S_M_F > Set_Of_Sub_Mine_Field;	// 子雷区集合
		S_O_P Set_Mine, Set_Nomine;			// 雷和非雷位置集
		list < pair < Point, int> > List_Of_Command;		// 命令列表

		////////////////////////////
		// 这里填加捕获窗口函数, 获取雷区参数m_row, m_col, m_num
		////////////////////////////

		// 根据雷区参数创建雷区
		int **m = new int *[m_row + 2];
		for (int i = 0; i < m_row + 2; ++i) {
			m[i] = new int[m_col + 2];
		}

	while (true) {
		/////////////////////////////
		// 这里填加窗口扫描函数, 获取雷区信息, 即各方格的值
		/////////////////////////////

		// 初始化变量
		Set_Of_Sub_Mine_Field.clear();
		Set_Mine.clear(); Set_Nomine.clear(); List_Of_Command.clear();

		//  算法部分
		make_set_of_sub_mine_field(m, Set_Of_Sub_Mine_Field);
		converge(Set_Of_Sub_Mine_Field);
		search_mine_and_nomine_to_set(m, Set_Of_Sub_Mine_Field, Set_Mine, Set_Nomine);
		make_command_list(List_Of_Command, Set_Nomine, Set_Mine);

		// 执行部分
//		excute_the_command_list(m, List_Of_Command);
	}
}
*/
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

BOOL CMineCrackDlg::ScanTheMap(int **m_map)
{
	int i, j, ret;
	CPoint pt; 
	BOOL finish = FALSE;

	for (i=1; i<= m_nHeight; i++)
	{
		for(j=1; j<= m_nWidth; j++)
		{
			pt = CPoint( 12 + j*BMPWIDTH, 55 + i*BMPHEIGHT + 7 );	//坐标从上至下；
			ret = GetColorSample(pt);
			switch(ret) 
			{
			case 0:	m_map[i][j] = 0; break;
			case 1:	m_map[i][j] = 1; break;
			case 2:	m_map[i][j] = 2; break;
			case 3:	m_map[i][j] = 3; break;
			case 4:	m_map[i][j] = 4; break;
			case 5:	m_map[i][j] = 5; break;
			case 6:	m_map[i][j] = 6; break;
			case 7:	m_map[i][j] = 7; break;
			case 8:	m_map[i][j] = 8; break;
			case 9:	m_map[i][j] = 0; break;
			case 10: finish = TRUE; break;
			case 11: finish = TRUE; break;
			case 12: finish = TRUE; break;
			case 13: m_map[i][j] = MASKED; break;
			case 14: m_map[i][j] = MARKED; break;	
			case 15: m_map[i][j] = MASKED; break;
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

void CMineCrackDlg::OnBtsweepmine()  // 我的主程序算法填加在这里
{
try {
	// 定义变量
	srand(time((time_t*) NULL));
	set< S_M_F > Set_Of_Sub_Mine_Field;	// 子雷区集合
	S_O_P Set_Mine, Set_Nomine;			// 雷和非雷位置集
	list < pair < Point, int> > List_Of_Command;		// 命令列表

	if ( ! m_bStart )
	{
		MessageBox("先点击开始按钮！", NULL, MB_OK | MB_ICONINFORMATION);
		return;
	}

	if(::IsIconic(hWinMineWnd))	::ShowWindow(hWinMineWnd,SW_SHOWNORMAL);
	::BringWindowToTop(hWinMineWnd);
	::SetActiveWindow(hWinMineWnd);
	
	// 根据雷区参数创建雷区
	int **m = new int *[m_row + 2];
	for (int i = 0; i < m_row + 2; ++i) {
		m[i] = new int[m_col + 2];
	}

	while (true) {
		/////////////////////////////
		// 这里填加窗口扫描函数, 获取雷区信息, 即各方格的值
		if (!ScanTheMap(m)) { // 失败了
			MessageBox("扫雷失败了，GAME OVER！", NULL, MB_OK | MB_ICONINFORMATION);
			break;
		}
		if (is_win(m)) {
			MessageBox("恭喜啊，扫雷成功了！", NULL, MB_OK | MB_ICONINFORMATION); 		
			break;
		}
		/////////////////////////////

		// 初始化变量
		Set_Of_Sub_Mine_Field.clear();
		Set_Mine.clear(); Set_Nomine.clear(); List_Of_Command.clear();

		//  算法部分
		make_set_of_sub_mine_field(m, Set_Of_Sub_Mine_Field);
		converge(Set_Of_Sub_Mine_Field);
		search_mine_and_nomine_to_set(m, Set_Of_Sub_Mine_Field, Set_Mine, Set_Nomine);
		make_command_list(List_Of_Command, Set_Nomine, Set_Mine);

		// 执行部分
		excute_the_command_list(List_Of_Command);
	}
} catch(...) { ; }
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


