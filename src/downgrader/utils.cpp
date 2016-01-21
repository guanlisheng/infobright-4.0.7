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

#include <boost/filesystem.hpp>

#include "utils.h"

using namespace std;
using namespace boost;
using namespace boost::filesystem;

std::vector<std::string> find( std::string const& in, boost::regex const& pattern,
		int min_depth, int max_depth, file_find_type::enum_t file_type ) {
	std::vector<std::string> result;
	path p( in );
	if ( exists( p ) ) {
		if ( is_regular_file( p ) ) {
			if ( ( min_depth == 0 ) && ( file_type & file_find_type::REGULAR_FILE ) && regex_match( in, pattern ) ) {
				result.push_back( in );
			}
		} else if ( is_directory( p ) ) {
			for ( directory_iterator it( p ), end; it != end; ++ it ) {
				if ( is_regular_file( *it ) ) {
					if ( ( min_depth == 0 ) && ( file_type & file_find_type::REGULAR_FILE ) && regex_match( it->path().string(), pattern ) )
						result.push_back( it->path().string() );
				} else if ( is_directory( *it ) ) {
					if ( ( min_depth == 0 ) && ( file_type & file_find_type::DIRECTORY ) && regex_match( it->path().string(), pattern ) )
						result.push_back( it->path().string() );
					if ( max_depth > 0 ) {
						std::vector<std::string> sub( find( it->path().string(), pattern, min_depth > 0 ? min_depth - 1 : 0, max_depth - 1 ) );
						result.insert( result.end(), sub.begin(), sub.end() );
					}
				}
			}
		}
	}
	return ( result );
}

