/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// \file UnitTest.h
///
/// @author Piotr Kuchta, ECMWF, Jan 2011

#include "odblib/FastODA2Request.h"

#include "UnitTest.h"

using namespace std;
using namespace eckit;


namespace odb {
namespace tool {
namespace test {



(int argc, char **argv)
: UnitTest(argc, argv)
{}

() { }

void UnitTest::setUp() {}

const char * cfg = 
//"CLASS: class\n"
"DATE: andate@desc\n"
"TIME: antime@desc\n"
"TYPE: type\n"
//"REPORTYPE: reportype\n"
//"STREAM: stream\n"
"EXPVER: expver\n"
;

const char * cfgFile = "/tmp/p4/mars/server/dev/oda/mars/marsKeywordToODBColumn";

void UnitTest::test()
{
	FastODA2Request<ODA2RequestClientTraits> o;
	o.parseConfig(cfg);
	o.scanFile("2000010106.2.0.odb");

	//o.parseConfig(StringTool::readFile(cfgFile));
	//o.scanFile("/hugetmp/mondb_conv.17.16001.odb");
	string r = o.genRequest();
	unsigned long long n = o.rowsNumber();

	Log::info() << "UnitTest::test: request is:" << endl << r << std::endl;
	Log::info() << "UnitTest::test: file has " << n << " rows(s)." << std::endl;

	ASSERT(o.getValues("DATE").size() == 1);
	ASSERT(*o.getValues("DATE").begin() == "20000101");

	ASSERT(o.getValues("TIME").size() == 1);
	ASSERT(*o.getValues("TIME").begin() == "60000");

	ASSERT(o.getValues("TYPE").size() == 1);
	ASSERT(*o.getValues("TYPE").begin() == "MISSING");

	map<string, double> m = o.getUniqueValues();
	ASSERT(m["TYPE"] == odb::MDI::realMDI());
	ASSERT(m["DATE"] == 20000101);
	ASSERT(m["TIME"] == 60000);
}


void UnitTest::tearDown() { }

} // namespace test 
} // namespace tool 
} // namespace odb 


