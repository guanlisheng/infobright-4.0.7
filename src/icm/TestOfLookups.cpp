#include <string>
#include "TestOfLookups.h"
#include "boost/filesystem.hpp"
#include "core/RCAttr.h"
#include "common/CommonDefinitions.h"
#include "edition/local.h"

#define TEST_OF_LOOKUPS_H_ "Test of lookup dictionary consistency"

using namespace std;
using namespace boost::filesystem;

TestOfLookups::TestOfLookups(void)
{
	table_test = 0;
	test_name = string(TEST_OF_LOOKUPS_H_);
}

TestOfLookups::TestOfLookups(string& tp, TableTest* table_test, int col_number)
{
	this->table_test = (table_test);
	test_name = string(TEST_OF_LOOKUPS_H_);
	vector<string> tp_dec = TableTest::ParseTablePath(tp);
	data_dir = tp_dec[0];
	database_name = tp_dec[1];
	table_name = tp_dec[2].substr(0, tp_dec[2].find('.'));
//	column_name = string((table_test->GetRCTable())->AttrName(col_number));
	column_number = col_number;
}

TestOfLookups::~TestOfLookups(void)
{
}

std::pair<int, std::string> TestOfLookups::Run() throw(DatabaseRCException)
{
//	assert((table_test->rc_table)->IsLookup(column_number));
	pair<int, string> res;
	if(table_test->IsEmptyRCTable()) {
		res.first = 0;
		res.second = string("RC Table could not be read.");
		return res;
	}
	RCTable& rct = table_test->GetRCTable();
	RCAttr* rca = rct.GetAttr(column_number);
	try {
		if(rct.NoObj()==0) {
			res.first = 1;
			res.second = string("");
			return res;
		}
		rca->LoadLookupDictFromFile();
		switch (rca->dic->CheckConsistency()) {
		case 0:
			res.first = 1;
			res.second = string("");
			break;
		case 1:
			res.first = 2;
			res.second = string("Invalid lookup value length.");
			break;
		case 2:
			res.first = 2;
			res.second = string("Lookup values are not unique.");
			break;
		}
	} catch (...) {
		res.first = 2;
		res.second = string("Lookup dictionary of column ") + string(rct.AttrName(column_number)) + string(" could not be read.");
		return res;
	}

	return res;
}

