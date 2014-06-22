#ifndef CLEAR_MINES_H
#define CLEAR_MINES_H
//  程序说明：
//	开始时用键盘输入 1 2 3 选择等级
//	然后输入 行 列 鼠标(L代表左键，R代表右键，D代表双键)

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>
#include <stack>
#include <cassert>
using namespace std;

const string STATE_INFOR[] = { "游戏进行中", "真不幸, 又失败了", "恭喜你, 扫雷成功了" };
const string outside[] = 
{
	"□",	"①",	"②",	"③",	"④",	"⑤",	"⑥",	"⑦",	"⑧",
	"●",	"■",	"★",	"¤",	"×", " "
};
enum Button { Left, Right };
typedef pair< int, Button> Cmd;

const int m_row		= 16;
const int m_col		= 30;
const int m_num		= 99;
const int MINE		= 9;	
const int MASKED	= 10;
const int MARKED	= 11;
const int BLAST		= 12;
vector< int > mf;	// 雷区mine field
vector< int > offset(8);

bool is_range_in(int i, int F, int L)
{
	return (F <= i && i < L);
}
void create_mine_field(vector< int >& u)
{
    // u为用户看到的雷区, m 为雷区建造者看到的雷区
	int sz = m_row * m_col;
	mf.resize(sz);
	u.resize(sz);
	for (int i = 0; i < sz; ++i) {
		u[i] = MASKED;
		mf[i] = 0;
	}

	int t[] = { -m_col - 1,		-m_col,		-m_col + 1,
				-1,							1, 
				m_col - 1,		m_col,		m_col + 1 };
	offset.assign(t, t + 8); 

	int cnt = m_num, k;
	while (cnt) {
		k = rand() % sz;
		if (mf[k] != MINE) {
			mf[k] = MINE;
			--cnt;
		}
	}
	int cnt_mines;
	for (int i = 0; i < sz; ++i) {
		cnt_mines = 0;
		if (mf[i] == MINE) continue;
		for (k = 0; k < 8; k++) {
			int p = i + offset[k];
			if (!is_range_in(p, 0, sz)) continue; 
			if (mf[p] == MINE) cnt_mines++;
		}
		mf[i] = cnt_mines;
	}
}

bool dig(vector< int >& u, const Cmd& c)
{	// m 为用户看到雷区, r 为雷区建造者看到的雷区
	int p = c.first;
	switch (c.second) {
	case Left:
		if (u[p] != MASKED) return true;
		if (mf[p] == MINE) { u[p] = BLAST; return false; system("pause"); }
		else if (mf[p] == 0) { // open_near
			stack <int> s;
			s.push(p);
			while (!s.empty()) {
				int t = s.top();
				s.pop();
				u[t] = 0;
				for (int i = 0; i < 8; i++) {
					int tt = t + offset[i];
					if (!is_range_in(tt, 0, 480)) continue;
					if (u[tt] == MASKED) {
						if (mf[tt] == 0) s.push(tt);
						else u[tt] = mf[tt];
					}
				}
			}
		}
		else u[p] = mf[p];
		break;
	case Right:
		switch (u[p]) {
		case MASKED: u[p] = MARKED; break;
		case MARKED: u[p] = MASKED; break;	
		}
		break;
	}
	return true;
}

void print(const vector<int>& m, int c)
{
	for (int i = 0; i < m.size(); ++i) {
		cout << outside[m[i]];
		if ((i + 1) % c == 0 && i != 0) cout << endl;
	}
	cout << endl;
}

bool is_win(const vector< int >& m)
{
	for (int i = 0; i < m_col * m_row; ++i) {
		if (m[i] == MASKED) return false;
	}
	return true;
}
/*
void main()
{
	srand(time((time_t*) NULL));
	for (;;) {
		vector <int> user;
		create_mine_field(user);
		for (int i = 0; i < 10; i++) {
			system("cls");
			int k = rand() % (16 * 30);
			if (!dig(user, Cmd(k, Left))) break;
		}
		print(user, 30);
		system("pause");
	}
}
*/

#endif
