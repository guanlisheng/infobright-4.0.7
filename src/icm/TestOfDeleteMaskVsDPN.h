#pragma once

#include "DataIntegrityTest.h"
#include "TableTest.h"

class TestOfDeleteMaskVsDPN :
	public DataIntegrityTest
{
public:
	TestOfDeleteMaskVsDPN(void);
	TestOfDeleteMaskVsDPN(std::string& tp, TableTest* table_test, int col_number);
	~TestOfDeleteMaskVsDPN(void);
	std::pair<int, std::string> Run() throw(DatabaseRCException);
};
