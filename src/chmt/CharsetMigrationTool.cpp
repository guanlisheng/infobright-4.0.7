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
#include <fstream>
#include <utility>
#include <boost/bind.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>

#include "CharsetMigrationTool.h"
#include "CHMTParameters.h"
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


CharsetMigrationTool::CharsetMigrationTool(const CHMTParameters& ic_params)
	:	data_dir(ic_params.DataDirPath()),	database_name(ic_params.DatabaseName())
	,	table_name(ic_params.TableName()),	conv_map_path(ic_params.ConversionMapPath())
	,	print_help(ic_params.IsPrintHelpParamSet()),	print_version(ic_params.IsVersionParamSet()) 
	//,	ch_conv_map(shared_ptr<map<int,int> >)
{
	//WriteToLog("Starting data integrity checking process...");	
	ch_conv_map = shared_ptr<map<int, int> >(new map<int, int>());
	if(conv_map_path.length()==0)
		LoadCharsetConversionSymbols(CHMT_CONV_FILE);
	else
		LoadCharsetConversionSymbols(conv_map_path);
}

void CharsetMigrationTool::WriteToLog(const string& report)
{
	CHMTParameters::LogOutput() << lock << report << unlock;
}

void CharsetMigrationTool::LoadCharsetConversionSymbols(const string& path)
{		
	ifstream fstr(path.c_str());	
	string st;
	int from, into, pos1, pos2, pos3;
	while(getline(fstr, st, '\n').good()) {
		pos1 = st.find(';');
		pos2 = st.find(';',pos1+1);
		pos3 = st.find(';',pos2+1);
		from = lexical_cast<int>(st.substr(pos1+1,pos2-pos1-1));
		into = lexical_cast<int>(st.substr(pos3+1));
		(*ch_conv_map)[from] = into;				
	}	
}

int CharsetMigrationTool::Run()
{
	directory_iterator end_itr;
	string fn, fnc, dn, cn, column_name;
	WriteToLog("Charset migration started...");
	for( directory_iterator itr_db(data_dir); itr_db != end_itr; itr_db++) {
		// for each database in datadir...
		//dn = (itr_db->path()).string();
		dn = itr_db->path().string();

		assert((*(data_dir.rbegin()))=='/' || (*(data_dir.rbegin()))=='\\');

		//if( !is_directory(itr_db->status()) ||
		if( !is_directory(*itr_db) ||
			(database_name.length() != 0 && dn.compare(data_dir.length(), database_name.length(), database_name, 0, database_name.length()) != 0) )
		{
			continue;
		} else {
			//for( directory_iterator itr_tab(itr_db->path()); itr_tab != end_itr; itr_tab++)
			for( directory_iterator itr_tab(*itr_db); itr_tab != end_itr; itr_tab++)
			{	// for each table...
				//fn = (itr_tab->path()).native_file_string();	
				fn = itr_tab->path().string();	
				fnc = to_upper_copy(fn);

				//if( !is_directory(itr_tab->status()) || (fnc.find_last_of(".BHT") + 1 != fnc.length()) ||
				if( !is_directory(*itr_tab) || (fnc.find_last_of(".BHT") + 1 != fnc.length()) ||
					(table_name.length()!=0 && fn.compare(dn.length()+1, table_name.length(), table_name, 0, table_name.length()) != 0 ) ) {
					continue;
				} else {
					string tab_name = fn.substr(fn.find_last_of(DIR_SEPARATOR_STRING) + 1, fn.length() - fn.find_last_of(DIR_SEPARATOR_STRING) - 5);
					string db_name = fn.substr(0, fn.find_last_of(DIR_SEPARATOR_STRING));
					db_name = db_name.substr(db_name.find_last_of(DIR_SEPARATOR_STRING) + 1);

					string frn_name = fn.substr(0, fn.length() - 4) + ".frm";
					string frn_name_cpy = frn_name + "_cpy";
					string frn_name_bkp = fn + DIR_SEPARATOR_STRING + "frm_bkp";
					//cerr << "upgrading " << frn_name.c_str();
					int migration_needed = IsMigrationNeeded(frn_name);
					if(migration_needed == 0) {
						CopyFileWithSecurity(frn_name, frn_name_cpy);
						pair<int, int> migration_success = MigrateCS(frn_name_cpy);
						if(migration_success.first==1){							
							CopyFileWithSecurity(frn_name, frn_name_bkp);
							RenameFile(frn_name_cpy, frn_name);
							WriteToLog("Charsets migration for table " + db_name + DIR_SEPARATOR_STRING + tab_name + " ... [ PASS ]");
						} else {
							WriteToLog("Charsets migration for table " + db_name + DIR_SEPARATOR_STRING + tab_name + " ... [ FAILED ] Collation of id " + lexical_cast<string>(migration_success.second) + " could not be found. Table was not migrated.");
						}
					} else {
						if(migration_needed == 1)
							WriteToLog("Charsets migration for table " + db_name + DIR_SEPARATOR_STRING + tab_name + " ... [ NOT NEEDED ] Already migrated.");
						else if(migration_needed == 2)
							WriteToLog("Charsets migration for table " + db_name + DIR_SEPARATOR_STRING + tab_name + " ... [ NOT NEEDED ] Charsets up to date.");
					}					
				}
			}
		}
	}
	WriteToLog("Charset migration finished.");
	return 1;
}

