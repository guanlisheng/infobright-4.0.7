#pragma once

#include "DataIntegrityTest.h"
#include "TableTest.h"

class TestOfPackFileOverlapping :
	public DataIntegrityTest 
{	
public:
	TestOfPackFileOverlapping(void);
	TestOfPackFileOverlapping(std::string& tp, TableTest* table_test, int col_number);
	~TestOfPackFileOverlapping(void);	
	std::pair<int, std::string> Run() throw(DatabaseRCException);
};
