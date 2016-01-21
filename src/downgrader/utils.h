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

#ifndef DOWNGRADER_UTILS_H_INCLUDED
#define DOWNGRADER_UTILS_H_INCLUDED 1

#include <vector>
#include <boost/regex.hpp>

struct file_find_type {
	typedef enum {
		REGULAR_FILE = 1,
		DIRECTORY = 2,
		ALL = REGULAR_FILE | DIRECTORY
	} enum_t;
};

std::vector<std::string> find( std::string const& in, boost::regex const& pattern,
		int min_depth = 0, int max_depth = INT_MAX,
		file_find_type::enum_t file_type = file_find_type::ALL );

#endif /* DOWNGRADER_UTILS_H_INCLUDED */

