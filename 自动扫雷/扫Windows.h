/*
  程序说明：
	开始时用键盘输入 1 2 3 选择等级
	然后输入 行 列 鼠标(L代表左键，R代表右键，D代表双键)

*/
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <stack>
using namespace std;
ifstream _in;
ofstream _out;
#define	 MINE_MATRIX_FILE "c:\\MineMatrix.txt"
#define  COMMAND_LIST_FILE "c:\\CommandList.txt"
#define MAXSIZE 100
bool	ISCHANGED = false;
int		COUNT_LEFT_CLICK=0, COUNT_RIGHT_CLICK=0, COUNT_DOUBLE_CLICK=0, COUNT_TOTAL_CLICK=0;
#define COUNT_RESET { COUNT_LEFT_CLICK=0; COUNT_RIGHT_CLICK=0; COUNT_DOUBLE_CLICK=0; COUNT_TOTAL_CLICK=0; }
#define CHANGE { ISCHANGED = true; }
#define RESET  { ISCHANGED = false; }
#define INCDENT(n) { for (int i=0; i<n; i++) cout << '\t'; }
#define _C_I const_iterator
#define _I   iterator
class	POINT;
enum	Mouse;
typedef vector <POINT> POINT_BUFF;
typedef stack <POINT> POINT_STACK;
typedef set <POINT> POINT_SET;
typedef pair < set<POINT>, int > BLOCK;
typedef set <BLOCK> BLOCK_SET;
typedef pair <double, POINT> POSSIBLE;
typedef set <POSSIBLE> POSSIBLE_SET;
typedef pair <POINT, Mouse> COMMAND;
typedef list <COMMAND> COMMAND_LIST;
typedef vector <POINT> POINT_BUFF;
enum { WHITE = 0, MASK = 9, MINE = 10, MARK = 11, ERROR = 12 };
enum Mouse {LEFT, RIGHT, LEFTRIGHT};
const string MineCover[] = 
{
	"□",	"①",	"②",	"③",	"④",	"⑤",	
	"⑥",	"⑦",	"⑧",	"■",	"●",	"★",	"×"
};
class Error
{	// 用于调试错误, 用try-catch块把怀疑有错误的代码封闭起来 (非常适用于系统出错)
public:
	Error(const string& s) { cout << "error: " << s << endl; }
};
class Time
{
	long t;
public:
	Time()	{ t = clock(); }
	~Time() { cout << endl << "used " << (clock() - t) / 1000.0 << " seconds. " << endl; }
};
class POINT
{
	int x, y;
	friend class MineMatrix;
	friend class AutoClearMines;
public:
	int GetX() const					{ return x; }
	int GetY() const					{ return y; }
	void Set(int _x=0, int _y=0)		{ x = _x; y = _y; }
	POINT(int _x=0, int _y=0)			{ x = _x; y=_y; }
	POINT& operator +=(const POINT& p)	{ x += p.x; y += p.y; return *this; }
	POINT& operator =(int _x)			{ x = y = _x; return *this; }
};
POINT OffSet[] = 
{	// 某个mine的周围8个位置的偏移量, 用for循环即可访问所有8个位置
	POINT(-1, -1),	 POINT(-1, 0),	 POINT(-1, 1),	 POINT(0, -1),	
	POINT(0, 1),	 POINT(1, -1),	 POINT(1, 0),	 POINT(1, 1)
};

istream& operator >> (istream& in, POINT& p) {
	int x, y;
	in >> x >> y;
	p.Set(x, y);
	return in;
}

bool operator <(const POINT& p1, const POINT& p2) 
{	// set 所用的比较函数一
	return ((p1.GetX() <= p2.GetX())?((p1.GetX()<p2.GetX())?(true):((p1.GetY()<p2.GetY())?(true):(false))):(false)); 
}

bool operator ==(const POINT& p1, const POINT& p2)
{	// set 所用的比较函数二
	return (p1.GetX() == p2.GetX() && p1.GetY() == p2.GetY()); 
}

POINT operator +(const POINT& p1, const POINT& p2) 
{ return POINT(p1.GetX()+p2.GetX(), p1.GetY()+p2.GetY()); }

