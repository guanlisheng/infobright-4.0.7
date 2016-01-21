#pragma once

#include "DataIntegrityTest.h"
#include "TableTest.h"

class TestOfNoObjFirstColumnVsDelMask :
	public DataIntegrityTest
{
public:
	TestOfNoObjFirstColumnVsDelMask(void);
	TestOfNoObjFirstColumnVsDelMask(std::string& tp, TableTest* table_test);
	~TestOfNoObjFirstColumnVsDelMask(void);
	std::pair<int, std::string> Run() throw(DatabaseRCException);
};

