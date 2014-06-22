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

enum STATE_GAME { CONTINUE, FAIL, SUCCESS };
const string STATE_INFOR[] = { "游戏进行中", "真不幸, 又失败了", "恭喜你, 扫雷成功了" };
enum CONTENT { BLANK, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, MINE, MASKED, MARKED, QUESTION, BLAST, ERROR, NONE};
int m_row, m_col, m_num;

// 算法:
// 1. 定义雷区, 初始化变量
// 2. 扫描雷区, 获取雷区信息
// 3. 用集合算法make_set_of_sub_mine_field() 将雷区内边界化成子雷区集合
// 4. 用夹逼算法converge() 进一步拆分子雷区集合
// 5. 搜索子雷区集合search_mine_and_nomine_to_set中的雷和非雷并加入到相应集合
// 6. 用make_command_list通过雷和非雷集合得到命令列表
// 7. 执行命令列表excute_the_command_list

#define IS_NUMBER() ()
typedef set< Point > S_O_P;			// 格的位置的集合(set of position)
typedef int N_O_M;					// 雷数(number of mines)
typedef pair< S_O_P, N_O_M > S_M_F;	// 子雷区(sub mine field)
typedef int MineBox;				// 格
typedef int** MineField;			// 雷区
enum Button { LEFT, RIGHT, DOUBLE };
typedef pair < Point, Button > CMD;	// 命令
set< Point > Map_Backup_Point;		// 候补位置集合, 以POSB升序排序

set< S_M_F > Set_Of_Sub_Mine_Field;	// 子雷区集合
S_O_P Set_Mine, Set_Nomine;			// 雷和非雷位置集
list < CMD > List_Of_Command;		// 命令列表

// 定义:
// 
// 格(pane):						雷区中的一个方格
// 未知格(unknown pane):			没有打开与没有标记的格
// 标记格(marked pane):				标记为雷的格
// 问题格(questioned pane):			标记为问号的格
// 数字格(numbered pane):			内容为数字的格
// 雷区外边界(outter bolderline) :	一系列相邻的周围有未知格的数字格
// 雷区内边界(inner bolderline):	一系列相信的周围有数字格的未知格

// 子雷区(sub mine field):	由1-8个与一个数字格相邻的未知格组成的区域

bool is_number(MineBox b)
{
	return (1 <= b && b <= 8);
}
int cnt_masked_near(MineField& m, int x, int y) 
{	// field[x][y]这个位置周围有几个未知格
	for (int i = 0, c = 0; i < 8; ++i) {
		if (m[x + OFFSET[i].getx()][y + OFFSET[i].gety()] == MASKED) ++c;
	}
	return c;
}

int cnt_num_near(MineField& m, int x, int y)
{	// field[x][y]这个位置周围有几个未知格
	for (int i = 0, c = 0; i < 8; ++i) {
		if (is_number(m[x + OFFSET[i].getx()][y + OFFSET[i].gety()])) ++c;
	}
	return c;
}

int cnt_marked_near(MineField& m, int x, int y)
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
		if (Flag) 
			s.insert(f);
		else
			s_new.pop();
		_i = s_del.begin();
		for (; _i != s_del.end(); ++_i) {
			s.erase(*_i);
		}
		s_del.clear();
		f = s_new.top();
		s_new.pop();
	} while (!s_new.empty());
}

void make_set_of_sub_mine_field(MineField& m, set< S_M_F >& s)
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
{	// 用组合法搜索子雷区
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
	set_intersection(Set_Less_Than.begin(), Set_Less_Than.end(), Set_More_Than.begin(), Set_More_Than.end(), back_inserter(Buff_SMF));
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

Point search_for_min_possible_rand_point(MineField& m)
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

void search_mine_and_nomine_to_set(MineField& m, const set < S_M_F >& s, S_O_P& Set_Mine, S_O_P& Set_Nomine)
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

void make_command_list(list < CMD >& List_Of_Command, S_O_P& SOP_Nomine, S_O_P& SOP_Mine)
{
	S_O_P ::iterator _i = SOP_Nomine.begin();
	for (; _i != SOP_Nomine.end(); ++_i) {
		List_Of_Command.push_back(CMD(*_i, LEFT));
	}
	for (_i = SOP_Mine.begin(); _i != SOP_Mine.end(); ++_i) {
		List_Of_Command.push_back(CMD(*_i, RIGHT));
	}
}

/*
void excute_the_command_list(const list< CMD >& l)
{
	list < CMD > ::const_iterator _i = l.begin();
	for (; _i != l.end(); ++_i) {
		switch ((*_i).second) {
		case LEFT:
			::PostMessage( hWinMineWnd, WM_LBUTTONDOWN, MK_LBUTTON,
				MAKELPARAM(12 + (*_i).first.getx() * BMPWIDTH + 8, 55 + (*_i).first.gety() * BMPHEIGHT + 8) );
			::PostMessage( hWinMineWnd, WM_LBUTTONUP, 0,
				MAKELPARAM(12 + (*_i).first.getx() * BMPWIDTH + 8, 55 + (*_i).first.gety() * BMPHEIGHT + 8) );
			break;
		case: RIGHT:
			::PostMessage( hWinMineWnd, WM_RBUTTONDOWN, MK_RBUTTON,
				MAKELPARAM(12 + (*_i).first.getx() * BMPWIDTH + 8, 55 + (*_i).first.gety() * BMPHEIGHT + 8) );
		}
	}
}
*/
STATE_GAME game_state(const MineField& m)
{
	bool Flag = true;
	for (int i = 1; i <= m_row; ++i) {
		for (int j = 1; j <= m_col; ++j) {
			if (m[i][j] == MASKED) Flag = false;
			if (m[i][j] == BLAST || m[i][j] == MINE) return FAIL;
		}
	}
	if (Flag) 
		return SUCCESS;
	else
		return CONTINUE;
}

/*
void excute_the_command_list(MineField& m, const list< CMD >& l)
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

void main()
{
		// 定义变量
		srand(time((time_t*) NULL));
		set< S_M_F > Set_Of_Sub_Mine_Field;	// 子雷区集合
		S_O_P Set_Mine, Set_Nomine;			// 雷和非雷位置集
		list < CMD > List_Of_Command;		// 命令列表

		////////////////////////////
		// 这里填加捕获窗口函数, 获取雷区参数m_row, m_col, m_num
		////////////////////////////

		// 根据雷区参数创建雷区
		MineField m = new int *[m_row + 2];
		for (int i = 0; i < m_row + 2; ++i) {
			m[i] = new int[m_col + 2];
		}

	while (true) {
		/////////////////////////////
		// 这里填加窗口扫描函数, 获取雷区信息, 即各方格的值
		/////////////////////////////

		// 判断雷区的状态
		STATE_GAME state = game_state(m);
		if (state != CONTINUE) break; // 成功或失败则结束程序

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