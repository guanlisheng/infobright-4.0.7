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

#include <iostream>
#include <algorithm>
#include <iterator>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>

#include "trycatch.h"
#include "setup.h"
#include "utils.h"
#include "migrator.h"
#include "common/CommonDefinitions.h"
#include "system/IBFileSystem.h"
#include "edition/core/SavableFilter.h"

using namespace std;
using namespace boost;
using namespace boost::algorithm;
using namespace boost::filesystem;

class SavableFilterConverter : public SavableFilter
{
	std::string path;
public:
	SavableFilterConverter( std::string const& path )
		: SavableFilter( path.c_str() ), path( path )
		{}
	void Convert();
	static uchar GetFileFormat(std::string const& path);
};

uchar SavableFilterConverter::GetFileFormat(std::string const& path)
{
	IBFile f;
	f.OpenReadOnly(path);
	Header header;
	f.ReadExact(&header, sizeof(Header));
	return ( header.v );
}

void SavableFilterConverter::Convert()
{
	assert(track_changes);
	for(int i = 0; i < (no_blocks - 1); ++ i) {
		if ( ( block_status[i] == FB_FULL ) && ( block_last_one[i] != 0xffff ) ) {
			blocks[i] = block_allocator->Alloc();
			if(blocks[i] == NULL)
				throw OutOfMemoryRCException();
			new(blocks[i]) Block(this);
			blocks[i]->Set(0, block_last_one[i]);		// create a block with a contents before Set
			block_status[i] = FB_MIXED;
			was_block_changed[i] = true;
		}
	}
	if ( no_blocks > 0 ) {
		int last_block(no_blocks - 1);
		if ( ( block_status[last_block] == FB_FULL ) && ( block_last_one[last_block] != ( no_of_bits_in_last_block - 1 ) ) ) {
			blocks[last_block] = block_allocator->Alloc();
			if(blocks[last_block] == NULL)
				throw OutOfMemoryRCException();
			new(blocks[last_block]) Block(this, no_of_bits_in_last_block);
			blocks[last_block]->Set(0, block_last_one[last_block]);		// create a block with a contents before Set
			block_status[last_block] = FB_MIXED;
			was_block_changed[last_block] = true;
		}
	}
	assert(track_changes);

	char buf[COMPRESSED_BIT_BLOCK_SIZE];

	IBFile f;
	IBFile f_orig;

	std::string ftmp(path);
	ftmp += ".tmp";

	f.OpenCreateEmpty(ftmp.c_str());
	f_orig.OpenReadOnly(path);

	Header header;

	header.v = file_format_v1;
	header.nob = no_blocks;
	header.bil = no_of_bits_in_last_block;

	// read in original filter header
	uchar* block_status_orig = 0;

	Header header_orig;
	try {
		f_orig.ReadExact(&header_orig, sizeof(Header));
		block_status_orig = new uchar[header_orig.nob];
		f_orig.ReadExact(block_status_orig, header_orig.nob * sizeof(uchar));
		f_orig.Seek(no_blocks * sizeof(ushort), SEEK_CUR);

		f.WriteExact(&header, sizeof(Header));
		f.WriteExact(block_status, no_blocks);

		for(uint i = 0; i < (uint)no_blocks; i++) {
			// read in original block header
			BHeader bheader;

			if(i < header_orig.nob && block_status_orig[i] == FB_MIXED)
				f_orig.ReadExact(&bheader, sizeof(bheader));

			if(block_status[i] == FB_MIXED) {
				if(WasBlockChanged(i)) {
					static_cast<SavableBlock*>(blocks[i])->Store(f);
					if(i < header_orig.nob && block_status_orig[i] == FB_MIXED) {
						f_orig.Seek(bheader.len, SEEK_CUR);
					}
				} else {
					BHASSERT_WITH_NO_PERFORMANCE_IMPACT(i < header_orig.nob);
					f.WriteExact(&bheader, sizeof(bheader));
					f_orig.ReadExact(buf, bheader.len);
					f.WriteExact(&buf, bheader.len);
				}
			} else { // block_status in (FB_EMPTY, FB_FULL)
				if(i < header_orig.nob && block_status_orig[i] == FB_MIXED)
					f_orig.Seek(bheader.len, SEEK_CUR);
			}
		}
		delete [] block_status_orig;
	} catch( ... ) {
		delete [] block_status_orig;
		throw;
	}
	f.Flush();
	f.Close();
	f_orig.Close();

	// set block statuses for unchanged
	was_block_changed.reset();
}

