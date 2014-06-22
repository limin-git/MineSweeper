#pragma warning(disable: 4786)
#include <iostream>
#include <ctime>
#include <list>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <map>
#include <stack>
#include "ClearMines.h"
#include "MyCombination.h"
using namespace std;

const int FAIL = 0;
const int SUCCESS = 1;
const int CONTINUE = 2;
typedef pair< set< int >, int > S_M_F;				// 子雷区sub mine field
typedef map< int, pair< set<int>, int > > I_S_M_F;	// 带索引的子雷区indexed sub mine field, 
void split_subminefields2(const vector< int >& m, I_S_M_F& s);

ostream& operator << (ostream& oo, const S_M_F& s)
{
	set < int >::const_iterator _i = s.first.begin();
	oo << "〖 "; 
	for (; _i != s.first.end(); ++_i) {
		oo << *_i << '\t';
	}
	return oo << "\t" << s.second << " 〗" << endl;
}
ostream& operator << (ostream& oo, const vector< S_M_F >& s)
{
	for (int i = 0; i < s.size(); ++i) 
		oo << s[i];
	return oo;
}
ostream& operator << (ostream& oo, const I_S_M_F& s)
{
	I_S_M_F::const_iterator it = s.begin();
	for (; it != s.end(); ++it) {
		oo << it->first << '\t' << it->second;
	}
	return oo;
}													// 关键码为 int型的的位置
int cnt_marked_near(const vector< int>& m, int i)
{
    int k = 0;
    int c = 0;

    // 方格i周围有几个未知格
	for (int k = 0, c = 0; k < 8; ++k) {
		if (!is_range_in(i + offset[k], 0, m_row * m_col)) continue;
		if (m[i + offset[k]] == MARKED) ++c;
	}
	return c;
}													// 值为 子雷区
void search_for_subminefields(const vector< int >& m, I_S_M_F& s)
{	
	set< int > t;
	for (int i = 0; i < m_row * m_col; ++i) {
		if (1 <= m[i] && m[i] <= 8) {	// 数字格
			for (int j = 0; j < 8; j++) {	// 搜索周围8个方格
				if (!is_range_in(i + offset[j], 0, m_row * m_col)) continue;
				if (m[i + offset[j]] == MASKED) t.insert(i + offset[j]);
			}
			if (t.size() > 0) {
				s[i] = S_M_F(t, m[i] - cnt_marked_near(m, i));
				t.clear();
			}
		}
	}
}

template< class T >
bool my_includes(const set< T >& a, const set< T >& b) 
{	// 如果集合A 真包含集合 B, 则返回true
	if (a.size() <= b.size()) return false;
	set< T >::const_iterator itb = b.begin();
	for (; itb != b.end(); ++itb) 
		if (a.find(*itb) == a.end()) return false;
	return true;
}

template< class T >
void my_erase(set< T >& a, set< T >& b)
{	// 从集合a中删除集合b中的所有元素
	set< T >::iterator it = b.begin();
	for (; it != b.end(); ++it) 
		a.erase(*it);
}
/*
void insert_to_subminefields(I_S_M_F& s, int index, const S_M_F& smf)
{
	I_S_M_F::iterator it = 
	stack< I_S_M_F::iterator > s_buff;
	s_buff.push(it);
	while (!s_buff.empty()) {
		I_S_M_F::iterator itp = s_buff.top();
		s_buff.pop();
		for (int i = 0; i < 8; ++i) {  // 只需查看关键码周围的8个数字格
			if (!is_range_in(itp->first + offset[i], 0, m_row * m_col)) continue;
			if (s.find(itp->first + offset[i]) == s.end()) continue;	// 该位置没有带索引的子雷区
			if (my_includes((itp->second).first, s[itp->first + offset[i]].first)) { // 有包含关系, 改变自身
				my_erase((itp->second).first, s[itp->first + offset[i]].first);
				(itp->second).second -= s[itp->first + offset[i]].second;
				s_buff.push(itp);
			}
			else if (my_includes(s[itp->first + offset[i]].first, s[itp->first].first)) { // 有被包含关系, 改变它
				my_erase(s[itp->first + offset[i]].first, (itp->second).first);
				s[itp->first + offset[i]].second -= (itp->second).second;
				s_buff.push(s.find(itp->first + offset[i]));				
			}
		}
	}
}
*/

