//	#define DEBUG
#include <iostream>
#include <ctime>
#include <list>
#include <vector>
#include <algorithm>
#include <set>
#include <map>
#include <stack>
#include "扫Windows.h"
#include "MyCombination.h"
using namespace std;
class AutoClearMines
{
	MineMatrix m;
	COMMAND_LIST cl;
	BLOCK_SET bs;
	POSSIBLE_SET ps;
	bool include(const POINT&, const POINT_SET&) const;
	void AddToBlockSet(const BLOCK&);
	void MakeBlockSet();
	void MakeBlockSetII();
	void InsertMaxPossiblePointToSet(const POSSIBLE&);
	void MakeBlockPointSet(POINT_SET&);
	void ComputePossiblity();
	void AddCommandListFromPointSet(const POINT_SET&, const Mouse&);
	void RandDig();
	void MakeCommandList();
	void MakeCommandListToFile(const char*);
	void MakePossibleBlockSet(const vector<POINT>&, BLOCK_SET& bs, int m, int n);
public:
	AutoClearMines(const char* FileName) { m.CreateFromFile(FileName); }	// 从文件构造雷区矩阵
	AutoClearMines(int r, int c, int n) { m.SetLevel(r, c, n); }			// 标准等级
	void Simulation();	// 模拟扫雷
	void AutoClearWindowsMines(const char*, const char*);	// Windows扫雷
};
void AutoClearMines::AutoClearWindowsMines(const char* MineMatrixFile, const char* CommandListFile)
{
	m.CreateFromFile(MineMatrixFile);
	MakeCommandListToFile(CommandListFile);
}
template <class T>
void SetToVector(vector<T>& v, set<T>& s)
{
	try {
	set<T>::iterator p = s.begin();
	for (; p != s.end(); ++p) {
		v.push_back(*p);
	}
	} catch(...) { cout << "SetToVector" << endl; }
}
void AutoClearMines::MakeBlockSetII()
{	// 利用"夹逼定理"
	// 进一步对边界进行细分
	// 这一次加入可能性
	try {
	vector<BLOCK> buff(200);
	int i, j, k;
	BLOCK_SET MaxBlock, MinBlock, MustBlock;
	vector<POINT> t, v;
	BLOCK_SET::_I p = bs.begin();
	for (; p != bs.end(); ++p) {
		if ((*p).first.size() > (*p).second && (*p).second > 0) {
			t.clear(); v.clear();
			SetToVector(t, (*p).first);
			for (i = (*p).second +1; i < t.size(); ++i) { // 加入MaxBlock集
				v.clear();
				combi(t, i, v);
				MakePossibleBlockSet(v, MaxBlock, i, (*p).second);
			}
			for (i = (*p).second-1, k = t.size() - 1; i > 0; --i) { // 加入MinBlock集
				v.clear();
				combi(t, k, v);
				MakePossibleBlockSet(v, MinBlock, k--, i);
			}
		}
	}
	set_intersection(MaxBlock.begin(), MaxBlock.end(), MinBlock.begin(), MinBlock.end(), buff.begin());
	vector<BLOCK>::_I p_vb = buff.begin();
	for (; p_vb != buff.end(); ++p_vb) {
		if ((*p_vb).first.size() != 0) {
			MustBlock.insert(*p_vb);
		}
	}
/*
m.Print();
cout << "MaxBlock" << endl << MaxBlock << endl;
cout << "MinBlock" << endl << MinBlock << endl;
cout << "MustBlock" << endl << MustBlock << endl;
*/
	p = MustBlock.begin();
	for (; p != MustBlock.end(); ++p) {
		AddToBlockSet(*p);
	}
	}catch (...) { cout << "MakeBlockSetII" << endl; }
}
void AutoClearMines::MakePossibleBlockSet(const vector<POINT>&comb, BLOCK_SET& pbs, int m, int n)
{
	// 所有组合都存放在vp中, 一个组合含m个元素, 每个组合中可能含有n个雷, 
	// 将每个组合与雷组成的块插入到pbs中 
	try {
	int i=0;
	POINT_SET p;
	while (i < comb.size()) {
		p.clear();
		for (int j = 0; j < m; j++) {
			p.insert(comb[i++]);
		}
		pbs.insert(BLOCK(p, n));
	}
	} catch(...) { cout << "MakePossibleBlockSet" << endl; }
}
void AutoClearMines::Simulation()
{
	Time t; int cnt = 0;
	for (;;) {	cnt++;
		MakeCommandList();
		COMMAND_LIST::_C_I p = cl.begin();
		for (; p != cl.end(); ++p) {
			m.Dig(*p);
		}
		if (m.IsWin()) {
			m.Print(); cout << "success." << endl << cnt << endl;return; 
		}
		else if (m.IsGameOver()) {
			m.Print(true); cout << "failed" << endl; return; 
		}
		else {
			m.Print();
		}
	}
	
}
void AutoClearMines::MakeCommandListToFile(const char* FileName)
{
	MakeCommandList();
	ofstream out(FileName, ios::out);
	output(out, cl);
	out.close();
}
bool AutoClearMines::include(const POINT& p, const POINT_SET& ps) const
{
	// 判断一个位置是否属于一个集合, 计算概率时用此函数
	POINT_SET::_C_I cp = ps.begin();
	for (; cp != ps.end(); ++cp) {
		if (p == (*cp)) return true;
	}
	return false;
}
void AutoClearMines::AddToBlockSet(const BLOCK& b)
{
	// 原名叫: CutToPieces
	// 这是此算法的核心部分
	// 将一个雷块加入雷块集, 且确保该雷块不包含或包含于其它雷块
	// 一个雷块是一个pair: 位置集, 雷数. 也可以将其看是一个子雷区, 并可考虑用递归算法
	POINT_BUFF buff(8);	// 用于存放set_difference中的差集
	BLOCK_SET::_I p = bs.begin();
	for (; p != bs.end(); ++p) {
		if (includes((*p).first.begin(), (*p).first.end(), // 如果b包含一个子雷块
					 b.first.begin(), b.first.end()) ) {
			set_difference(((*p).first).begin(), ((*p).first).end(), // 将差集送入buff
							(b.first).begin(), (b.first).end(), 
							buff.begin());
			POINT_SET t;
			for (int i=0; i<((*p).first).size() - (b.first).size(); ++i) { // 从buff中取出差集
				t.insert(buff[i]);
			} 
			bs.insert(BLOCK(t, (*p).second - b.second)); // 插入新的雷块
		}
		else if (includes(b.first.begin(), b.first.end(),  // 处理b是一个雷块的子雷块的情况
						  (*p).first.begin(), (*p).first.end())) {
			set_difference((b.first).begin(), (b.first).end(), 
							((*p).first).begin(), ((*p).first).end(), 
							buff.begin());
			POINT_SET t;
			for (int i=0; i<((b).first).size() - ((*p).first).size(); ++i) {
				t.insert(buff[i]);
			}
			bs.insert(BLOCK(t, (b).second - (*p).second));
		}
	}
	bs.insert(b);
}
void AutoClearMines::MakeBlockSet()
{	// 将未展开雷区的边界上的雷划分成一个个雷块
	bs.clear();
	static POINT p;
	for (int i=1; i<=m.r; i++) {
		for (int j=1; j<=m.c; j++) {
			if (!m.Get(i, j).IsMasked() && m.Get(i, j).IsNumber()) { // 在已展开区域中搜索
				POINT_SET sp; sp.clear();
				for (int k=0; k<8; k++) { // 查看四周
					p = POINT(i, j) + OffSet[k];
					Mine& mi = m.Get(p);
					if (mi.IsMasked() && !mi.IsMarked()) { // 是未展开雷区的边界
						sp.insert(p); // 收集位置
					}
				}
				if (sp.size() >0) { // 找到一个雷块
					BLOCK b(sp, m.Get(i, j).GetN() - m.HowManyMarkedNear(POINT(i, j)));// 构造雷块
					AddToBlockSet(b); // 将该雷块插入到雷块集
				}
			}
		}
	}
cout << bs << endl;
}
void AutoClearMines::InsertMaxPossiblePointToSet(const POSSIBLE& p)
{
	// 考虑到交叉情况, 总是将"是雷的概率"大的位置放入集合
	POSSIBLE_SET::_C_I cp = ps.begin();
	for (; cp != ps.end(); ++cp) {
		if (p.second == (*cp).second) {
			if (p.first > (*cp).first) {
				ps.erase(*cp);
				ps.insert(p);
			}
			return;
		}
	}
	ps.insert(p);
}
void AutoClearMines::MakeBlockPointSet(POINT_SET& bp)
{
	/* 从雷块集合中计算边界的位置集, 计算非边界未展开雷区中的位置"是雷的概率"时用此函数
	目的是求雷块集合中的交集的个数, 以确定边界中最少的雷数, 
	从而算出非边界区域中"是雷的概率"最大值 */
	bp.clear();
	BLOCK_SET::_I p = bs.begin();
	for (; p != bs.end(); p++) {
		POINT_SET::_I ps = (*p).first.begin();
		for (; ps != (*p).first.end(); ps++) {
			bp.insert(*ps);
		}
	}
}
void AutoClearMines::ComputePossiblity()
{
	ps.clear();
	POINT_SET BlockPointSet;
	MakeBlockPointSet(BlockPointSet); // 获取边界位置集合
	BLOCK_SET::_I p_bs = bs.begin();
	int max=0; // 边界中雷最大数
	for (; p_bs != bs.end(); p_bs++) {  // 将边界位置的位置"是雷的概率"加入集合
		max += (*p_bs).second;
		POINT_SET::_I p_p_bs = (*p_bs).first.begin();
		for (; p_p_bs != (*p_bs).first.end(); p_p_bs++) { // 确保大的概率值
			InsertMaxPossiblePointToSet(POSSIBLE((double)(*p_bs).second/(*p_bs).first.size(), *p_p_bs));
		}
	}
	double min = max - BlockPointSet.size(); // 边界中雷的最小值
	double Possiblity = (m.n - m.HowManyMarkedWhole() - min)/  // 非边界未展开区域中位置"是雷的概率"最大值
					    (m.HowManyMaskedWhole() - BlockPointSet.size());
	for (int i=1; i<=m.r; ++i) { // 将非边界未展开区域中"是雷的概率"加入集合
		for (int j=1; j<=m.c; ++j) {
			if (m.Get(i, j).IsMasked() && !m.Get(i, j).IsMarked() && !include(POINT(i, j), BlockPointSet)) {
				ps.insert(POSSIBLE(Possiblity, POINT(i, j)));
			}
		}
	}
}
void AutoClearMines::AddCommandListFromPointSet(const POINT_SET& ps, const Mouse& m)
{	// 加入一系列的命令到命令集
	POINT_SET::_C_I p = ps.begin(); 
	for (; p != ps.end(); ++p) 
		cl.push_back(COMMAND(*p, m));
}
void AutoClearMines::MakeCommandList()
{
	POINT_SET LeftSet, RightSet, DoubleSet; // 确保所加入的命令是唯一的
	MakeBlockSet();
	MakeBlockSetII();
	BLOCK_SET::_I p = bs.begin();
	POINT_SET::_C_I cp;
	for (; p != bs.end(); ++p) {
		if ((*p).second == 0) {	// 该雷块无雷, 加入鼠标左键集合
			cp = ((*p).first).begin();
			for (; cp != ((*p).first).end(); cp++) {
				LeftSet.insert(*cp);
			}
		}
		else if (((*p).first).size() == (*p).second) { // 该雷块全是雷, 加入鼠标右键集合
			cp = ((*p).first).begin();
			for (; cp !=((*p).first).end(); ++cp) {
				RightSet.insert(*cp);
			}
		}
	}
	for (int i=1; i<=m.r; i++) { // 为了加快速度, 将可能用双键的位置加入双键集合
		for (int j=1; j<=m.c; j++) {
			const Mine& M = m.Get(i, j);
			if (!M.IsMasked() && 
				M.IsNumber() && 
				M.GetN() == m.HowManyMarkedNear(POINT(i, j)) &&
				m.HowManyMaskedNear(POINT(i, j))) {
				DoubleSet.insert(POINT(i, j));
			}
		}
	}
	// 加入一系列的命令
	cl.clear();
	AddCommandListFromPointSet(RightSet, RIGHT);
	AddCommandListFromPointSet(DoubleSet, LEFTRIGHT);
	AddCommandListFromPointSet(LeftSet, LEFT);
	if (cl.size() == 0) {
		RandDig();
	}
}
void AutoClearMines::RandDig()
{	// 根据概率随机挖一次雷
	ComputePossiblity();
	POSSIBLE_SET::_I p = ps.begin();
	for (int k=0; p != ps.end() && (*p).first == (*ps.begin()).first; ++p, ++k) NULL;
	srand(time((time_t*) NULL));
	int r = rand()%k;	
	p = ps.begin(); while(r--) p++;
	cl.push_back(COMMAND((*p).second, LEFT));
}
#ifndef DEBUG
void main()
{
	AutoClearMines ACM(16, 30, 99);
	ACM.Simulation();
//	AutoClearMines ACM(0, 0, 0);
//	ACM.AutoClearWindowsMines(MINE_MATRIX_FILE, COMMAND_LIST_FILE);

}
#endif
#ifdef DEBUG
void main()
{
}
#endif
