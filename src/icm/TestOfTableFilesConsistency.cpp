#include "TestOfTableFilesConsistency.h"
#include "boost/filesystem.hpp"
#include <string>

#define TEST_OF_TABLE_FILES_CONSISTENCY "TestOfTableFilesConsistency"

using namespace std;
using namespace boost::filesystem;

TestOfTableFilesConsistency::TestOfTableFilesConsistency(void)
{	
	table_test = 0;
	test_name = (string(TEST_OF_TABLE_FILES_CONSISTENCY));
}

TestOfTableFilesConsistency::TestOfTableFilesConsistency(string& tp, TableTest* table_test)
{	
	this->table_test = (table_test);
	test_name = (string(TEST_OF_TABLE_FILES_CONSISTENCY));
	vector<string> tp_dec = TableTest::ParseTablePath(tp);
	data_dir = tp_dec[0];
	database_name = tp_dec[1];
	table_name = tp_dec[2].substr(0, tp_dec[2].find('.'));	
}

TestOfTableFilesConsistency::~TestOfTableFilesConsistency(void)
{
}

std::pair<int, std::string> TestOfTableFilesConsistency::Run() throw(DatabaseRCException)
{			
	string table_path = data_dir + DIR_SEPARATOR_CHAR + database_name + DIR_SEPARATOR_CHAR + table_name + string(".bht");
	string ab_switch_path = table_path + DIR_SEPARATOR_CHAR + string("ab_switch");
	string del_mask_alt_path = table_path + DIR_SEPARATOR_CHAR + string("del_mask_alt.flt");
	string del_mask_path = table_path + DIR_SEPARATOR_CHAR + string("del_mask.flt");

	pair<int, string> res;
	try {
		if(exists(ab_switch_path))
		{
			if(exists(del_mask_alt_path))
			{
				res.first = 1;
				res.second = "";
			}				
			else 
			{
				res.first = 2;
				res.second = "ab_switch file exists, but no del_mask_alt.flt found.";
			}				
		} else {
			if(exists(del_mask_path)) 
			{
				res.first = 1;
				res.second = "";
			}
			else
			{
				res.first = 2;
				res.second = "ab_switch file does not exist, but no del_mask.flt found.";
			}
		}
	} catch(...) {
		throw DatabaseRCException("boost::exists method failed");
	}		
	return res;
}