void split_subminefields(const vector< int >& m, I_S_M_F& s)
{	// 对于有包含关系的子雷区进行拆分
	I_S_M_F::iterator it = s.begin(); 
	stack< I_S_M_F::iterator > s_buff;
	for (; it != s.end(); ++it) {
		s_buff.push(it);
		while (!s_buff.empty()) {
			I_S_M_F::iterator itp = s_buff.top();
			s_buff.pop();
			for (int i = 0; i < 8; ++i) {  // 只需查看关键码周围的8个数字格
				if (!is_range_in(itp->first + offset[i], 0, m_row * m_col)) continue;
				if (s.find(itp->first + offset[i]) == s.end()) continue;	// 该位置没有带索引的子雷区
				if (my_includes((itp->second).first, s[itp->first + offset[i]].first)) { // 有包含关系, 改变自身
					my_erase((itp->second).first, s[itp->first + offset[i]].first);
					(itp->second).second -= s[itp->first + offset[i]].second;
					s_buff.push(itp);
				}
				else if (my_includes(s[itp->first + offset[i]].first, s[itp->first].first)) { // 有被包含关系, 改变它
					my_erase(s[itp->first + offset[i]].first, (itp->second).first);
					s[itp->first + offset[i]].second -= (itp->second).second;
					s_buff.push(s.find(itp->first + offset[i]));				
				}
			}
		}
	}
}

template < class T > 
void set_to_vector(const set< T >& s, vector< T >& v)
{	// 将集合转化成向量以便利用组合算法
	set< T >::const_iterator it = s.begin();
	for (; it != s.end(); ++it) {
		v.push_back(*it);
	}
}
template < class T > 
void vector_to_set(vector< T >& s, set< T >& v)
{	// 将集合转化成向量以便利用组合算法
	for (int i = 0; i < v.size(); ++i)
		s.insert(v[i]);
}


template< class T >
void my_insert(set< T >& L, const set< T >& R)
{
	set< T >::const_iterator it = R.begin();
	for (; it != R.end(); ++it) {
		L.insert(*it);
	}
}

void combination(const vector< int >& m, I_S_M_F& s, set< int >& Mine, set< int >& Nomine)
{
	I_S_M_F::iterator it = s.begin();
	vector< int > buff, v; // buff 为组合缓冲区, v 为临时
	set< S_M_F > Less_Than, More_Than;
	set< int > p;		// p为临时集合
	for (; it != s.end(); ++it) { 
		int sz = (it->second).first.size();		// 子雷区方格的个数
		int num = (it->second).second;			// 子雷区中的雷数
		set_to_vector((it->second).first, v);
		if (sz > 2 && num != 0 && sz > num) {	// 参加组合的条件
			for (int i = num + 1; i < sz; ++i) {	
				v.clear();			//buff.resize(500);
				combi(v, i, buff);	// 将所有的组合都放到向量v中
				for (int j = 0; j < buff.size(); ++j) {
					p.clear();
					for (int k = 0; k < i; k++)
						p.insert(buff[j++]);
					Less_Than.insert(S_M_F(p, num));
				}
			}
			int n;
			for (int i = num - 1, n = sz - 1; i > 0; --i) {	// 加入Set_More_Than集
				v.clear();
				combi(v, n, buff);
				for (int j = 0; j < buff.size(); ) {
					p.clear();
					for (int k = 0; k < n; k++)
						p.insert(buff[j++]);
					More_Than.insert(S_M_F(p, i));
				}
				--n;
			}
		}
	}
	// 求交集
	vector< S_M_F > buff_s;
	set_intersection(Less_Than.begin(), Less_Than.end(), More_Than.begin(), More_Than.end(), back_inserter(buff_s));
	for (int i = 0; i < buff_s.size(); ++i) {
		int sz = buff_s[i].first.size();
		int num = buff_s[i].second;
		if (sz == num) {	// 加入雷集合
			my_insert(Mine, buff_s[i].first);
		}
		else if (num == 0) { // 加入非雷集合
			my_insert(Nomine, buff_s[i].first);
		}
		else {	// 由于交集个数比较少, 可以采用遍历的方式
			I_S_M_F::iterator it = s.begin();
			for (; it != s.end(); ++it) {
				if (my_includes((it->second).first, buff_s[i].first)) {
					(it->second).first.erase(buff_s[i].first.begin(), buff_s[i].first.end());
				}
				else if (my_includes(buff_s[i].first, (it->second).first)) {
					buff_s[i].first.erase((it->second).first.begin(), (it->second).first.end());
				}
			}
		}		
	}
}


bool is_number(const vector< int >& m, int i)
{
	return (1 <= m[i] && m[i] <= 8); 
}

