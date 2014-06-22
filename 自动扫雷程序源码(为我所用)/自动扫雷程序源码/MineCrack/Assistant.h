#ifndef ASSISTANT_H
#define ASSISTANT_H
template < class T > void set_to_vector(set< T >& s, vector< T >& v)
{	// 将集合转化成向量以便利用组合算法
	set< T >::iterator _i = s.begin();
	for (; _i != s.end(); ++_i) {
		v.push_back(*_i);
	}
}

#endif