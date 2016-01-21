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

#include "DataIntegrityManager.h"
#include "DIMParameters.h"
#include "common/CommonDefinitions.h"
#include "system/Channel.h"
#include "system/MemoryManagement/Initializer.h"
#include "edition/core/GlobalDataCache.h"

#define DATA_INTEGRITY_TEST

using namespace std;

int main(int argc, char** argv)
{	
	process_type = ProcessType::DIM;
	int r = 1;
	try {
		auto_ptr<DIMParameters> icparams(DIMParameters::CreateDIMParameters(argc, argv));

		MemoryManagerInitializer::Instance(100, 100, 100 / 10, 100 / 10);		
		the_filter_block_owner = new TheFilterBlockOwner();

		if(icparams.get()) {
			if(icparams->IsPrintHelpParamSet()) {
				icparams->PrintHelpMessage(cout);
				return 0;
			} else if(icparams->IsVersionParamSet()) {
				icparams->PrintVersionMessage(cout);
				return 0;
			}
			r = DataIntegrityManager(*icparams).Run();
		}
		GlobalDataCache::GetGlobalDataCache().ReleaseAll();
		delete the_filter_block_owner;
		the_filter_block_owner = 0;

	} catch (std::exception& e) {		
		DIMParameters::LogOutput() << lock << string("Infobright Consistency Manager error: ") + e.what() << unlock;
		r = 1;
	}
	return r;
}