int cnt_masked_near(const vector< int >& m, int i)
{
    int k = 0;
    int c = 0;

	for (k = 0, c = 0; k < 8; ++k) {
		if (!is_range_in(i + offset[k], 0, m_row * m_col)) continue;
		if (m[i + offset[k]] == MASKED) ++c;
	}
	return c;
}
int cnt_num_near(const vector< int >& m, int i)
{
    int k = 0;
    int c = 0;

	for (k = 0, c = 0; k < 8; ++k) {
		if (!is_range_in(i + offset[k], 0, m_row * m_col)) continue;
		if (is_number(m, i + offset[k])) ++c;
	}
	return c;
}

// 组合算法思想: 得到交集, 用交集中的元素进行拆分, 因为最多含有三个元素, 可以区别对待:
// 一个: 查看8个位置
// 二个: 同样查看8个位置　　　　 A　        □
// 三个: 判断是何种类型 如果 是□□□  或  A□B 则查看A, B 两个位置
//								 B		    □
// 对于子雷区中的集合包含交集中的集合的情况, 简单地将子雷区中的集合减去交集中的集合, 并将该集合入栈, 触发连锁反应
// 对于交集中的集合包含子雷区中的集合的情况, 将自身的裁剪, 然后...

int search_for_min_possible_rand_point(const vector< int >& m)
{	// 搜索是雷的可能性最小的随机位置
	double whole_possiblity = (1.0 * m_num) / (m_row * m_col); // 全局是雷的可能性
	multimap < double, int > Map_Backup;
	// 遍历雷区获取候补位置集合
	for (int i = 0; i < m_row * m_col; ++i) {
			if (m[i] == MASKED) { // 是未知格
				if (cnt_num_near(m, i) == 0) {	// 是雷区内格
					Map_Backup.insert(pair<double, int>(whole_possiblity, i));
					continue;
				}
				double posb = 0;	
				for (int k = 0; k < 8; ++k) { // 查看该位置是否属于子雷区, 即周围是否有数字格
					if (is_number(m, i + offset[k])) {
						double c = cnt_masked_near(m, i + offset[k]);
						double tp = 0; 
						if (0 < c) 
							tp = m[i + offset[k]] / c;
						if (posb < tp) posb = tp;
					}
				}
				if (0 < posb)  { // 在雷区边界上, 为最大概率
					Map_Backup.insert(pair<double, int>(posb, i));
				}
			}
	}
	int number = Map_Backup.count((Map_Backup.begin())->first);
	int r = rand() % number;
	multimap < double, int > ::iterator p = Map_Backup.begin();
	for (int i = 0; i < r; ++i, ++p) NULL;
	return p->second;
}

void search_for_mine_and_nomine(const I_S_M_F& s, set< int >& Mine, set<int>& Nomine)
{
	I_S_M_F::const_iterator it = s.begin();
	for (; it != s.end(); ++it) {
		if ((it->second).second == 0) {	// 都不是雷
			my_insert(Nomine, (it->second).first);
		}
		else if ((it->second).first.size() == (it->second).second) { // 都是雷
			my_insert(Mine, (it->second).first);
		}
	}
}

void make_command_list(const vector< int >& m, list< Cmd >& cl, const set< int >& Nomine, const set< int >& Mine)
{
	set< int >::const_iterator it = Nomine.begin();
	for (; it != Nomine.end(); ++it) 
		cl.push_back(Cmd(*it, Left));
	it = Mine.begin();
	for (; it != Mine.end(); ++it) 
		cl.push_back(Cmd(*it, Right));
	if (cl.size() == 0)
		cl.push_back(Cmd(search_for_min_possible_rand_point(m), Left));

}

void execute_command_list(vector< int >&m, list< Cmd >& cl)
{
	if (cl.size() == 0) {
		cl.push_back(Cmd(rand() % (m_row * m_col), Left));
	}
	list< Cmd >::const_iterator it = cl.begin();
	for (; it != cl.end(); ++it) {
		dig(m, *it);
	}
}

int GameState(const vector< int >& m)
{
	bool Flag = true;
	for (int i = 0; i < m_col * m_row; ++i) {
		if (m[i] == BLAST) return FAIL;
		else if (m[i] == MASKED) Flag = false;
	}
	if (Flag)
		return SUCCESS;
	else
		return CONTINUE;
}

