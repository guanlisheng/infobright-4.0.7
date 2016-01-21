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
#include <iostream>
#include <string.h>
#include <algorithm>
#include <vector>
#include <iterator>
#include <time.h>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem/operations.hpp>

#include "CharsetMigrationTool.h"
#include "CHMTParameters.h"
#include "common/CommonDefinitions.h"
#include "system/Channel.h"
#include "system/MemoryManagement/Initializer.h"

#define CHARSET_MIGRATION_TOOL_TEST

using namespace std;

int main(int argc, char** argv)
{
	int r = 1;
	try {
		std::auto_ptr<CHMTParameters> icparams = CHMTParameters::CreateCHMTParameters(argc, argv);

		MemoryManagerInitializer::Instance(100, 100, 100 / 10, 100 / 10);

		if(icparams.get()) {
			if(icparams->IsPrintHelpParamSet()) {
				icparams->PrintHelpMessage(cout);
				return 0;
			} else if(icparams->IsVersionParamSet()) {
				icparams->PrintVersionMessage(cout);
				return 0;
			}

			int r = CharsetMigrationTool(*icparams).Run();

		}
	} catch (std::exception& e) {
		CHMTParameters::LogOutput() << lock << string("Character Set Migration Tool error: ") + e.what() << unlock;
	}
	return r;
}


