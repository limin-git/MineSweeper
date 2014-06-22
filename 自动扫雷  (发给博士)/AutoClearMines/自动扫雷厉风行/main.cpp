
#include "headers.h"
#include "mine_box.h"
int i ;

vector<Box> box(9);
int main()
{
	box[0].nearby.insert(&box[1]);
	box[0].nearby.insert(&box[3]);
	box[0].nearby.insert(&box[4]);
	box[0].me = blank;

	box[1].nearby.insert(&box[0]);
	box[1].me = 1;

	box[3].nearby.insert(&box[0]);
	box[3].nearby.insert(&box[6]);
	box[3].me = 2;

	box[4].nearby.insert(&box[0]);
	box[4].nearby.insert(&box[6]);
	box[4].me = 2;

	box[6].nearby.insert(&box[3]);
	box[6].nearby.insert(&box[4]);
	box[6].nearby.insert(&box[7]);
	box[6].me = blank;

	box[7].nearby.insert(&box[6]);
	box[7].me = 1;


	for (i = 0; i < 2; ++i)
	{
		box[i].check();
	}

	for (i = 0; i < box.size(); ++i)
	{
		cout << i << '\t' ;
		box[i].print();
		cout << endl;
	}
	cout << endl;

	return 0;
}