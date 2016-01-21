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

#ifndef DATAPROESSOR_ENSURE_H_INCLUDED
#define DATAPROESSOR_ENSURE_H_INCLUDED 1

#include <sstream>

#include "setup.h"

#define ENSURE( condition, comment ) \
	do { \
		if ( ! ( condition ) ) { \
			throw_exception( #condition, __FILE__, __LINE__, ( comment ) ); \
		} \
	} while( 0 )

inline void throw_exception( std::string const& reason_, char const* path_, int line_, std::string const& comment_ )
{
#ifndef NDEBUG
	if ( setup.verbose ) {
		std::stringstream msg;
		msg << "Runtime error: " << reason_ << ": \"" << comment_ << "\"" << ", from: " << path_ << ":" << line_;
		throw std::runtime_error( msg.str() );
	} else
		throw std::runtime_error( "Runtime error: " + comment_ );
#else /* #ifndef NDEBUG */
	throw std::runtime_error( "Runtime error: " + comment_ );
#endif /* #else #ifndef NDEBUG */
}

#endif /* #ifndef DATAPROESSOR_ENSURE_H_INCLUDED */