int CharsetMigrationTool::IsMigrationNeeded(const string& path_to_frm)
{
	string tab_name = path_to_frm.substr(path_to_frm.find_last_of(DIR_SEPARATOR_STRING) + 1, path_to_frm.length() - path_to_frm.find_last_of(DIR_SEPARATOR_STRING) - 5);
	string frn_name_bkp = path_to_frm.substr(0, path_to_frm.length() - 4) + ".bht"  + DIR_SEPARATOR_STRING + "frm_bkp";
	if(DoesFileExist(frn_name_bkp))
		return 1;
	else {
		string ctb_filename = path_to_frm.substr(0, path_to_frm.find_last_of(".")) + ".bht" + DIR_SEPARATOR_STRING + "Table.ctb";
		fstream f(ctb_filename.c_str(), ios::binary | ios::in | ios::out);
		if(f) {
			char buf[5] = {};
			f.read(buf, 5);
			if(buf[4] >= '3') {
				f.close();
				return 2;
			}
			f.close();
		} else
			return -1;
	}
	return 0;
}

pair<int, int> CharsetMigrationTool::MigrateCS(const string& path_to_frm)
{
	pair<int, int> result;
	fstream f(path_to_frm.c_str(), ios::binary | ios::in | ios::out);
	if(f) {
		PrepareCharsetsToUpdate(f);
		result = UpdateCharsets();
		if(result.first==1) {
			StoreCharsets(f);			
		} else {
			charsets_to_update.clear();			
		}
		f.close();		
	} else {
		WriteToLog("File " + path_to_frm + " could not be opened. No charset migration for corresponding table was done.");
		result.first = 0;		
	}
	charsets_to_update.clear();
	return result;
}

void CharsetMigrationTool::PrepareCharsetsToUpdate(fstream& frm_file)
{
	char c = 0;
	uchar	old_c = 0;
	uchar 	type = 0;
	ushort	no_columns = 0;
	ushort	pos = 0;

	frm_file.seekp(2, ios::beg);
	frm_file.read(&c, 1);
	uint field_pack_length = (c - FRM_VER) < 2 ? 11 : 17;

	frm_file.seekp(38, ios::beg);
	frm_file.read((char*)&old_c, 1); // Table default collation
	charsets_to_update.push_back(std::make_pair((fstream::off_type)frm_file.tellg() - 1, (uchar)old_c));

	frm_file.seekp(8450, ios::beg);
	frm_file.read((char*)&no_columns, 2);
	frm_file.read((char*)&pos, 2);

	frm_file.seekp(8480 + pos, ios::beg);

	for(int i = 0; i < no_columns; i++) {
		old_c = 0;
		frm_file.seekp(13, ios::cur);
		frm_file.read((char*)&type, 1);		//Get column type
		frm_file.read((char*)&old_c, 1);	//Get collation's id
		if(type != MYSQL_TYPE_GEOMETRY && old_c != 0) {
			charsets_to_update.push_back(std::make_pair((fstream::off_type)frm_file.tellg() - 1, (uchar)old_c));
		}
		if(i != no_columns -1)
			frm_file.seekp(field_pack_length - 15, ios::cur);
	}
}

pair<int, int> CharsetMigrationTool::UpdateCharsets()
{
	pair<int, int> result = pair<int, int>(1,-1);
	std::vector<std::pair<fstream::off_type, uchar> >::iterator iter = charsets_to_update.begin();
	std::vector<std::pair<fstream::off_type, uchar> >::iterator end = charsets_to_update.end();
	std::map<int,int>::iterator conv;
	for(; iter != end; ++iter) {
		if((conv = ch_conv_map->find(iter->second)) != ch_conv_map->end()) {
			iter->second = (uchar)conv->second;
		} else {
			result.first = -1;
			result.second = (int)(iter->second);
			break;			
		}
	}
	return result;
}

void CharsetMigrationTool::StoreCharsets(fstream& frm_file)
{
	std::vector<std::pair<fstream::off_type, uchar> >::iterator iter = charsets_to_update.begin();
	std::vector<std::pair<fstream::off_type, uchar> >::iterator end = charsets_to_update.end();
	for(; iter != end; ++iter) {
		try {
			frm_file.seekp(iter->first, ios::beg);
			char c = iter->second;
			frm_file.write(&c, 1);
		} catch(...) {
			//TODO: errorek
		}
	}
}
