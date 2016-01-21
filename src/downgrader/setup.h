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

#ifndef DOWNGRADER_SETUP_H_INCLUDED
#define DOWNGRADER_SETUP_H_INCLUDED 1

#include <string>
#include <boost/filesystem.hpp>

namespace defaults
{

extern boost::filesystem::path const LOG_PATH;

}

struct Setup
{
	boost::filesystem::path logPath;
	std::string datadir;
	bool cleanup;
	bool verbose;
	bool quiet;
	Setup( void )
		: logPath( defaults::LOG_PATH ),
		datadir(),
		cleanup( false ),
		verbose( false ),
		quiet( false )
		{ }
	void test( void );
} extern setup;

#endif /* DOWNGRADER_SETUP_H_INCLUDED */
