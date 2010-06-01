#include "preproc_flags.h"
#include "BFHT_HDDD_Node.h"
#include <sstream>
#include <bitset>

namespace Treewidth_Namespace {

uint BFHT_HDDD_State::nVerts_val = 0;

string BFHT_HDDD_State::printState() const {
	ostringstream ost;
	ost << "state(";
	int leftPos=TW_INDEX_MOD-nVerts_val%TW_INDEX_MOD;
	leftPos= (leftPos==TW_INDEX_MOD) ? 0 : leftPos;
	int leftNum=TW_INDEX_MOD-leftPos;
	bitset<TW_INDEX_MOD> b = bits[TW_NINTS-1];
	//string s = b.template to_string<char,char_traits<char>,allocator<char> >();
	string s = b.to_string<char,char_traits<char>,allocator<char> >();
	ost << s.substr(leftPos,leftNum);
	int i = TW_NINTS-2;
	while(i>=0) {
		bitset<TW_INDEX_MOD> b = bits[i--];
		//ost << " " << b.template to_string<char,char_traits<char>,allocator<char> >();
		ost << " " << b.to_string<char,char_traits<char>,allocator<char> >();
	}
	ost << ")";
	return ost.str();
}

}
