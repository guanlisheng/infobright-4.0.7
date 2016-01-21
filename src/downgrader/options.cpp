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

#define _GL_ERRNO_H 1
#include <iostream>
#include <boost/program_options.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <ctype.h>


#include "options.h"
#include "setup.h"
#include "ensure.h"
#include "trycatch.h"
#include "common/bhassert.h"

using namespace std;
using namespace boost;

int process_options( int argc_, char** argv_ ) {
	try {
		string configPathStr;
		filesystem::path configPath;
		string logPath;
		program_options::options_description options;
		options.add_options()
			( "datadir,D", program_options::value<string>( &setup.datadir ), "Infobright server data directory." )
			( "cleanup,C", "Remove backup files created by downgrade operation." )
			( "version,V", "Print program version number and exit." )
			( "quiet,q", "Print fewer number of informational messages." )
			( "verbose,v", "Print more information about program execution." )
			( "help,h", "Show this help message." );

		program_options::variables_map vm;
		program_options::store( program_options::parse_command_line( argc_, argv_, options ), vm );
		program_options::notify( vm );
		if ( vm.count( "help" ) > 0 ) {
			cout << options << endl;
			throw ( 0 );
		}

		if ( ! logPath.empty() )
			setup.logPath = logPath;
		setup.cleanup = ( vm.count( "cleanup" ) > 0 );
		if ( vm.count( "version" ) > 0 ) {
			cout << "version=1.0.0" << ", " << MYSQL_COMPILATION_COMMENT << endl;
			throw ( 0 );
		}
		setup.verbose = vm.count( "verbose" ) > 0;
		setup.quiet = vm.count( "quiet" ) > 0;
		return ( 0 );
	} catch ( ... ) {
		log_catch( __PRETTY_FUNCTION__ );
	}
	return ( -1 );
}
