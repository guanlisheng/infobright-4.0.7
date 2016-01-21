#include <string>
#include "TestOfNoObjFirstColumnVsDelMask.h"
#include "boost/filesystem.hpp"
#include "core/Filter.h"
#include "core/RCAttr.h"
#include "edition/local.h"

#define TEST_OF_NO_OBJ_FIRST_COLUMN_VS_DEL_MASK "Test of equality of object number in first column vs delete mask"

using namespace std;
using namespace boost::filesystem;

TestOfNoObjFirstColumnVsDelMask::TestOfNoObjFirstColumnVsDelMask(void)
{
	table_test = 0;
	test_name = string(TEST_OF_NO_OBJ_FIRST_COLUMN_VS_DEL_MASK);
}

TestOfNoObjFirstColumnVsDelMask::TestOfNoObjFirstColumnVsDelMask(string& tp, TableTest* table_test)
{
	this->table_test = (table_test);
	test_name = string(TEST_OF_NO_OBJ_FIRST_COLUMN_VS_DEL_MASK);
	vector<string> tp_dec = TableTest::ParseTablePath(tp);
	data_dir = tp_dec[0];
	database_name = tp_dec[1];
	table_name = tp_dec[2].substr(0, tp_dec[2].find('.'));

}

TestOfNoObjFirstColumnVsDelMask::~TestOfNoObjFirstColumnVsDelMask(void)
{
}

std::pair<int, std::string> TestOfNoObjFirstColumnVsDelMask::Run() throw(DatabaseRCException)
{
	pair<int, string> res;
	RCAttr* rca;
	Filter& dm = table_test->GetDeleteMask();
	RCTable& rct = table_test->GetRCTable();
	int ex_dm = table_test->ExistsProperDelMaskInFolder();

	if(table_test->IsEmptyDelMask() || ex_dm==0) {
		res.first = 0;
		res.second = string("No delete mask found.");
		return res;
	}
	if(ex_dm==1) {
		res.first = 2;
		res.second = string("No proper file with delete mask found");
		return res;
	}

	if(table_test->IsEmptyRCTable()){
		res.first = 0;
		res.second = string("RC Table could not be read.");
		return res;
	}
	rca = rct.GetAttr(0);
	_int64 rcan = rca->NoObj();
	_int64 dmn = dm.NoObj();
	if(rcan == dmn){
		res.first = 1;
		res.second = string("");
	}
	else {
		res.first = 2;
		res.second = string("Inconsistent number of obj. in column file (") + boost::lexical_cast<string>(rcan) +
				string(") and in delete mask (") + boost::lexical_cast<string>(dmn) + string(")");
	}

	return res;
}

