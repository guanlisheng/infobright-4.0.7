#include "TestOfPackFileOverlapping.h"
#include "boost/filesystem.hpp"
#include "core/Filter.h"
#include "core/RCAttr.h"
#include "common/CommonDefinitions.h"
#include "edition/local.h"
#include <string>
#include <algorithm>

#define TEST_OF_PACK_FILE_OVERLAPPING "Test of data packs overlapping."

using namespace std;
using namespace boost::filesystem;

TestOfPackFileOverlapping::TestOfPackFileOverlapping(void)
{	
	table_test = 0;
	test_name = (string(TEST_OF_PACK_FILE_OVERLAPPING));
}

TestOfPackFileOverlapping::TestOfPackFileOverlapping(string& tp, TableTest* table_test, int col_number)
{
	this->table_test = (table_test);
	test_name = string(TEST_OF_PACK_FILE_OVERLAPPING);
	vector<string> tp_dec = TableTest::ParseTablePath(tp);
	data_dir = tp_dec[0];
	database_name = tp_dec[1];
	table_name = tp_dec[2].substr(0, tp_dec[2].find('.'));
	//	column_name = string((table_test->GetRCTable())->AttrName(col_number));
	column_number = col_number;
}

TestOfPackFileOverlapping::~TestOfPackFileOverlapping(void)
{
}

struct PackTriple {
	int pack_no;
	int file_id;
	unsigned int beg;
	unsigned int fin;
	bool operator() (PackTriple const & i, PackTriple const& j) const { 
		if(i.file_id == j.file_id) {
			if(i.beg==j.beg) 
				return (i.fin < j.fin);
			else
				return (i.beg < j.beg);
		} else {
			return (i.file_id < j.file_id);
		}
	}

	PackTriple() {pack_no=file_id=beg=fin=-1;}
	PackTriple(int p, int id, uint b, uint f){
		pack_no = p; file_id = id; beg = b; fin = f;
	}
} p_triple;

std::pair<int, std::string> TestOfPackFileOverlapping::Run() throw(DatabaseRCException)
{
	pair<int, string> res;

	RCTable& rct = table_test->GetRCTable();
	RCAttr* rca = rct.GetAttr(column_number);	
	FilterPtr dm_ptr = rct.GetDeleteMask();

	string act_fname, fname;
	act_fname = fname = string("");
	vector<PackTriple> addr_book;	
	vector<int> overlapped_packs;
	if(rca->NoObj() == 0) {
		res.first = 1;
		res.second = string("");
	} else {				
		unsigned int ad;
		int f_id = 0;
		for(int i = 0; i<rca->NoPack(); i++) {
			if(rca->ShouldExist(i) && dm_ptr->GetBlockStatus(i) != Filter::FB_EMPTY) {
				fname = rca->AttrPackFileName(i);
				if(fname!=act_fname){
					act_fname = fname;			
					f_id++;
				}
				ad = rca->dpns[i].pack_addr;				
				addr_book.push_back(PackTriple(i, f_id, ad, ad + rca->LoadPackSize(i)));
			}
		}

		sort(addr_book.begin(), addr_book.end(), p_triple);

		for(vector<PackTriple>::iterator i = addr_book.begin(); i != addr_book.end(); ++i) {
			vector<PackTriple>::iterator j = i; 
			j++;
			if(j == addr_book.end())
				continue;
			if( j->file_id != i->file_id )
				continue;
			if( j->beg < i->fin ) {
				overlapped_packs.push_back(i->pack_no);
				overlapped_packs.push_back(j->pack_no);
			}
		}

		if(overlapped_packs.size() != 0) {
			string s("");
			for(size_t i = 0; i < overlapped_packs.size() / 2; i++) {
				s += string("(") + boost::lexical_cast<string>(overlapped_packs.at(2*i)) + string(", ") + 
					boost::lexical_cast<string>(overlapped_packs.at(2*i + 1)) + string(")  ");
			}
			res.first = 2;
			res.second = string("Overlapped packs found. Number of pack overlapped: ") + s;
		} else {
			res.first = 1;
			res.second = string("");
		}
	}
	return res;		
}
