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
#include <stdexcept>
#include <iostream>

#include "trycatch.h"
#include "setup.h"
#include "common/bhassert.h"
#include "system/RCSystem.h"

using namespace std;

int log_catch( char const* const frame_, bool final_ )
{
	int error( -1 );
	try {
		throw;
	} catch ( std::exception const& e ) {
		if ( final_ ) {
			cerr << ( setup.verbose ? "Program terminated by an exception: " : "" ) << e.what() << endl;
			rclog << lock << e.what() << unlock;
		} else {
#ifndef NDEBUG
			if ( setup.verbose )
				cout << "An exception at: " << frame_ << ", caught: " << e.what() << endl;
			rclog << lock << e.what() << unlock;
#endif /* #ifndef NDEBUG */
			throw;
		}
	} catch ( FailedAssertion const& fa ) {
		if ( final_ )
			cerr << fa.what() << endl;
		else {
#ifndef NDEBUG
			if ( setup.verbose )
				cout << fa.what() << ", cought at: " << frame_ << endl;
#endif /* #ifndef NDEBUG */
			throw;
		}
	} catch ( int code ) {
		if ( final_ ) {
			if ( setup.verbose )
				cerr << "Program exited normally: " << code << endl;
			error = 0;
		} else {
#ifndef NDEBUG
			if ( setup.verbose )
				cout << "Frame manually unwinded at: " << frame_ << ", by code: " << code << endl;
#endif /* #ifndef NDEBUG */
			throw;
		}
	} catch ( ... ) {
		if ( final_ ) {
			char const unknownEx[] = "Program terminated by an unknown(!) exception.";
			cerr << unknownEx << endl;
			rclog << lock << unknownEx << unlock;
		} else {
#ifndef NDEBUG
			if ( setup.verbose )
				cout << "An unknown(!) exception caught at: " << frame_ << endl;
#endif /* #ifndef NDEBUG */
			throw;
		}
	}
	return ( error );
}

