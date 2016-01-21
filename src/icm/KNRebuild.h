#pragma once

#include "DataIntegrityTest.h"
#include "TableTest.h"

class KNRebuild :
	public DataIntegrityTest
{
	std::string knfolder_path;
public:
	KNRebuild(void);
	KNRebuild(std::string& tp, TableTest* table_test, int col_number, std::string& knf_path);
	~KNRebuild(void);	
	std::pair<int, std::string> RebuildHistogram() throw(DatabaseRCException);
	std::pair<int, std::string> RebuildCMap() throw(DatabaseRCException);
	std::pair<int, std::string> Run() throw(DatabaseRCException);
};
