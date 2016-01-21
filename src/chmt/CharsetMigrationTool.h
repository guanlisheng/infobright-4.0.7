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
02111-1307 USA  */
#ifndef CHARSET_MIGRATION_H
#define CHARSET_MIGRATION_H

#include <iostream>
#include <fstream>
#include <utility>
#include <map>
#include <boost/bind.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>

#include "system/RCException.h"
#include "system/Channel.h"
#include "system/ChannelOut.h"
#include "system/StdOut.h"
#include "system/FileOut.h"
#include "system/IBFile.h"
#include "system/IBFileSystem.h"

class CHMTParameters;

class CharsetMigrationTool
{
public:	
	CharsetMigrationTool(const CHMTParameters& ic_params);

public: 
	int Run();
private:
	std::pair<int, int>	 MigrateCS(const std::string& path_to_frm);
	void PrepareCharsetsToUpdate(std::fstream& frm_file);
	std::pair<int, int> UpdateCharsets();
	void StoreCharsets(std::fstream& frm_file);
	int IsMigrationNeeded(const std::string& path_to_frm);
	
private:
	void LoadCharsetConversionSymbols(const std::string& path);
	void WriteToLog(const std::string& report);
	std::string data_dir;	
	std::string database_name;
	std::string table_name;	
	std::string conv_map_path;	
	bool print_help;
	bool print_version;	
	boost::shared_ptr<std::map<int,int> > ch_conv_map;

	std::vector<std::pair<std::fstream::off_type, uchar> > charsets_to_update; //
};

#endif
