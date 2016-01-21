#pragma once

#include <string>
#include <utility>
//#include "TableTest.h"
#include "system/RCException.h"
#include "system/IBFileSystem.h"

class TableTest;

class DataIntegrityTest
{
protected:
	std::string data_dir;
	std::string database_name;
	std::string table_name;
	std::string column_name;	
	std::string test_name;
	TableTest* table_test;
	int column_number;

public:
	DataIntegrityTest(void);
	~DataIntegrityTest(void);
	std::string GetTestName() const { return test_name;}
	std::string GetDatabaseName() const { return database_name;}
	std::string GetTableName() const { return table_name;}
	std::string GetColumnName() const { return column_name;};
	const TableTest& GetTableTest() const { return *table_test; };
	virtual std::pair<int, std::string> Run() throw(DatabaseRCException) = 0;
	int RunDataIntegrityTest() throw(DatabaseRCException);
	bool IsTableTestCorrupted();
};
