
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

