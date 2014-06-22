#ifndef POINT_H
#define POINT_H
#include <iostream>
using namespace std;

class Point {
	int x, y;
public:
	Point(int _x = 0, int _y = 0) { x = _x; y = _y; }
	Point& operator += (const Point& p) { x += p.x; y += p.y; return *this; }
	Point& operator = (const Point& p) { x = p.x; y = p.y; return *this; }
	int getx() const { return x; }
	int gety() const { return y; }
};

bool operator == (const Point& pl, const Point& pr) { return pl.getx() == pr.getx() && pl.gety() == pr.gety(); }
bool operator < (const Point& pl, const Point& pr) { if (pl.getx() < pr.getx() || pl.getx() == pr.getx() && pl.gety() < pr.gety()) return true; return false; }
Point operator + (const Point& pl, const Point& pr) { return Point(pl.getx() + pr.getx(), pl.gety() + pr.gety()); }
ostream& operator << (ostream& oo, const Point& p) { return oo << '(' << p.getx() << ',' << p.gety() << ')'; }

Point OFFSET[] = {
	Point(-1, -1), Point(0, -1), Point(1, -1), 
	Point(-1, 0), Point(1, 0), 
	Point(-1, 1), Point(0, 1), Point(1, 1) 
};


/* ÓÃÓÚ²âÊÔ
void main()
{
	Point p1(2, 3), p2( 3, 4);
	cout << (p1 + p2) << endl;
	p1 += p2;
	p1 += p1;
	cout << p1 << endl;
}
*/
#endif