class Mine  
{
	bool mask, mark;// mask: 是否展开, mark: 是否标记为mine
	int n;			// 0: 空白, 1-8: mine的个数, 9: 未探测, 10: mine, 11: 标记为mine, 12: 判断错误
	friend class MineMatrix;
public:
	Mine(int _n=0)			{ mask = true; mark = false; n=_n; }
	Mine& Set(int _n)		{ n = _n; return *this; }
	bool IsMasked() const	{ return mask; }
	bool IsMine() const		{ return n == MINE; }
	bool IsMarked() const	{ return mark; }
	bool IsWhite() const	{ return n == WHITE; }
	bool IsNumber() const	{ return (1<=n && n<=8); }
	int  GetN() const		{ return n; }
	Mine& RemoveMask()		{ mask = false; return *this; }
	Mine& RemoveMark()		{ mark = false; return *this; }
	Mine& Mask()			{ mask = true; return *this; }
	Mine& Mark()			{ mark = true; return *this; }
	void Print(bool = false) const; // true时, 用于GameOver(success or failed), 不显示未展开区域
};
void Mine::Print(bool end) const 
{
	if (end) { // 除未展开区域外全部显示
		if (mark) {
			if (n == MINE) { cout << MineCover[MARK]; }
			else { cout << MineCover[ERROR]; }
		}
		else if (n == MINE || n == ERROR) { cout << MineCover[n]; }
		else { cout << MineCover[MASK]; }
		return;
	}
	if (mark) { // 显示挖一次后的状态
		cout << MineCover[MARK];
	}
	else if (mask) { cout << MineCover[MASK]; }
	else { cout << MineCover[n]; }
}
class MineMatrix
{
	int r, c, n;	// r: mine区的行, c: mine区的列, n: mine数
	Mine **m;		// 指向mine区的指针(动态创建mine区)
	friend class AutoClearMines;
public:
	~MineMatrix();
	MineMatrix(int = 1);
	bool CreateFromFile(const char*);					// 从文件创建雷区
	bool Dig(const POINT& p, const Mouse&);			// 核心函数
	bool Dig(const COMMAND& c)						{ return Dig(c.first, c.second); }
	void SetLevel(int, int, int);
	void SetLevel(int Level = 1);
	bool OpenNear(const POINT& p);
	Mine& Get(int x, int y)							{ return m[x][y]; }
	Mine& Get(const POINT& p)						{ return m[p.x][p.y]; }

	void Print(bool = false) const;					// true时, 用于游戏结束时显示
	void Output(const char*);						// 输出到文件
	bool IsWin() const;								// 是否成功
	bool IsGameOver() const;						// 是否失败
	bool IsWhiteOpen() const;						// mine区中是否出现白色区域
	int  HowManyMinesNear(const POINT& p) const;	// 四周mine的个数
	int  HowManyMarkedNear(const POINT& p) const;	// 四周标记为mine的个数
	int  HowManyMarkedWhole() const;				// mine区中标记为mine的个数
	int  HowManyMaskedWhole() const;				// mine区中未展开的个数
	int  HowManyMaskedNear(const POINT&) const;		// 四周未展开的个数
	int GetR() const 								{ return r; }
	int GetC() const 								{ return c; }
	int GetN() const								{ return n; }
	Mine& Get(int x, int y)	const					{ return m[x][y]; }
	Mine& Get(const POINT& p) const 				{ return m[p.x][p.y]; }
};
void MineMatrix::Output(const char *f)
{
	ofstream out(f, ios::out);
	for (int i=1; i<=r; ++i) {
		for (int j=1; j<=c; j++) {
			if (m[i][j].IsMarked()) out << MARK << '\t';
			else if (m[i][j].IsMasked()) out << MASK << '\t';
			else
				out << m[i][j].n << '\t';
		}
		out << endl;
	}
	out.close();
}
bool MineMatrix::CreateFromFile(const char* f)
{	
	for (int i=0; i<r+2; i++) 
		delete m[i];
	delete[] m;

	ifstream in(f, ios::in);
	in >> r >> c >> n;
	if (r == c && c == n && n == 0) return false;
	m = new Mine* [r+2];
	for (i=0; i<r+2; ++i)
		m[i] = new Mine[c+2];
	for (i=0; i<r+2; i++) { Get(i, 0).RemoveMask().Set(0);	Get(i, c+1).RemoveMask().Set(0); }
	for (i=0; i<c+2; i++) { Get(0, i).RemoveMask().Set(0);	Get(r+1, i).RemoveMask().Set(0); }
	
	int ch;
	for (i=1; i<=r; i++) {
		for (int j=1; j<=c; j++) {
			in >> ch;
			m[i][j].Set(ch);
			switch (ch) {
			case 0:	case 1:	case 2:	case 3:
			case 4:	case 5:	case 6:	case 7:	case 8:
				m[i][j].mask = false;
				break;
			case 9:
				m[i][j].mask = true;
				break;
			case 10:
				m[i][j].mask = false;
				m[i][j].Set(MINE);
				break;
			case 11:
				m[i][j].mask = true;
				m[i][j].mark = true;
				break;
			case 12:
			default:
				m[i][j].Set(ERROR);
			}
		}
	}
	if (IsGameOver() || IsWin()) return false;
	return true;
}
bool MineMatrix::Dig(const POINT& p, const Mouse& Mouse)
{
	Mine& m = Get(p);
	switch (Mouse) {
	case LEFT:		
		if(m.mask && !m.mark)	// 未展开且未标记为mine
			if (m.IsMine()) { m.mask = false; m.n = ERROR; CHANGE; return false; } // 触雷
			else if (m.IsWhite()) { return OpenNear(p); }	// 展开白色区域
			else { m.mask = false; CHANGE; return true; }	// 为数字
		return true;
		break;
	case RIGHT:
		if (m.mask) {	// 未展开或已标记
			if (m.mark) {m.mark = false; }
			else { m.mark = true; }
			CHANGE;
		}
		return true;
		break;
	case LEFTRIGHT:
		if (!m.mask && m.n != 0 && HowManyMarkedNear(p) == m.n) 
			return OpenNear(p); 
		return true;
		break;
	default:
		throw Error("key error");
	}
}
bool MineMatrix::OpenNear(const POINT& p)
{
	POINT_STACK s;
	s.push(p);
	while (!s.empty()) {
		POINT pt = s.top();
		s.pop();
		Get(pt).mask = false;
		for (int i=0; i<8; i++) {
			Mine& m = Get(pt + OffSet[i]);
			if (m.mask && !m.mark) {
				if (m.n == WHITE) { s.push(pt + OffSet[i]); }
				else if (m.n == MINE) { m.n =ERROR; CHANGE; return false; }
				else { m.mask = false; CHANGE; }
			}
		}
	}
	return true;
}

