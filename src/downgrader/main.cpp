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

#include <fstream>
#include <boost/lexical_cast.hpp>

#include "trycatch.h"
#include "setup.h"
#include "options.h"
#include "ensure.h"
#include "migrator.h"
#include "common/CommonDefinitions.h"
#include "system/MemoryManagement/Initializer.h"
#include "edition/core/SavableFilter.h"
#include "system/DatadirVersion.cpp"

using namespace std;
using namespace boost;

static int const BEFORE_MIGRATOR(2);

void downgrade_to_2();

int main(int argc, char** argv) try {
	do {
		migrators_t& migrators( Migrators::GetInstance() );
		process_type = ProcessType::DOWNGRADER;
		MemoryManagerInitializer::Instance(100, 100, 100 / 10, 100 / 10);		
		the_filter_block_owner = new TheFilterBlockOwner();
		process_options(argc, argv);
		setup.test();
		if( setup.cleanup ) {
			downgrade_to_2();
			break;
		}
		int migrate_to( BEFORE_MIGRATOR );
		DatadirVersion datadir_ver(setup.datadir);
		int current_version(datadir_ver.GetDataVersion());
		if( current_version == migrate_to && migrate_to == BEFORE_MIGRATOR ) {
			cout << "The above specified data directory is already in version prior to 4.0.7. Downgrade is not needed." << endl;
			break;
		}
		if( current_version != migrate_to ) {
			migrators_t::const_iterator from( migrators.find( current_version + ( migrate_to > current_version ? 1 : -1 ) ) );
			migrators_t::const_iterator to( migrators.find( migrate_to ) );
			ENSURE( from != migrators.end(), "Downgrader does not support your current Infobright Datadir Version." );
			ENSURE( to != migrators.end(), "Downgrader does not support requested Infobright Datadir Version." );
//			ENSURE( from != migrators.end(), "Migrator does not support your current Infobright Datadir Version." );
//			ENSURE( to != migrators.end(), "Migrator does not support requested Infobright Datadir Version." );
			if ( migrate_to > current_version ) {
				++ to;
				for ( ; from != to; ++ from ) {
					Migrator const& migrator( from->second );
					if ( migrator.upgrader )
						(migrator.upgrader)();
				}
			} else {
				++ from;
				++ to;
				migrators_t::const_reverse_iterator rfrom( from );
				migrators_t::const_reverse_iterator rto( to );
				++ rto;
				for ( ; rfrom != rto; ++ rfrom ) {
					Migrator const& migrator( rfrom->second );
					if ( migrator.downgrader )
						(migrator.downgrader)();
				}
			}
		} else {
			cout << "Downgrade is not needed." << endl;
		}
	} while ( false );
	return ( 0 );
} catch ( ... ) {
	return ( log_catch( __PRETTY_FUNCTION__, true ) );
}


