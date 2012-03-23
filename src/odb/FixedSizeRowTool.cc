/*
 * © Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "odblib/oda.h"

#include "Tool.h"
#include "ToolFactory.h"
#include "odblib/Comparator.h"
#include "FixedSizeRowTool.h"

namespace odb {
namespace tool {

ToolFactory<FixedSizeRowTool> fixedSizeRow("fixrowsize");

FixedSizeRowTool::FixedSizeRowTool (int argc, char *argv[]) : Tool(argc, argv) { }

void FixedSizeRowTool::run()
{
	if (parameters().size() != 3)
	{
		Log::error() << "Usage: ";
		usage(parameters(0), Log::error());
		Log::error() << endl;
		return;
	}

	PathName inFile = parameters(1);
	PathName outFile = parameters(2);

	odb::Reader in(inFile);
	odb::Writer<> out(outFile);

	odb::Reader::iterator it = in.begin();
	odb::Reader::iterator end = in.end();
	odb::Writer<>::iterator outIt(out.begin());
	outIt->pass1(it, end);
	
	odb::Reader outReader(outFile);
    Log::info() << "Verifying." << endl;
    odb::Reader::iterator it1 = in.begin();
    odb::Reader::iterator end1 = in.end();
    odb::Reader::iterator it2 = outReader.begin();
    odb::Reader::iterator end2 = outReader.end();
    odb::Comparator comparator;
    comparator.compare(it1, end1, it2, end2, inFile, outFile);
}

} // namespace tool 
} // namespace odb 

