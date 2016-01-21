#include "TestOfDeleteMaskConsistency.h"
#include "boost/filesystem.hpp"
#include "core/Filter.h"
#include <string>

#define TEST_OF_DELETE_MASK_CONSISTENCY "Test of delete mask consistency"

using namespace std;
using namespace boost::filesystem;

TestOfDeleteMaskConsistency::TestOfDeleteMaskConsistency(void)
{	
	table_test = 0;
	test_name = (string(TEST_OF_DELETE_MASK_CONSISTENCY));
}

TestOfDeleteMaskConsistency::TestOfDeleteMaskConsistency(string& tp, TableTest* table_test)
{	
	this->table_test = (table_test);
	test_name = (string(TEST_OF_DELETE_MASK_CONSISTENCY));
	vector<string> tp_dec = TableTest::ParseTablePath(tp);
	data_dir = tp_dec[0];
	database_name = tp_dec[1];
	table_name = tp_dec[2].substr(0, tp_dec[2].find('.'));	
}

TestOfDeleteMaskConsistency::~TestOfDeleteMaskConsistency(void)
{
}

std::pair<int, std::string> TestOfDeleteMaskConsistency::Run() throw(DatabaseRCException)
{
	pair<int, string> res;
	int ex_dm = table_test->ExistsProperDelMaskInFolder();

	if(table_test->IsEmptyDelMask() || ex_dm==0) {
		res.first = 0;
		res.second = string("No delete mask found");
		return res;
	}
	if(ex_dm==1) {
		res.first = 2;
		res.second = string("No proper file with delete mask found");
		return res;
	}
	Filter& dm = table_test->GetDeleteMask();
	string block_list = "";
	int block_no_ones;
	try {
		for(int blockInd = 0; blockInd < dm.NoBlocks(); blockInd++) {
			if(dm.GetBlockStatus(blockInd)==Filter::FB_MIXED && !(dm.GetBlock(blockInd)->IsNumberOfOnesInHeaderProper(block_no_ones))) {
				block_list += boost::lexical_cast<string>(blockInd)+" ";
			}
		}
	} catch (...) {
		res.first = 2;
		res.second = string("Can not read delete mask file.");
		return res;
	}

	if(block_list.length()!=0) {
		res.first = 2;
		res.second = string("Inconsistency in delete mask header found in block(s): ")+block_list;
	} else {
		res.first = 1;
		res.second = string("");
	}

	return res;		
}
