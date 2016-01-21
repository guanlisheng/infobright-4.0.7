#include <string>
#include "TestOfMaxLookupVsDPN.h"
#include "boost/filesystem.hpp"
#include "core/Filter.h"
#include "core/RCAttr.h"
#include "common/CommonDefinitions.h"
#include "edition/local.h"

#define TEST_OF_MAX_LOOKUP_VS_DPN_H_ "Test of equality of min/max values in lookup dict and dpn"

using namespace std;
using namespace boost::filesystem;

TestOfMaxLookupVsDPN::TestOfMaxLookupVsDPN(void)
{
	table_test = 0;
	test_name = string(TEST_OF_MAX_LOOKUP_VS_DPN_H_);
}

TestOfMaxLookupVsDPN::TestOfMaxLookupVsDPN(string& tp, TableTest* table_test, int col_number)
{
	this->table_test = (table_test);
	test_name = string(TEST_OF_MAX_LOOKUP_VS_DPN_H_);
	vector<string> tp_dec = TableTest::ParseTablePath(tp);
	data_dir = tp_dec[0];
	database_name = tp_dec[1];
	table_name = tp_dec[2].substr(0, tp_dec[2].find('.'));
//	column_name = string((table_test->GetRCTable())->AttrName(col_number));
	column_number = col_number;
}

TestOfMaxLookupVsDPN::~TestOfMaxLookupVsDPN(void)
{
}

std::pair<int, std::string> TestOfMaxLookupVsDPN::Run() throw(DatabaseRCException)
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
		rca->LoadPackInfo();		
		_int64 lmax, max_dpn = MINUS_INF_64;
		for(int i = 0; i < bh::common::NoObj2NoPacks(rca->NoObj()); i++) {
			lmax = rca->GetMaxInt64(i);
			if(lmax > max_dpn)
				max_dpn = lmax;
		}
		_int64 max_look = rca->dic->CountOfUniqueValues();
		if(((max_look - 1) == max_dpn) || (max_dpn == PLUS_INF_64)){ // ok || in columns there are only nulls
			res.first = 1;
			res.second = string("");
		} else {
			res.first = 2;
			res.second = string("Inequality between max values: lookup_dict / dpn ( ") + boost::lexical_cast<string>(max_look-1) + string(" / ") +
					boost::lexical_cast<string>(max_dpn) + string(" )");
		}
	} catch (...) {
		res.first = 2;
		res.second = string("File of column ") + string(rct.AttrName(column_number)) + string(" could not be read.");
		return res;
	}

	return res;
}

