/*
 * (C) Copyright 1996-2012 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/io/Buffer.h"
#include "eckit/io/MemoryHandle.h"
#include "eckit/testing/Test.h"
#include "eckit/types/FloatCompare.h"

#include "odb_api/Writer.h"
#include "odb_api/Select.h"

#include "TemporaryFiles.h"

using namespace eckit::testing;
using eckit::types::is_approximately_equal;

// ------------------------------------------------------------------------------------------------------

CASE("Various distance measuring functions return sensible things") {

    TemporaryFile f;

    // Write some data with a latitude and longitude.

    {
        odb::Writer<> oda(f.path());
        odb::Writer<>::iterator row = oda.begin();

        row->setNumberOfColumns(2);

        row->setColumn(0, "lat", odb::REAL);
        row->setColumn(1, "lon", odb::REAL);

        row->writeHeader();

        // Include some extreme values

        (*row)[0] = 45.0;
        (*row)[1] = 0.0;
        ++row;

        (*row)[0] = 90.0;
        (*row)[1] = 180.0;
        ++row;

        (*row)[0] = -90.0;
        (*row)[1] = 360.0;
        ++row;

        (*row)[0] = -45.0;
        (*row)[1] = -90.0;
        ++row;
    }

    // And test that the SQL functions get the right data out!!!

    const std::string sql = std::string("select rad(45.0,0.0,1.0,lat,lon), ") +
                                        "rad(10.0,0.0,0.0,lat,lon), " +
                                        "distance(46.0,0.0,lat,lon), " +
                                        "km(46.0,0.0,lat,lon), " +
                                        "dist(100.,46.0,1.0,lat,lon), " +
                                        "dist(40.0,5.0,1000.0,lat,lon) " +
                                        "from \"" + f.path() + "\";";

    const double eps = 7.e-6;

    {
        odb::Select oda(sql);
        odb::Select::iterator it = oda.begin();

        EXPECT((*it)[0] == 1);                                  // Inside relevant great-circle
        EXPECT((*it)[1] == 0);
        EXPECT(is_approximately_equal((*it)[2], 111120., eps)); // Surface distance to specified point
        EXPECT(is_approximately_equal((*it)[3], 111.12, eps));  // In m
        EXPECT((*it)[4] == 0);                                  // Within specified distance of given point
        EXPECT((*it)[5] == 1);
        ++it;

        EXPECT((*it)[0] == 0);                                  // Inside relevant great-circle
        EXPECT((*it)[1] == 0);
        // pi/4 * R
        EXPECT(is_approximately_equal((*it)[2], 4889280., eps)); // Surface distance to specified point
        EXPECT(is_approximately_equal((*it)[3], 4889.28, eps));  // In m
        EXPECT((*it)[4] == 0);                                  // Within specified distance of given point
        EXPECT((*it)[5] == 0);
        ++it;

        EXPECT((*it)[0] == 0);                                  // Inside relevant great-circle
        EXPECT((*it)[1] == 0);
        // 3*pi/4 * R
        EXPECT(is_approximately_equal((*it)[2], 15112320., eps)); // Surface distance to specified point
        EXPECT(is_approximately_equal((*it)[3], 15112.32, eps));  // In m
        EXPECT((*it)[4] == 0);                                  // Within specified distance of given point
        EXPECT((*it)[5] == 0);
        ++it;

        EXPECT((*it)[0] == 0);                                  // Inside relevant great-circle
        EXPECT((*it)[1] == 0);
        EXPECT(is_approximately_equal((*it)[2], 13398177.5541776344, eps)); // Surface distance to specified point
        EXPECT(is_approximately_equal((*it)[3], 13398.1775541776344, eps));  // In m
        EXPECT((*it)[4] == 0);                                  // Within specified distance of given point
        EXPECT((*it)[5] == 0);
        ++it;
    }

    // Test against things measured from the north and south poles

    const std::string sql2 = std::string("select "
                                         "distance(90.0,0.0,lat,lon), "
                                         "km(90.0,0.0,lat,lon), "
                                         "distance(-90.0,0.0,lat,lon), "
                                         "km(-90.0,0.0,lat,lon), "
                                         "distance(0.0,180.0,lat,lon), "
                                         "km(0.0,180.0,lat,lon), "
                                         "from \"") + f.path() + "\";";

    {
        odb::Select oda(sql2);
        odb::Select::iterator it = oda.begin();

        // pi/4 * R_e
        EXPECT(is_approximately_equal((*it)[0], 5000400., eps));
        EXPECT(is_approximately_equal((*it)[1], 5000.4, eps));
        // 3*pi/4 * R_e
        EXPECT(is_approximately_equal((*it)[2], 15001200., eps));
        EXPECT(is_approximately_equal((*it)[3], 15001.2, eps));
        // 3*pi/4 * R_e
        EXPECT(is_approximately_equal((*it)[4], 15001200., eps));
        EXPECT(is_approximately_equal((*it)[5], 15001.2, eps));
        ++it;

        EXPECT(is_approximately_equal((*it)[0], 0., eps));
        EXPECT(is_approximately_equal((*it)[1], 0., eps));
        // pi * R_e
        EXPECT(is_approximately_equal((*it)[2], 20001600., eps));
        EXPECT(is_approximately_equal((*it)[3], 20001.6, eps));
        // pi / 2 * R_e
        EXPECT(is_approximately_equal((*it)[4], 10000800., eps));
        EXPECT(is_approximately_equal((*it)[5], 10000.8, eps));
        ++it;

        EXPECT(is_approximately_equal((*it)[0], 20001600., eps));
        EXPECT(is_approximately_equal((*it)[1], 20001.6, eps));
        // pi * R_e
        EXPECT(is_approximately_equal((*it)[2], 0., eps));
        EXPECT(is_approximately_equal((*it)[3], 0., eps));
        // pi / 2 * R_e
        EXPECT(is_approximately_equal((*it)[4], 10000800., eps));
        EXPECT(is_approximately_equal((*it)[5], 10000.8, eps));
        ++it;

        // 3*pi/4 * R_e
        EXPECT(is_approximately_equal((*it)[0], 15001200., eps));
        EXPECT(is_approximately_equal((*it)[1], 15001.2, eps));
        // pi/4 * R_e
        EXPECT(is_approximately_equal((*it)[2], 5000400., eps));
        EXPECT(is_approximately_equal((*it)[3], 5000.4, eps));
        // pi / 2 * R_e
        EXPECT(is_approximately_equal((*it)[4], 10000800., eps));
        EXPECT(is_approximately_equal((*it)[5], 10000.8, eps));
        ++it;
    }
}


CASE("Inside or outside detection works for circrles") {

    TemporaryFile f;

    // Write some data with a latitude and longitude.

    {
        odb::Writer<> oda(f.path());
        odb::Writer<>::iterator row = oda.begin();

        row->setNumberOfColumns(2);

        row->setColumn(0, "lat", odb::REAL);
        row->setColumn(1, "lon", odb::REAL);

        row->writeHeader();

        // Include some extreme values

        (*row)[0] = 45.0;
        (*row)[1] = 10.0;
        ++row;
    }

    // And test that the SQL functions get the right data out!!!

    const std::string sql = std::string("select circle(lat,46.0, lon,11.0,1.0), ") +
                                        "circle(lat,46.0, lon,11.0,1.5) " +
                                        "from \"" + f.path() + "\";";

    {
        odb::Select oda(sql);
        odb::Select::iterator it = oda.begin();

        EXPECT((*it)[0] == 0);                                  // Inside relevant great-circle
        EXPECT((*it)[1] == 1);
    }
}

// ------------------------------------------------------------------------------------------------------

int main(int argc, char* argv[]) {
    return run_tests(argc, argv);
}
