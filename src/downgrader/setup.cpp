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
#include <boost/lexical_cast.hpp>

#include "setup.h"
#include "ensure.h"
#include "trycatch.h"
#include "system/IBFileSystem.h"
#include "system/Configuration.h"

using namespace std;
using namespace boost;

namespace defaults
{

filesystem::path const LOG_PATH = "downgrader.log";

}

Setup setup;

/*
namespace
{

std::string KNFolderFromDataDir(std::string const& datadir) {
	infobright_data_dir = datadir + DIR_SEPARATOR_STRING;
	Configuration::LoadSettings();
	return (Configuration::GetProperty(Configuration::KNFolder));
}

}
*/

void Setup::test( void ) {
	try {
    ENSURE( ! ( verbose && quiet ), "quiet and verbose options are mutually exclusive" );
		ENSURE( ! filesystem::is_directory( logPath ), logPath.string() );
		ENSURE( ! datadir.empty(), "Infobright data directory must be specified with --datadir=/path/ option." );
		ENSURE( filesystem::is_regular_file( datadir + DIR_SEPARATOR_STRING + "brighthouse.ini" ), "Specified path: `" + datadir + "' is not Infobright data directory." );
	} catch ( ... ) {
		log_catch( __PRETTY_FUNCTION__ );
	}
}
