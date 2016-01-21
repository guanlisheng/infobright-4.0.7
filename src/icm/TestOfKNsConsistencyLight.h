#pragma once

#include "DataIntegrityTest.h"
#include "TableTest.h"

class TestOfKNsConsistencyLight :
	public DataIntegrityTest
{
	std::string knfolder_path;
public:
	TestOfKNsConsistencyLight(void);
	TestOfKNsConsistencyLight(const std::string& tp, TableTest* table_test, int col_number, const std::string& knf_path);
	~TestOfKNsConsistencyLight(void);	
	std::pair<int, std::string> Run() throw(DatabaseRCException);
};
