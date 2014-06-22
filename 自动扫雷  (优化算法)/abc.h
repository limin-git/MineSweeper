#include <iostream>
using namespace std;

void print(const vector<int>& v)
{
	for (int i = 0; i < v.size(); ++i)
		cout << v[i] << '\t';
	cout << endl;
}