int MineMatrix::HowManyMaskedNear(const POINT& p ) const
{
	for (int i=0, k=0; i<8; i++) {
		Mine& m = Get(p + OffSet[i]);
		if (m.mask && !m.mark) k++;
	}
	return k;
}
bool MineMatrix::IsGameOver() const
{
	for (int i=1; i<=r; i++) 
		for (int j=1; j<=c; j++)
			if (m[i][j].n == ERROR) return true;
	return false;
}
int MineMatrix::HowManyMaskedWhole() const
{
	for (int i=1, k=0; i<=r; i++) 
		for (int j=1; j<=c; j++) 
			if (m[i][j].mask) k++;
	return k;
}
int MineMatrix::HowManyMarkedWhole() const
{
	for (int i=1, k=0; i<=r; i++) {
		for (int j=1; j<=c; j++) {
			if (m[i][j].mark) k++;
		}
	}
	return k;
}
bool MineMatrix::IsWhiteOpen() const
{
	for (int i=1; i<=r; i++) 
		for (int j=1; j<=c; j++) 
			if (!m[i][j].mask && m[i][j].n == 0) 
				return true;
	return false;
}
MineMatrix::MineMatrix(int level) 
{
	switch (level) {
	case 1:	SetLevel( 9,  9, 10); break;
	case 2:	SetLevel(16, 16, 40); break;
	case 3:	SetLevel(16, 30, 99); break;
	case 4:	SetLevel(10, 10,  2); break;
	default:SetLevel( 9,  9, 10);
	}
}
bool MineMatrix::IsWin() const
{
	for (int i=1; i<=r; i++) {
		for (int j=1; j<=c; j++) {
			Mine& M = m[i][j];
			if (!(!M.mask || M.mark && M.n == MINE)) 
				return false;
		}
	}
	return true;
}
void MineMatrix::SetLevel(int _r, int _c, int _n)
{
	int i, j, k;
	r = _r; c = _c; n = _n;
	// 在四周加一圈, 便于边界处理
	m = new Mine* [r+2];
	for (i=0; i<r+2; ++i)
		m[i] = new Mine[c+2];
	// 外围全部展开(因为OpenNear可能会将边界未展开的区域入栈)
	for (i=0; i<r+2; i++) { Get(i, 0).RemoveMask().Set(0);	Get(i, c+1).RemoveMask().Set(0); }
	for (i=0; i<c+2; i++) { Get(0, i).RemoveMask().Set(0);	Get(r+1, i).RemoveMask().Set(0); }
	// 随机产生n个为mine
	srand(time((time_t*)NULL));
	for (i=0; i<n; ) {
		j = rand()%r+1; k = rand()%c+1;
		if (!Get(j, k).IsMine()) { 
			Get(j, k).Set(MINE); i++; 
		}
	}
	// 计算四周mine的个数
	for (i=1; i<=r; i++) 
		for (j=1; j<=c; j++)
			if (!m[i][j].IsMine()) 
				m[i][j].Set(HowManyMinesNear(POINT(i, j)));
}