struct TableDowngrader
{
	typedef std::vector<std::string> di_tables_t;
	int errors;
	int backups;
	di_tables_t di_tables;
	TableDowngrader() 
		: errors(0), backups(0)
		{}
	void DowngradeTable( std::string const& table );
	bool DowngradeDeleteMask( std::string const& delete_mask );
	void CleanupTableBackup( std::string const& table );
	bool CleanupDeleteMaskBackup( std::string const& delete_mask );
};

bool TableDowngrader::DowngradeDeleteMask( std::string const& delete_mask )
{
	bool downgraded( false );
	if ( setup.verbose )
		cout << "\nProcessing `" << delete_mask << "' ... " << flush;
	if ( SavableFilterConverter::GetFileFormat(delete_mask) == SavableFilter::file_format_v2 ) {
		SavableFilterConverter sfc( delete_mask.c_str() );
		sfc.Convert();
		downgraded = true;
	}
	if ( setup.verbose )
		cout << ( downgraded ? "[ Done ]" : "[ Not needed ]" ) << flush;
	return ( downgraded );
}

void TableDowngrader::DowngradeTable( std::string const& table )
{
	string tableName(table.substr( setup.datadir.length() + ( table[setup.datadir.length()] == DIR_SEPARATOR_CHAR ? 1 : 0 ) ));
	tableName = tableName.substr(0, tableName.length() - 4 );
	if ( ! setup.quiet )
		cout << "Downgrade of `" << tableName << "' table ... " << flush;
	std::vector<std::string> di_files( find( table, regex( ".*DI\\.ctb" ), 0, 0, file_find_type::REGULAR_FILE ) );
	if ( ! di_files.empty() )
		di_tables.push_back( tableName );
	path p1( table + DIR_SEPARATOR_STRING + "del_mask.flt" );
	path p2( table + DIR_SEPARATOR_STRING + "del_mask_alt.flt" );
	string p1s(p1.string());
	string p2s(p2.string());
	try {
		bool d1r( false );
		if ( exists( p1 ) && is_regular_file( p1 ) )
			d1r = DowngradeDeleteMask( p1.string() );
		bool d2r( false );
		if ( exists( p2 ) && is_regular_file( p2 ) )
			d2r = DowngradeDeleteMask( p2.string() );
		if ( d1r ) {
			RenameFile(p1s, p1s + ".bck");
			RenameFile(p1s + ".tmp", p1s);
		}
		if ( d2r ) {
			RenameFile(p2s, p2s + ".bck");
			RenameFile(p2s + ".tmp", p2s);
		}
		if ( d1r || d2r ) {
			string table_dir(setup.datadir + DIR_SEPARATOR_STRING + tableName + ".bht");
			FlushDirectoryChanges(table_dir);
		}
		if ( ! setup.quiet )
			cout << ( ( d1r || d2r ) ? "[ Finished ]" : "[ Not needed ]" ) << endl;
	} catch ( ... ) {
		if ( ! setup.quiet )
			cout << "[ Failed ]" << endl;
		RemoveFile( p1s + ".tmp" );
		RemoveFile( p2s + ".tmp" );
		throw;
	}
}

