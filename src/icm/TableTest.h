#pragma once

#include <string>
#include <map>
#include "DataIntegrityTest.h"
#include "system/RCException.h"
#include "system/IBFileSystem.h"
#include "core/JustATable.h"
//#include "shared/core/RCEngine.h"


class Filter;

typedef std::map<int, CHARSET_INFO> charset_infos_t;

extern charset_infos_t charset_infos;

class TableTest : public DataIntegrityTest
{
//	boost::shared_ptr<Filter>			delete_mask;
	boost::shared_ptr<RCTable> 			rc_table;
//	RCAttr*			rc_attr;
	std::vector<DataIntegrityTest*> t;
	std::string		table_path;

public:
	TableTest(std::string& table_path);	
	~TableTest(void);
	void Init(const std::string& kn_folder = "", const std::string& ext_tool = "");	
	void InitColumn(const std::string& col_name);
	bool IsEmptyDelMask();
	bool IsEmptyRCTable();
	Filter& GetDeleteMask();
	RCTable& GetRCTable();
//	std::string GetColumnName();

	std::pair<int, std::string> Run() throw(DatabaseRCException);
	int ExistsProperDelMaskInFolder();

	static std::vector<std::string> ParseTablePath(const std::string& path_to_parse);
	bool IsCorrupted() const {return (rc_table == 0);};

};
