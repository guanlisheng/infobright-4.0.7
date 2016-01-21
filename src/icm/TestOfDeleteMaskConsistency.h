#pragma once

#include "DataIntegrityTest.h"
#include "TableTest.h"

class TestOfDeleteMaskConsistency :
	public DataIntegrityTest
{
public:
	TestOfDeleteMaskConsistency(void);
	TestOfDeleteMaskConsistency(std::string& tp, TableTest* table_test);
	~TestOfDeleteMaskConsistency(void);	
	std::pair<int, std::string> Run() throw(DatabaseRCException);
};