void main()
{
	srand(time((time_t*) NULL));
	set< int > mine, nomine;
	list< Cmd > clst;
	vector<int> user;
	I_S_M_F ismf;
//	for (int cnt =1, W = 0, R = 0; cnt <= 100 ; ++cnt) {
	for (;;) {
		create_mine_field(user);
		long t = clock();
		int state = CONTINUE;
		while (state == CONTINUE) {
			system("cls");
			mine.clear(); nomine.clear(); ismf.clear(); clst.clear();
			search_for_subminefields(user, ismf);
			split_subminefields2(user, ismf);
			search_for_mine_and_nomine(ismf, mine, nomine);
//	combination(user, ismf, mine, nomine);
			make_command_list(user, clst, nomine, mine);
			execute_command_list(user, clst);
			print(user, m_col);
			state = GameState(user);
		}
		if (state == FAIL) {
			cout << "又失败了" << endl;
		}
		else {
			cout << "恭喜你, 扫雷成功了" << endl;
		}
//		cout << "used " << (clock() - t) / 1000.0 << " second" << endl;
		system("pause");
	}
//	cout << "成功率: " << (W * 1.0) / 10  << "%" << endl;
}


void position(int i, const set< int >& s, vector< int >& p)
{	// i 为子雷区的位置索引
	vector< int > v;
	set_to_vector(s, v);
	int t, k;
	switch (v.size()) {
	case 1:
	case 2: 
		for (k = 0; k < 8; ++k) {
			if (!is_range_in(i + offset[k], 0, m_row * m_col)) continue;
			p.push_back(i + offset[k]); 
		}
		break;
	case 3: 
		if		(v[1] + 1 == v[2] && v[2] + 1 == v[3] && v[2] < i)			t = v[2] - m_col;	// 排列1
		else if (v[1] + 1 == v[2] && v[2] + 1 == v[3] && i < v[2])			t = v[2] + m_col;	// 排列2
		else if (v[1] + m_col == v[2] && v[2] + m_col == v[3] && i < v[2])	t = v[2] + 1;		// 排列3
		else if (v[1] + m_col == v[2] && v[2] + m_col == v[3] && v[2] < i)	t = v[2] - 1;		// 排列4
		else if (v[1] + 2 == v[2] && v[2] + m_col - 2 == v[3] && v[3] < i)	t = v[1] + 1;		// 排列5
		else if (v[1] + 2 == v[2] && v[2] + m_col - 2 == v[3] && i < v[3])	t = v[3] + 1;		// 排列6 
		else if (v[1] + m_col == v[2] && v[2] + 2 == v[3] && v[1] < i)		t = v[2] + 1;		// 排列7
		else if (v[1] + m_col == v[2] && v[2] + 2 == v[3] && i < v[3])		t = v[1] + 1;		// 排列8
		if (is_range_in(t, 0, m_row * m_col)) p.push_back(t);
		break;
	case 4: 
		if		(v[1] + 2 == v[2] && v[2] + m_col == v[4] && v[4] - 2 == v[3] && v[2] < i) t = v[1] + 1; // 排列9 
		else if (v[1] + 2 == v[2] && v[2] + m_col == v[4] && v[4] - 2 == v[3] && i < v[2]) t = v[3] + 1; // 排列10
		if (is_range_in(t, 0, m_row * m_col)) p.push_back(t);
		break;
	default:
		p.resize(0);
	}
}

void split_subminefields2(const vector< int >& m, I_S_M_F& s)
{	// 对于有包含关系的子雷区进行拆分
	I_S_M_F::iterator it = s.begin(); 
	stack< I_S_M_F::iterator > s_buff;
	vector< int > pv;		// 存放子雷区要查看的位置
	for (; it != s.end(); ++it) {
		s_buff.push(it);
		while (!s_buff.empty()) {
			I_S_M_F::iterator itp = s_buff.top();
			s_buff.pop();
			pv.clear();
			position(itp->first, (itp->second).first, pv);
			for (int i = 0; i < pv.size(); ++i) {  // 只需查看关键码周围的8个数字格
				if (s.find(pv[i]) == s.end()) continue;	// 该位置没有带索引的子雷区
				if (my_includes((itp->second).first, s[pv[i]].first)) { // 有包含关系, 改变自身
					my_erase((itp->second).first, s[pv[i]].first);
					(itp->second).second -= s[pv[i]].second;
					s_buff.push(itp);
				}
				else if (my_includes(s[pv[i]].first, s[itp->first].first)) { // 有被包含关系, 改变它
					my_erase(s[pv[i]].first, (itp->second).first);
					s[pv[i]].second -= (itp->second).second;
					s_buff.push(s.find(pv[i]));				
				}
			}
		}
	}
}



















