#pragma once

#include "DataIntegrityTest.h"
#include "TableTest.h"

class TestOfTableFilesConsistency : public DataIntegrityTest
{
public:
	TestOfTableFilesConsistency(void);
	TestOfTableFilesConsistency(std::string& tp, TableTest* table_test);
	~TestOfTableFilesConsistency(void);	
	std::pair<int, std::string> Run() throw(DatabaseRCException);
};
