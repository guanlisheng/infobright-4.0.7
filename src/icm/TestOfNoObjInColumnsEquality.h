#pragma once

#include "DataIntegrityTest.h"
#include "TableTest.h"

class TestOfNoObjInColumnsEquality :
	public DataIntegrityTest
{
public:
	TestOfNoObjInColumnsEquality(void);
	TestOfNoObjInColumnsEquality(std::string& tp, TableTest* table_test);
	~TestOfNoObjInColumnsEquality(void);
	std::pair<int, std::string> Run() throw(DatabaseRCException);
};

