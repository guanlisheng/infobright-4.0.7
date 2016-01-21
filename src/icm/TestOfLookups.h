#pragma once

#include "DataIntegrityTest.h"
#include "TableTest.h"

class TestOfLookups :
	public DataIntegrityTest
{
public:
	TestOfLookups(void);
	TestOfLookups(std::string& tp, TableTest* table_test, int col_number);
	~TestOfLookups(void);
	std::pair<int, std::string> Run() throw(DatabaseRCException);
};

