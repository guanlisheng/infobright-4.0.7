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


#include <iostream>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem/operations.hpp>

#include "CHMTParameters.h"
#include "CharsetMigrationTool.h"
#include "core/RSI_Framework.h"
#include "system/Channel.h"
#include "system/ChannelOut.h"
#include "system/StdOut.h"
#include "system/FileOut.h"

using namespace std;
using namespace boost;
using namespace boost::filesystem;

shared_ptr<ChannelOut>	CHMTParameters::channel = shared_ptr<ChannelOut>(new StdOut());
shared_ptr<Channel>		CHMTParameters::log_output = shared_ptr<Channel>(new Channel(CHMTParameters::channel.get(), true));

CHMTParameters::CHMTParameters()
	:	print_help(false), print_version(false),
		desc("Character Set Migration Tool version: " + string(CHMT_VERSION) + ", options")
{
	desc.add_options()
		("help", 											"Display help message and exit.")
		("version,V", 										"Display version information and exit.")
		("conv-map",	po::value<string>(&conv_map_path),	"Absolute path to conversion map file.")
		("datadir",		po::value<string>(&data_dir), 		"Absolute path to data directory.")		
		("database",	po::value<string>(&database_name),	"Name of database chosen for data integrity testing.")
		("table",		po::value<string>(&table_name),		"Name of table chosen for data integrity testing.")		
		("log-file",	po::value<string>(&log_file_name),	"Print output to log file.")
	;
}

void CHMTParameters::PrintHelpMessage(std::ostream& os) const
{
	os << desc << "\n";
}

void CHMTParameters::PrintVersionMessage(std::ostream& os) const
{
	os << "Character Set Migration Tool version: " + string(CHMT_VERSION) << "\n";
}

void CHMTParameters::PrepareLogOutput()
{
	 if(vm.count("log-file")) {
		string ic_logfile_path = data_dir + log_file_name;
		channel = boost::shared_ptr<ChannelOut>(new FileOut(ic_logfile_path.c_str()));
		log_output = boost::shared_ptr<Channel>(new Channel(channel.get(), true));
	 }
}

void CHMTParameters::PrepareParameters(const string& argv0)
{
	if(!IsPrintHelpParamSet() && !IsVersionParamSet()) {
		if(!vm.count("datadir"))
			throw SystemRCException("No data directory specified.");
		else if (!exists(data_dir) || !is_directory(data_dir))
			throw SystemRCException("Specified data directory does not exist.");
		else if((*data_dir.rbegin()) != '/' && (*data_dir.rbegin()) != '\\')
			 data_dir += "/";

		if (vm.count("table") && !vm.count("database"))
			throw SystemRCException("Table name specified but lack of database name.");
		else {
			if(vm.count("database")){
				string database_path = data_dir + database_name;
				if(!exists(database_path) || !is_directory(database_path))
					throw SystemRCException("Specified database does not exist.");
				if(vm.count("table")){
					string table_path = database_path + DIR_SEPARATOR_CHAR + table_name + string(".bht");
					if(!exists(table_path) || !is_directory(table_path))
						throw SystemRCException("Specified table does not exist.");
				}					
			}			
		}

		if(vm.count("conv-map")) {
			if(!exists(conv_map_path) || is_directory(conv_map_path))
				throw SystemRCException("Unable to open conversion map file: " + conv_map_path);
		} else {
			string path_to_chmt = argv0.substr(0, argv0.find_last_of(DIR_SEPARATOR_STRING) + 1);
			path_to_chmt += string("..") + DIR_SEPARATOR_STRING + "support-files" + DIR_SEPARATOR_STRING + CHMT_CONV_FILE;	
			if(!exists(path_to_chmt)) {
				string path_to_chmt2 = argv0.substr(0, argv0.find_last_of(DIR_SEPARATOR_STRING) + 1);				
				path_to_chmt2 += CHMT_CONV_FILE;				
				if(!exists(path_to_chmt2)) {
					throw SystemRCException("No conversion map file founded " + path_to_chmt2);
				} else {
					conv_map_path = path_to_chmt2;	
				}
			} else {
				conv_map_path = path_to_chmt;			
			}			
		}
		PrepareLogOutput();
	}
}

auto_ptr<CHMTParameters> CHMTParameters::CreateCHMTParameters(int argc, char** argv)
{
	std::auto_ptr<CHMTParameters> chmtp = std::auto_ptr<CHMTParameters>(new CHMTParameters());

	try {
		po::store(po::parse_command_line(argc, argv, chmtp->desc), chmtp->vm);
		po::notify(chmtp->vm);

		chmtp->PrepareParameters(string(argv[0]));

	} catch (...) {
		throw;
	}
	return chmtp;
}
