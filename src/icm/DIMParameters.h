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

#ifndef DIM_PARAMETERS_H
#define DIM_PARAMETERS_H

#define DIM_VERSION "1.1"

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/program_options.hpp>
#include "DataIntegrityTest.h"

namespace po = boost::program_options;

class ChannelOut;
class Channel;

class FailedTest
{
public:
	FailedTest(std::string dbname, std::string tblname, std::string testname) :
		database_name(dbname), table_name(tblname),test_name(testname)
	{
	};
	std::string database_name;
	std::string table_name;
	std::string test_name;
};

class DIMParameters
{
private:
	DIMParameters();

public:
	const std::string&	DataDirPath() const 			{ return data_dir; }
	const std::string&	KNFolderPath() const 			{ return kn_folder; }
	const std::string&	TableName() const 				{ return table_name; }
	const std::string&	DatabaseName() const 			{ return database_name; }
	const std::string&	ColumnName() const 				{ return column_name; }
	bool				FullScan() const				{ return full; }
	bool				WithDataFormatReport() const	{ return report; }
	bool				ScanWithRepair() const			{ return repair; }
	const std::string&	ExternalTool() const			{ return ext_tool; }

	bool				IsPrintHelpParamSet() const	{ return vm.count("help"); }
	bool				IsVersionParamSet()	const	{ return vm.count("version"); }

	void 				PrintHelpMessage(std::ostream& os) const;
	void 				PrintVersionMessage(std::ostream& os) const;

private:
	void 				PrepareLogOutput();
	void 				PrepareParameters();

private:
	std::string	data_dir;
	std::string kn_folder;
	std::string database_name;
	std::string table_name;
	std::string column_name;
	bool        print_help;
	bool		print_version;
	bool		full;
	bool		report;
	bool		repair;
	std::string	ext_tool;

	std::string log_file_name;

	po::variables_map vm;
	po::options_description desc;

	static boost::shared_ptr<ChannelOut>	channel;
	static boost::shared_ptr<Channel>		log_output;

public:
	static Channel&	LogOutput()	{ return *log_output; }
	static std::auto_ptr<DIMParameters> CreateDIMParameters(int argc, char** argv);
	static std::vector<FailedTest> failed_tests;
};

#endif
