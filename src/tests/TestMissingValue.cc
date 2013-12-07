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

#include "odblib/Comparator.h"
#include "odblib/Select.h"
#include "odblib/Reader.h"

#include "odblib/Tracer.h"
#include "odblib/Writer.h"
#include "UnitTest.h"

using namespace std;
using namespace eckit;

namespace odb {
namespace tool {
namespace test {


void UnitTest::setUp()
{
	Tracer t(Log::debug(), "UnitTest::setUp");

	odb::Writer<> f("UnitTest.odb");
	odb::Writer<>::iterator it = f.begin();
	MetaData& md = it->columns();

	md.setSize(2);
	it->setColumn(0, "lat@hdr", odb::REAL);
	it->missingValue(0, 1);

	BitfieldDef bfDef;
	bfDef.first.push_back("x");
	bfDef.second.push_back(1);
	bfDef.first.push_back("y");
	bfDef.second.push_back(2);

	it->setBitfieldColumn(1, "bf", odb::BITFIELD, bfDef);

	it->writeHeader();

	for (size_t i = 0; i <= 2; i++)
	{
		(*it)[0] = i;
		(*it)[1] = i;
		++it;
	}
}

void UnitTest::test()
{
	selectIntoSecondFile();

	odb::Comparator().compare("UnitTest.odb", "UnitTest.odb");

	{
		odb::Reader f("UnitTest.odb");
		odb::Reader::iterator fbegin(f.begin());
		odb::Reader::iterator fend(f.end());

		odb::Select s("select * from \"UnitTest.odb\";");
		odb::Select::iterator sbegin(s.begin());
		odb::Select::iterator send(s.end());

		odb::Comparator().compare(fbegin, fend, sbegin, send, "UnitTest.odb", "SELECT UnitTest.odb");
	}

	{
		odb::Reader f("UnitTest.odb");
		odb::Reader::iterator it = f.begin();
		odb::Reader::iterator end = f.end();

		MetaData& md = it->columns();
		Column& column = *md[0];
		codec::Codec& codec = column.coder();

		Log::info() << "UnitTest::test: codec: " << codec << std::endl;	

		ASSERT(codec.hasMissing());
		ASSERT(codec.missingValue() == 1);


		for (; it != end; ++it)
		{
			ASSERT( (*it).missingValue(0) == 1 );

			if ( (*it)[0] == 1 )
				ASSERT( (*it).isMissing(0) );
			else
				ASSERT( ! (*it).isMissing(0) );
		}
	}

	{
		// Check the isMissing and missingValue API of SelectIterator
		odb::Select s("select * from \"UnitTest.odb\";"); //, fileName);
		odb::Select::iterator i = s.begin();
		odb::Select::iterator e = s.end();
		for (; i != e; ++i)
		{
			ASSERT( (*i).missingValue(0) == 1 );
			ASSERT( (*i).missingValue(1) == 0 );

			if ( (*i)[0] == 1 )
				ASSERT( (*i).isMissing(0) );
			else
				ASSERT( ! (*i).isMissing(0) );

			// For Bitfields missing value by default equals 0
			if ( (*i)[1] == 0 )
				ASSERT( (*i).isMissing(1) );
			else
				ASSERT( ! (*i).isMissing(1) );
		}
	}

}

void UnitTest::selectIntoSecondFile()
{
	Tracer t(Log::debug(), "UnitTest::selectIntoSecondFile");

	const string fileName = "UnitTest.odb";
	string sql = "select lat,bf into \"UnitTest.odb\"";
	sql += " from \"" + fileName + "\" ;";

	odb::Select f(sql); //, fileName);
	odb::Select::iterator it = f.begin();

	++it; // this is needed to push the second row to the INTO file 
	++it; // this is needed to push the third row to the INTO file 
}

void UnitTest::tearDown() { }


} // namespace test 
} // namespace tool 
} // namespace odb 



