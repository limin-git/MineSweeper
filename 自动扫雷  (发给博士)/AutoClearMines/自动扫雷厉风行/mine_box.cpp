
#include "headers.h"
#include "mine_box.h"


void Box::check()
{
	switch (me)
	{
		case 0:	// 附近没有雷, 附近所有格为"空白格"
			if (!nearby.empty())
			{
				set<Box*>::iterator it = nearby.begin();
				for (; it != nearby.end(); ++it)
				{
					(*it)->setme(0, this);	// 设置该位置为"空白格", 并告诉该格自己的位置, 意思是我知道你是什么, 你不用再告诉我你是谁了
				}
				
				nearby.clear();
			}
			break;

		case 1:	case 2:	case 3:	case 4:
		case 5:	case 6:	case 7:	case 8:
			if (!nearby.empty())
			{
				if (nearby.size() == me)
				{
					set<Box*>::iterator it = nearby.begin();
					for (; it != nearby.end(); ++it)
					{
						(*it)->setme(mine, this);
					}
					
					me = 0;
					nearby.clear();
				}
			}
			break;

		default:
			break;
	}
}

void Box::setme(int what, Box* loc)
{
	if (blank != me)
	{
		return;
	}
	me = what;

	set<Box*>::iterator it = nearby.find(loc);
	if (it != nearby.end())
	{
		nearby.erase(loc);
	}

	it = nearby.begin();
	for (; it != nearby.end(); ++it)
	{
		(*it)->update(me, this);
	}

	nearby.clear();
}

void Box::update(int what, Box* loc)
{
	set<Box*>::iterator it = nearby.find(loc);
	if (it != nearby.end()) 
	{
		nearby.erase(loc);
		what? --me : NULL;
	}

	check();
}