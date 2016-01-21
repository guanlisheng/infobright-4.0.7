#pragma once

#include "DataIntegrityTest.h"
#include "TableTest.h"

class TestOfMaxLookupVsDPN :
	public DataIntegrityTest
{
public:
	TestOfMaxLookupVsDPN(void);
	TestOfMaxLookupVsDPN(std::string& tp, TableTest* table_test, int col_number);
	~TestOfMaxLookupVsDPN(void);
	std::pair<int, std::string> Run() throw(DatabaseRCException);
};