MineMatrix::~MineMatrix()
{
	for (int i=0; i<r+2; i++) 
		delete m[i];
	delete[] m;
}
void MineMatrix::Print(bool end) const
{
//	system("cls");
	int i, k=1;
	INCDENT(1); cout << "  ";
	for (i=1; i<=c; i++) {
		cout.width(2); cout.setf(ios::left, ios::adjustfield);
		cout << i;
	}
	cout << endl;
	for (i=1; i<=r; i++) {
		INCDENT(1); 
		cout.width(2);
		cout << k++;
		for (int j=1; j<=c; j++) {
			Mine& M = m[i][j];
			if (end && (M.IsMine() || M.mask || M.n == ERROR))
					M.Print(true);
			else 
				M.Print(false); 
		}
		cout << endl;
	}
}
int MineMatrix::HowManyMinesNear(const POINT& p) const
{
	for (int i=0, k=0; i<8; i++) 
		if (Get(p + OffSet[i]).IsMine()) k++;
	return k;
}
int MineMatrix::HowManyMarkedNear(const POINT& p) const
{
	for (int i=0, k=0; i<8; i++)
		if (Get(p + OffSet[i]).mark) k++; 
	return k;
}

void DelWhiteSpace(string& s)
{ while (s[0] == ' ' || s[0] == '\t') s.erase(0, 1); }
void Input(POINT& p, Mouse& mouse, const MineMatrix& matrix)
{
	string s;
	char *buff = new char[MAXSIZE];
	cout << endl << "PosX, PosY, Mouse: " ;
	cin >> p;
	while (!((1 <= p.GetX() && p.GetX() <= matrix.GetR()) &&
			 (1 <= p.GetY() && p.GetY() <= matrix.GetC())))
		cin >> p;
	cin.getline(buff, MAXSIZE);
	s.assign(buff);
	DelWhiteSpace(s);
	if (s.size() >=1)
		switch (s.substr(0, 1).c_str()[0]) {
		case 'l':	case 'L': mouse = LEFT; break;
		case 'r':	case 'R': mouse = RIGHT; break;
		case 'd':	case 'D': mouse = LEFTRIGHT; break;
		default: mouse = LEFT;
		}
	else { mouse = LEFT; }
}

bool operator ==(const POSSIBLE& p1, const POSSIBLE& p2)
{ return (p1.first == p2.first && p1.second == p2.second); }

bool operator < (const BLOCK& b1, const BLOCK& b2) 
{ return (b1.first < b2.first); }

bool operator == (const BLOCK& b1, const BLOCK& b2)
{ return (b1.first == b2.first); }

// 输出到文件只看这三个函数即可
void output(ostream& out, Mouse& m) 
{
	switch (m) {
	case LEFT: out << "L"; break;
	case RIGHT: out << "R"; break;
	case LEFTRIGHT: out << "D"; break;
	}
}
void output(ostream& out, POINT& p)
{
	if (p.GetX() >9) out << p.GetX() << " ";
	else out << p.GetX() << "  ";
	if (p.GetY() >9) out << p.GetY() << " ";
	else out << p.GetY() << "  ";
}
void output(ostream& out, COMMAND& c)
{ 
	output(out, c.first);
	output(out, c.second);
}

void output(ostream out, COMMAND_LIST& cl)
{
	COMMAND_LIST::_I p = cl.begin(), q = p;
	while (p != cl.end()) { q = p; p++; }
	p = cl.begin();
	for (; p != q; ++p) {
		output(out, *p);
		out << endl;
	}
	output(out, *p);
}
ostream& operator << (ostream& oo, const POINT& p)
{
	return oo << '(' << p.GetX() << ' ' << p.GetY() << ')';
}
ostream& operator << (ostream& oo, const POINT_SET& ps)
{
	POINT_SET::_C_I p = ps.begin();
	oo << "( " ;
	for (; p != ps.end(); ++p) {
		oo << *p << ' ';
	}
	return oo << ')';
}
ostream& operator << (ostream& oo, const BLOCK& b)
{
	return oo << "〖 " << b.first << ',' << b.second << " 〗" ;
}
ostream& operator << (ostream& oo, const BLOCK_SET& bs) 
{
	BLOCK_SET::_C_I p = bs.begin();
	for (; p != bs.end(); ++p) {
		oo << *p << endl;
	}
	return oo;
}