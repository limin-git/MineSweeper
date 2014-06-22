#ifndef CLEAR_MINES_H
#define CLEAR_MINES_H
//  程序说明：
//	开始时用键盘输入 1 2 3 选择等级
//	然后输入 行 列 鼠标(L代表左键，R代表右键，D代表双键)

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <stack>
#include <cassert>
#include "Point.h"
using namespace std;

typedef set< Point > S_O_P;			// 格的位置的集合(set of position)
typedef int N_O_M;					// 雷数(number of mines)
typedef pair< S_O_P, N_O_M > S_M_F;	// 子雷区(sub mine field)

enum STATE_GAME { CONTINUE, FAILED, SUCCESS };
const string STATE_INFOR[] = { "游戏进行中", "真不幸, 又失败了", "恭喜你, 扫雷成功了" };
enum CONTENT { BLANK, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, MINE, MASKED, MARKED, QUESTION, BLAST, ERROR, NONE};
enum DETECT { OPEN, SIGN };	// 探测命令: 打开, 标记 
const string STR_SHELL[] = 
{
	"□",	"①",	"②",	"③",	"④",	"⑤",	"⑥",	"⑦",	"⑧",
	"●",	"■",	"★",	"？",	"¤",	"×", " "
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

class MineBox  
{
	CONTENT real, appear;
	friend class MineField;
	friend void make_set_of_sub_mine_field(MineField& m, set< S_M_F >& s);
	friend STATE_GAME game_state(const MineField&);
	friend Point search_for_min_possible_rand_point(MineField& m);
public:
	MineBox()						{ real = BLANK; appear = MASKED; }
	int is_number()					{ return ONE <= appear && appear <= EIGHT; } // 是数字格吗
private:
	MineBox& setm()					{ real = MINE; return *this; }
	MineBox& set_real(CONTENT _r)	{ real = _r; return *this; }
	void action(DETECT);
	void print();
};

void MineBox::print()
{
	cout << STR_SHELL[appear];
}

void MineBox::action(DETECT d)
{
	switch (d) {
	case OPEN:
		if (appear = MASKED) { 
			if (real == MINE) {
				appear = BLAST;
			}
			else {
				appear = real; 
			}
		}
		break;
	case SIGN:
		if		(appear = MASKED)		{ appear = MARKED; }
		else if (appear = MARKED)		{ appear = QUESTION; }
		else if (appear = QUESTION)	{ appear = MASKED; }
		break;
	default: ;
	}
}

class MineField
{
	MineBox **field;
	static int row, col, num;
	friend void make_set_of_sub_mine_field(MineField& m, set< S_M_F >& s);
	friend STATE_GAME game_state(const MineField&);
	friend Point search_for_min_possible_rand_point(MineField& m);
public:
	~MineField();
	MineField(int, int, int);
	void dig(int, int, DETECT);		// 根据坐标和命令挖一次雷
	void print();
	void open_near(int, int);
	int  cnt_masked_near(int, int);		// field[x][y]这个位置周围有几个未知格
	int  cnt_num_near(int, int);		// field[x][y]这个位置周围有几个数字格
	int  cnt_marked_near(int, int);		// field[x][y]这个位置周围有几个标记格
};
int MineField::row = 0;
int MineField::col = 0;
int MineField::num = 0;

int MineField::cnt_masked_near(int x, int y) 
{
    int i = 0;
    int c = 0;

    // field[x][y]这个位置周围有几个未知格
	for (i = 0, c = 0; i < 8; ++i) {
		if (field[x + OFFSET[i].getx()][y + OFFSET[i].gety()].appear == MASKED) ++c;
	}
	return c;
}
int MineField::cnt_num_near(int x, int y) 
{
    int i = 0;
    int c = 0;

    // field[x][y]这个位置周围有几个未知格
	for (i = 0, c = 0; i < 8; ++i) {
		if (field[x + OFFSET[i].getx()][y + OFFSET[i].gety()].is_number()) ++c;
	}
	return c;
}
int MineField::cnt_marked_near(int x, int y) 
{
    int i = 0;
    int c = 0;

    // field[x][y]这个位置周围有几个未知格
	for (i = 0, c = 0; i < 8; ++i) {
		if (field[x + OFFSET[i].getx()][y + OFFSET[i].gety()].appear == MARKED) ++c;
	}
	return c;
}

void MineField::dig(int _r, int _c, DETECT d)
{
	assert(_r <= row && _c <= col);
	field[_r][_c].action(d);
	if (field[_r][_c].real == BLANK) {
		open_near(_r, _c);
	}
}

MineField::MineField(int _r, int _c, int _n)
{
	row = _r; col = _c; num = _n;
	// 在四周加一圈, 便于边界处理
	field = new MineBox* [row + 2];
	for (int i = 0; i < row + 2; ++i)
		field[i] = new MineBox[col + 2];
	// 外围全部展开(因为OpenNear可能会将边界未展开的区域入栈)
	for (int i = 0; i < row + 2; i++) {
		field[i][0].appear = field[i][col + 1].appear = BLANK; 
	}
	for (int i = 0; i< col + 2; i++) {
		field[0][i].appear = field[row + 1][i].appear = BLANK;
	}
	// 随机产生n个为mine
	srand(time((time_t*)NULL));
	for (int i = 0; i < num; ) {
		int j = rand() % row + 1; 
		int k = rand() % col + 1;
		if (field[j][k].real != MINE) { 
			field[j][k].setm();
			++i;
		}
	}
	// 计算四周mine的个数
	for (int i = 1; i <= row; i++) 
		for (int j = 1; j <= col; j++)
			if (field[i][j].real != MINE) {
				int k = 0;
				for (int m = -1; m <= 1; ++m)
					for (int n = -1; n <= 1; ++n)
						if (field[i + m][j + n].real == MINE) ++k;
				field[i][j].real = (CONTENT)k;
			}
}
void MineField::print()
{
	system("cls");
	for (int i = 1; i <= row; ++i) {
		for (int j = 1; j <= col; ++j)
			field[i][j].print();
		cout << endl;
	}
	cout << endl;
}

void MineField::open_near(int _r, int _c)
{
	assert (_r <= row && _c <= col);
	pair<int, int> pos(_r, _c);
	stack< pair<int, int> > stack_pos;
	stack_pos.push(pos);
	while (!stack_pos.empty()) {
		pair<int, int> t = stack_pos.top();
		stack_pos.pop();
		field[t.first][t.second].appear = BLANK;
		for (int i = -1; i <= 1; ++i)
			for (int j = -1; j <= 1; ++j) {
					if (field[t.first + i][t.second + j].appear == MASKED) {
						if (field[t.first + i][t.second + j].real == BLANK)
							stack_pos.push(pair<int, int>(t.first + i, t.second + j));
						else
							field[t.first + i][t.second + j].appear = field[t.first + i][t.second + j].real;
					}
			}
	}
}

MineField::~MineField()
{
	for (int i = 0; i< row + 2; i++) 
		delete field[i];
	delete[] field;
}
#endif