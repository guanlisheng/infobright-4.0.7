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
#include <boost/bind.hpp>

#include "DIMMethod.h"
#include "DIMParameters.h"
#include "system/Channel.h"
#include "system/ChannelOut.h"
#include "system/StdOut.h"
#include "system/FileOut.h"

using namespace std;
using namespace boost;

DIMMethod::DIMMethod(const boost::function0<int>& method, const std::string& description)
	:	method(method), description(description)
{
}

DIMMethod::DIMMethod(const boost::function0<void>& method, const std::string& description)
	:	method(bind(&DIMMethod::VoidFunctionAdaptor, method)), description(description)
{
}

int DIMMethod::Run()
{
	DIMParameters::LogOutput() << lock << description << "... ";
	int method_returned = -1;
	try {
		method_returned = method();
	} catch(std::exception& e) {
		DIMParameters::LogOutput() << "[ FAILED ]" << unlock;
		DIMParameters::LogOutput() << lock << "ERROR: " << e.what() << unlock;
		return -1;
	}

	if(method_returned < 0)
		DIMParameters::LogOutput() << "[ FAILED ]";
	else
		DIMParameters::LogOutput() << "[ OK ]";
	DIMParameters::LogOutput() << unlock;
	return method_returned;
}

int DIMMethod::VoidFunctionAdaptor(boost::function0<void> method)
{
	method();
	return 1;
}
