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
/// @author Piotr Kuchta, ECMWF, Feb 2009

#include "eckit/log/Log.h"
#include "odblib/StringTool.h"
#include "UnitTest.h"

using namespace std;
using namespace eckit;

namespace odb {
namespace tool {
namespace test {


void UnitTest::test()
{
	if (getenv("ODB_ROOT"))
        StringTool::shell("MALLOC_CHECK_=2 ODB_COMPILER_FLAGS=`pwd`/2000010106/ECMA/ECMA.flags ./odb_migrator 2000010106/ECMA . 2000010106.{obstype}.{sensor}.odb", Here());
	else {
		Log::warning() << "UnitTest: ODB_ROOT not set, skipping testing of odb_migrator" << std::endl;
        StringTool::shell("./odb split 2000010106.odb 2000010106.{obstype}.{sensor}.odb", Here());
	}
}

void UnitTest::setUp() {}

void UnitTest::tearDown() {}

} // namespace test 
} // namespace tool 
} // namespace odb 


