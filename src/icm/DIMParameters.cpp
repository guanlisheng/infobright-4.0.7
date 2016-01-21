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



#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>

#include "DIMParameters.h"
#include "DataIntegrityManager.h"
#include "core/RSI_Framework.h"
#include "system/Channel.h"
#include "system/ChannelOut.h"
#include "system/StdOut.h"
#include "system/FileOut.h"

using namespace std;
using namespace boost;
using namespace boost::filesystem;

shared_ptr<ChannelOut>	DIMParameters::channel = shared_ptr<ChannelOut>(new StdOut());
shared_ptr<Channel>		DIMParameters::log_output = shared_ptr<Channel>(new Channel(DIMParameters::channel.get(), true));
vector<FailedTest> DIMParameters::failed_tests = vector<FailedTest>();

DIMParameters::DIMParameters()
	:	print_help(false), print_version(false),
		desc("Infobright Consistency Manager version: " + string(DIM_VERSION) + ", options")
{
	desc.add_options()
		("help", 											"Display help message and exit.")
		("version,V", 										"Display version information and exit.")
		("datadir",		po::value<string>(&data_dir), 		"Absolute path to data directory.")
		("knfolder",	po::value<string>(&kn_folder),		"Absolute path to KNFolder directory.")
		("database",	po::value<string>(&database_name),	"Name of database chosen for data integrity testing.")
		("table",		po::value<string>(&table_name),		"Name of table chosen for data integrity testing.")
		("column",		po::value<string>(&column_name),	"Name of column chosen for data integrity testing.")
		("log-file",	po::value<string>(&log_file_name),	"Print output to log file.")
		//("ext-tool",	po::value<string>(&ext_tool),		"Use additional tool.")
	;
}

void DIMParameters::PrintHelpMessage(std::ostream& os) const
{
	os << desc << "\n";
}

void DIMParameters::PrintVersionMessage(std::ostream& os) const
{
	os << "Infobright Consistency Manager version: " << DIM_VERSION << "\n";
}

//string DIMParameters::GetMigrateToHelpMessage() const
//{
//	std::string message("Version to which migration should be done.\nAvailable versions: ");
//	RSI_Manager::versions_map_type::iterator iter = RSI_Manager::versions_map.begin();
//	RSI_Manager::versions_map_type::iterator end = RSI_Manager::versions_map.end();
//	for(; iter != end; ++iter)
//		message += (iter->first + ((iter + 1) == end ? "" : ", "));
//	return message;
//}

void DIMParameters::PrepareLogOutput()
{
	 if(vm.count("log-file")) {
		channel = boost::shared_ptr<ChannelOut>(new FileOut(log_file_name.c_str()));
		log_output = boost::shared_ptr<Channel>(new Channel(channel.get(), true));
	 }
}

void DIMParameters::PrepareParameters()
{
	if(!IsPrintHelpParamSet() && !IsVersionParamSet()) {
		if(!vm.count("datadir"))
			throw SystemRCException("No data directory specified.");
		else if (!exists(data_dir) || !is_directory(data_dir))
			throw SystemRCException("Specified data directory does not exist.");
		else if((*data_dir.rbegin()) != '/' && (*data_dir.rbegin()) != '\\')
			 data_dir += "/";

		if((vm.count("column") && !vm.count("table")) || (vm.count("column") && !vm.count("database")))
			throw SystemRCException("Column name specified but lack of table and/or database name.");
		else if (vm.count("table") && !vm.count("database"))
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

		if(vm.count("knfolder") && (!exists(kn_folder) || !is_directory(kn_folder)))
				throw SystemRCException("Specified kn-folder does not exist.");

		//if(vm.count("ext-tool") && (ext_tool != "kn_rebuild"))
		//	throw SystemRCException("Unknown name of external tool.");

//		if(!vm.count("knfolder"))
//			throw SystemRCException("No kn-folder specified.");
//		else if(!exists(kn_folder) || !is_directory(kn_folder))
//			throw SystemRCException("Specified kn-folder does not exist.");
//		else if(((*kn_folder.rbegin()) != '/' && (*kn_folder.rbegin()) != '\\'))
//			kn_folder += "/";

//		if(!vm.count("migrate-to"))
//			throw SystemRCException("No target version specified.");
//		else {
//			trim(migrate_to);
//			if (RSI_Manager::GetRSIVersionCode(migrate_to) < 0)
//				throw SystemRCException("Unknown version identifier. Type 'updater --help' for available versions.");
//		}
		PrepareLogOutput();
	}
}

auto_ptr<DIMParameters> DIMParameters::CreateDIMParameters(int argc, char** argv)
{
	std::auto_ptr<DIMParameters> icp (new DIMParameters());
	po::store(po::parse_command_line(argc, argv, icp->desc), icp->vm);
	po::notify(icp->vm);
	icp->PrepareParameters();
	return icp;
}
