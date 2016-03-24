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

#include "eckit/log/Timer.h"
#include "odb_api/MetaData.h"
#include "odb_api/Reader.h"

#include "odb_api/Writer.h"
#include "MockReader.h"
#include "TestCase.h"

using namespace std;
using namespace eckit;
using namespace odb;

const long the_const_value = 20090624;

class MockReaderIterator2 
{
public:
	MockReaderIterator2() : columns_(1), nRows_(2), n_(nRows_), data_(the_const_value), refCount_(0), noMore_(false)
	{
		odb::Column* col = columns_[0] = new odb::Column(columns_);
		ASSERT(col);

		col->name("date"); 
        col->type<odb::DataStream<odb::SameByteOrder, DataHandle> >(odb::INTEGER, false);
		col->hasMissing(false);
		odb::codec::CodecInt32<odb::SameByteOrder> *codec = new odb::codec::CodecInt32<odb::SameByteOrder>;
		col->coder(codec);
	}

	odb::MetaData& columns() { return columns_; }

	bool isNewDataset() { return false; } // { return n_ == nRows_; }
	double* data() { return &data_; }

	bool next(eckit::ExecutionContext*) { return !(noMore_ = nRows_-- <= 0); }

private:
	odb::MetaData columns_;
	int nRows_;
	int n_;
	double data_;

public:
	int refCount_;
	bool noMore_;
    ExecutionContext* context_;
};

static void setUp()
{
	Timer t("Writing test_integer_const.odb");
	odb::Writer<> oda("test_integer_const.odb");

	typedef tool::MockReader<MockReaderIterator2> M;
	M reader;

	odb::Writer<>::iterator outit = oda.begin();

	M::iterator b = reader.begin();
	const M::iterator e = reader.end();
	outit->pass1(b, e);
}

static void test()
{
	odb::Reader oda("test_integer_const.odb");
	odb::Reader::iterator it = oda.begin();
	odb::Reader::iterator end = oda.end();

	Log::info() << it->columns() << std::endl;
	
	for ( ; it != end; ++it)
		ASSERT((*it)[0] == the_const_value);

	Log::debug() << "test: codec name is '" << it->columns()[0]->coder().name() << "'" << std::endl;
	ASSERT(it->columns()[0]->coder().name() == "constant");
}


static void tearDown(){}

SIMPLE_TEST(ConstIntegerCodec)
