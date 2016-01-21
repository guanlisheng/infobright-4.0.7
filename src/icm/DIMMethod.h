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

#ifndef DIM_METHOD_H
#define DIM_METHOD_H

#include <iostream>
#include <string>
#include <boost/function.hpp>

class DIMMethod
{
public:
	DIMMethod(const boost::function0<int>& method, const std::string& description);
	DIMMethod(const boost::function0<void>& method, const std::string& description);

public:
	int Run();
	int operator()() { return Run(); }

private:
	boost::function0<int> method;
	std::string description;

private:
	static int VoidFunctionAdaptor(boost::function0<void> method);
};

#endif
