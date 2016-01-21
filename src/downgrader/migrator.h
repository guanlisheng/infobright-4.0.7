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

#ifndef DOWNGRADER_MIGRATOR_H_INCLUDED
#define DOWNGRADER_MIGRATOR_H_INCLUDED 1

#include <map>

typedef void (*migrator_impl_t)(void);

struct Migrator
{
	migrator_impl_t upgrader;
	migrator_impl_t downgrader;
	Migrator()
		: upgrader(0), downgrader(0)
		{}
	Migrator( migrator_impl_t upgrader_, migrator_impl_t downgrader_ )
		: upgrader( upgrader_ ), downgrader( downgrader_ )
		{}
};

typedef std::map<int, Migrator> migrators_t;

class Migrators
{
public:
	static migrators_t& GetInstance() {
		static migrators_t migrators;
		return ( migrators );
	}
};


#endif /* DOWNGRADER_MIGRATOR_H_INCLUDED */

