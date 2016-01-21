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
#ifndef INTEGRITY_CHECKER_H
#define INTEGRITY_CHECKER_H

#include <iostream>
#include <utility>
#include <boost/bind.hpp>
//#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>

#include "DIMParameters.h"
#include "TableTest.h"
#include "system/RCException.h"
#include "system/Channel.h"
#include "system/ChannelOut.h"
#include "system/StdOut.h"
#include "system/FileOut.h"
#include "system/IBFile.h"
#include "system/IBFileSystem.h"

class DataIntegrityManager
{
public:	
	DataIntegrityManager(const DIMParameters& ic_params);

public: 
	int Run();
	
private:
	void WriteToLog(const std::string& report);
	std::string data_dir;
	std::string kn_folder;
	std::string database_name;
	std::string table_name;
	std::string column_name;
	bool print_help;
	bool print_version;
	bool full;
	bool report;
	bool repair;
	std::string ext_tool;
};

#endif
