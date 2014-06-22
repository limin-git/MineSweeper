#ifndef ASSISTANT_H
#define ASSISTANT_H
ostream& operator << (ostream& oo, const set < S_M_F >& s)
{
	set < S_M_F >::const_iterator _i = s.begin();
	for (; _i != s.end(); ++_i) {
		set < Point > ::const_iterator _i_p = (*_i).first.begin();
		cout << "[\t";
		for (; _i_p != (*_i).first.end(); ++_i_p) {
			oo << *_i_p << '\t';
		}
		oo << "]\t" << (*_i).second << endl;
	}
	return oo;
}


bool operator < (const S_M_F& _L, const S_M_F& _R)
{
	return (_L.first < _R.first);
}

bool operator > (const S_M_F& _L, const S_M_F& _R)
{
	return _R < _L;
}

template < class T > void set_to_vector(set< T >& s, vector< T >& v)
{	// 将集合转化成向量以便利用组合算法
	set< T >::iterator _i = s.begin();
	for (; _i != s.end(); ++_i) {
		v.push_back(*_i);
	}
}

#endif