#include <string>
#include "TestOfNoObjInColumnsEquality.h"
#include "boost/filesystem.hpp"
#include "core/Filter.h"
#include "core/RCAttr.h"
#include "edition/local.h"

#define TEST_OF_NO_OBJ_IN_COLUMNS_EQUALITY_H_ "Test of equality of object number in columns file header"

using namespace std;
using namespace boost::filesystem;

TestOfNoObjInColumnsEquality::TestOfNoObjInColumnsEquality(void)
{
	table_test = 0;
	test_name = string(TEST_OF_NO_OBJ_IN_COLUMNS_EQUALITY_H_);
}

TestOfNoObjInColumnsEquality::TestOfNoObjInColumnsEquality(string& tp, TableTest* table_test)
{
	this->table_test = (table_test);
	test_name = string(TEST_OF_NO_OBJ_IN_COLUMNS_EQUALITY_H_);
	vector<string> tp_dec = TableTest::ParseTablePath(tp);
	data_dir = tp_dec[0];
	database_name = tp_dec[1];
	table_name = tp_dec[2].substr(0, tp_dec[2].find('.'));
}

TestOfNoObjInColumnsEquality::~TestOfNoObjInColumnsEquality(void)
{
}

std::pair<int, std::string> TestOfNoObjInColumnsEquality::Run() throw(DatabaseRCException)
{
	pair<int, string> res;
	if(table_test->IsEmptyRCTable()) {
		res.first = 0;
		res.second = string("RC Table could not be read.");
		return res;
	}
	RCAttr* rca;
	RCTable& rct = table_test->GetRCTable();
	_int64 eqno, tmp;
	bool equality_holds = true;
	uint i = 0;
	try {
		for(; i<rct.NoAttrs() && equality_holds; i++) {
			rca = rct.GetAttr(i);
			tmp = rca->NoObj();
			if(i==0)
				eqno = tmp;
			else
				if(eqno!=tmp)
					equality_holds = false;
		}
	} catch (...) {
		res.first = 2;
		res.second = string("Column ") + string(rct.AttrName(i-1)) + string(" could not be read.");
		return res;
	}

	if(equality_holds) {
		res.first = 1;
		res.second = string("");
	}
	else {
		res.first = 2;
		res.second = string("Inconsistent number of obj. in column ") + string(rct.AttrName(i-1)) + string(" - is: ") + boost::lexical_cast<string>(tmp) +
				string(" expected: ") + boost::lexical_cast<string>(eqno);
	}

	return res;
}

