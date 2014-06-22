#include "headers.h"

const int mine = 9;
const int blank = 10;

class Box
{
public:
	Box() : me(0) { };
	int me;		// 0-8, 附近的雷数; blank, 未知格; 
	set<Box*> nearby;	// 附近的位置集合

	void check();
	void setme(int what, Box* loc);
	void update(int what, Box* loc);

	void print()
	{
		cout << me;
	}
private:
};