bool TableDowngrader::CleanupDeleteMaskBackup( std::string const& delete_mask ) {
	if ( setup.verbose )
		cout << "Cleanup of `" << delete_mask << "' ... " << flush;
	bool cleaned(exists( delete_mask ));
	if ( cleaned )
		++ backups;
	RemoveFile( delete_mask );
	if ( setup.verbose )
		cout << ( cleaned? "[ Finished ]" : "[ Not needed ]" ) << endl;
	return ( cleaned );
}

void TableDowngrader::CleanupTableBackup( std::string const& table ) {
	string tableName(table.substr( setup.datadir.length() + ( table[setup.datadir.length()] == DIR_SEPARATOR_CHAR ? 1 : 0 ) ));
	tableName = tableName.substr(0, tableName.length() - 4 );
	if ( ! setup.quiet )
		cout << "Cleanup of `" << tableName << "' table ... " << flush;
	path p1( table + DIR_SEPARATOR_STRING + "del_mask.flt.bck" );
	path p2( table + DIR_SEPARATOR_STRING + "del_mask_alt.flt.bck" );
	string p1s(p1.string());
	string p2s(p2.string());
	bool c1(CleanupDeleteMaskBackup( p1s ));
	bool c2(CleanupDeleteMaskBackup( p2s ));
	if ( ! setup.quiet )
		cout << (( c1 || c2 ) ? "[ Finished ]" : "[ Not needed ]" ) << endl;
}

void downgrade_to_2()
{
	if ( ! setup.cleanup ) {
		cout <<
			"This operation will downgrade all necessary data in the above specified data directory,\n"
			"so that it can be used with Infobright versions prior to 4.0.7.\n"
			"Do you want to continue [yes/no]? " << flush;
	} else {
		cout <<
			"This operation will remove all backup data created during any past downgrade operation.\n"
			"Do you want to continue [yes/no]? " << flush;
	}
	string line;
	while ( ! getline( cin, line ).fail() ) {
		to_lower( line );
		if ( ( line != "yes" ) && ( line != "no" ) ) {
			cout << "Please answer `yes' or `no': " << flush;
		} else
			break;
	}
	if ( line == "yes" ) {
		if ( ! setup.quiet )
			cout << "Proceeding ..." << endl;
		std::vector<std::string> tables( find( setup.datadir, regex( ".*\\.bht" ), 1, 1, file_find_type::DIRECTORY ) );
		TableDowngrader d;
		if ( ! setup.cleanup ) {
			try {
				for_each( tables.begin(), tables.end(), bind( &TableDowngrader::DowngradeTable, &d, _1 ) );
				if ( ! d.di_tables.empty() ) {
					cout << "Warning: Downgrader found tables with Domain Expert used:" << endl;
					stringstream ss;
					ss << endl;
					sort( d.di_tables.begin(), d.di_tables.end() );
					copy( d.di_tables.begin(), d.di_tables.end(), ostream_iterator<string>( cout, ss.str().c_str() ) );
					cout << "If you are downgrading to version prior to 4.0 GA the above tables will be unavailable." << endl;
				}
				if ( ! setup.quiet )
					cout << "All done!" << endl;
				RemoveFile( setup.datadir + DIR_SEPARATOR_STRING + "ib_data_version" );
			} catch ( std::exception const& e ) {
				cerr << "Downgrading aborted due to error: " << e.what() << endl;
			}
		} else {
			for_each( tables.begin(), tables.end(), bind( &TableDowngrader::CleanupTableBackup, &d, _1 ) );
			if ( d.backups > 0 ) {
				if ( ! setup.quiet )
				cout << "All done!" << endl;
			} else {
				if ( ! setup.quiet )
					cout << "There is nothing to clean." << endl;
			}
		}
	} else {
		if ( ! setup.quiet )
			cout << "Aborting execution per user request." << endl;
	}
}

namespace {

bool RegisterMigrator()
{
	migrators_t& migrators( Migrators::GetInstance() );
	volatile bool registered( false );
	migrators[2].downgrader = downgrade_to_2;
	registered = true;
	return ( registered );
}

}

volatile bool registered = RegisterMigrator();

