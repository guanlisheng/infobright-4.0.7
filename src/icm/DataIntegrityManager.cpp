/* Copyright (C)  2005-2008 Infobright Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2.0 as
published by the Free  Software Foundation.

This program is distributed in the hope that  it will be useful, but
WITHOUT ANY WARRANTY; without even  the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License version 2.0 for more details.

You should have received a  copy of the GNU General Public License
version 2.0  along with this  program; if not, write to the Free
Software Foundation,  Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA   */

#include <iostream>
#include <utility>
#include <boost/bind.hpp>
//#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>

#include "DataIntegrityManager.h"
#include "DIMParameters.h"
#include "TableTest.h"
#include "system/RCException.h"
#include "system/Channel.h"
#include "system/ChannelOut.h"
#include "system/StdOut.h"
#include "system/FileOut.h"
#include "system/IBFile.h"
#include "system/IBFileSystem.h"

using namespace std;
using namespace boost;
using namespace boost::filesystem;
using namespace boost::algorithm;

DataIntegrityManager::DataIntegrityManager(const DIMParameters& ic_params)
	:	data_dir(ic_params.DataDirPath()),	kn_folder(ic_params.KNFolderPath())	,	database_name(ic_params.DatabaseName())
	,	table_name(ic_params.TableName()),	column_name(ic_params.ColumnName())	,	print_help(ic_params.IsPrintHelpParamSet())
	,	print_version(ic_params.IsVersionParamSet()),	full(ic_params.FullScan())	,	report(ic_params.WithDataFormatReport())
	,	repair(ic_params.ScanWithRepair()),		ext_tool(ic_params.ExternalTool())
{
	//WriteToLog("Starting data integrity checking process...");
}

void DataIntegrityManager::WriteToLog(const string& report)
{
	DIMParameters::LogOutput() << lock << report << unlock;
}

int DataIntegrityManager::Run()
{	
	WriteToLog("Infobright Consistency Manager started.");

	assert(exists(data_dir));
	
	directory_iterator end_itr;
	string fn, fnc, dn, cn;

	for( directory_iterator itr_db(data_dir); itr_db != end_itr; ++itr_db){
		// for each database in datadir...
		dn = itr_db->path().string();
		
		assert((*(data_dir.rbegin()))=='/' || (*(data_dir.rbegin()))=='\\');
		
		if( !is_directory(*itr_db) || (database_name.length()!=0 && dn.length() != (data_dir.length() + database_name.length())) ||
			(database_name.length()!=0 && dn.compare(data_dir.length(), database_name.length(), database_name, 0, database_name.length()) != 0) )
		{
			continue;
		} else {			
			for( directory_iterator itr_tab(*itr_db); itr_tab != end_itr; itr_tab++)
			{	// for each table...								
				fn = itr_tab->path().string();
				fnc = to_upper_copy(fn);	
								
				if( !is_directory(*itr_tab) || (fnc.find_last_of(".BHT") + 1 != fnc.length()) || 
					(table_name.length()!=0 && fn.length() != (dn.length() + table_name.length() + 5)) ||
					(table_name.length()!=0 && fn.compare(dn.length()+1, table_name.length(), table_name, 0, table_name.length())!=0 ) ) {
					continue;
				} else {									
					TableTest tt(fn);
					tt.Init(kn_folder, ext_tool);
					pair<int, string > res_t = tt.Run();
				}
			}
		}				
	}	
	size_t cnt_fails = DIMParameters::failed_tests.size();
	if (cnt_fails == 0)
		WriteToLog("Infobright Consistency Manager successfully finished. No corruptions found.");
	else if (cnt_fails > 0) {
		WriteToLog("Infobright Consistency Manager finished. Corruptions found for:");
		for(vector<FailedTest>::iterator it = DIMParameters::failed_tests.begin(); it!=DIMParameters::failed_tests.end(); ++it) {
			WriteToLog(string("db: ") + (*it).database_name + string(" tab: ") + (*it).table_name + " test: " + (*it).test_name);
		}
		WriteToLog("See test execution report for details.");
	}
	
	return 0;
}
