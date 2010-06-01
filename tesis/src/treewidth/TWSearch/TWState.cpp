#include "preproc_flags.h"
#include "TWState.h"

namespace Treewidth_Namespace {

///////////////////////////////////////////////////////////////////////////////
// TWState - initialize static member variables
///////////////////////////////////////////////////////////////////////////////
int TWState::nVerts_val = 0;
#ifdef TW_ANY_VERTS
int TWState::nInts = 0;
#endif

///////////////////////////////////////////////////////////////////////////////
// TWState printState - returns a string representing the state
///////////////////////////////////////////////////////////////////////////////
string TWState::printState() const {
	ostringstream ost;
	ost << "state(";
	int leftPos=indexMod-nVerts_val%indexMod;
	leftPos= (leftPos==indexMod) ? 0 : leftPos;
	int leftNum=indexMod-leftPos;
	bitset<indexMod> b = bits[nInts-1];
	//string s = b.template to_string<char,char_traits<char>,allocator<char> >();
	string s = b.to_string<char,char_traits<char>,allocator<char> >();
	ost << s.substr(leftPos,leftNum);
	int i = nInts-2;
	while(i>=0) {
		bitset<indexMod> b = bits[i--];
		//ost << " " << b.template to_string<char,char_traits<char>,allocator<char> >();
		ost << " " << b.to_string<char,char_traits<char>,allocator<char> >();
	}
	ost << ")";
	return ost.str();
}

}
