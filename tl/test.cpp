
#include "tl.h"
namespace tl {
	void test()
	{
		string wk;
		array<int> a;
		list<int> l;
		tree<int> t;
		btree<int,int> bt;
		array<retainer<int> > ar;
		list<retainer<int> > lr;
		tree<retainer<int> > tr;
		btree<int,retainer<int> > btr;
	}
};
