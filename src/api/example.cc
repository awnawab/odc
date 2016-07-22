/*
 * (C) Copyright 1996-2012 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// \file examples.cc
///
/// This file contains examples of usage of public APIs.
///
/// @author Piotr Kuchta, ECMWF, July 2016

#include <string>
#include <iostream>
#include <vector>
#include <sstream>

#include "sqlite3.h"

#include "eckit/filesystem/PathName.h"
#include "eckit/runtime/ContextBehavior.h"
#include "eckit/runtime/Context.h"

#include "odb_api/ODBBehavior.h"
#include "odb_api/tools/Tool.h"
#include "odb_api/tools/TestRunnerApplication.h"
#include "odb_api/tools/TestCase.h"
#include "odb_api/tools/ImportTool.h"

void checkRC(int rc, const char* message, sqlite3 *db) 
{
    if (rc != SQLITE_OK) {
        std::string msg (std::string(message) + sqlite3_errmsg(db));
        sqlite3_close(db);
        throw eckit::AssertionFailed (msg);
    }
}

TEST(example_insert_data)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;

    int rc = sqlite3_open("CREATE TABLE foo AS (x INTEGER, y INTEGER, v REAL)"
                          " ON 'example_select_data_read_results.odb';", &db);
    checkRC(rc, "Cannot open database", db);
    
    rc = sqlite3_prepare_v2(db, "INSERT INTO foo (x,y,v) VALUES (?,?,?);", -1, &stmt, 0);    
    checkRC(rc, "Failed to prepare INSERT statement", db);

    for (int i = 1; i <= 3; ++i)
    {
        rc = sqlite3_bind_int(stmt, 0, 1 * i);
        checkRC(rc, "Failed to bind int value", db);

        rc = sqlite3_bind_int(stmt, 1, 10 * i);
        checkRC(rc, "Failed to bind int value", db);

        rc = sqlite3_bind_double(stmt, 2, 0.1 * i);
        checkRC(rc, "Failed to bind double value", db);

        rc = sqlite3_step(stmt);
        //checkRC(rc, "Failed to step and write row", db);
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

TEST(example_select_data_read_results)
{
    sqlite3 *db;
    sqlite3_stmt *res;
    
    int rc = sqlite3_open("CREATE TABLE foo " //ON 'example_select_data_read_results.odb';", &db);
                           " ON 'mars://RETRIEVE,CLASS=OD,TYPE=MFB,STREAM=OPER,EXPVER=0001,DATE=20160720,TIME=1200,DATABASE=marsod';", &db);
    checkRC(rc, "Cannot open database", db);
    
    rc = sqlite3_prepare_v2(db, "SELECT * FROM foo;", -1, &res, 0);
    checkRC(rc, "Failed to prepare statement", db);

    int number_of_columns = sqlite3_column_count(res);

    // Print CSV header. Each field is a colon separated pair of column name and type.
    for (int i = 0; i < number_of_columns; ++i)
        printf("%s:%d%s", sqlite3_column_name(res, i), 
                          sqlite3_column_type(res, i), 
                          ((i < number_of_columns - 1) ? "," : ""));
    printf("\n");
   
    // Print rows of data. 
    while((rc = sqlite3_step(res)) != SQLITE_DONE)
    {
        if (rc == SQLITE_ROW)
        {
            for (int column = 0; column < number_of_columns; ++column)
                printf("%s%s", sqlite3_column_text(res, column),
                               ((column < number_of_columns - 1) ? "," : ""));
            printf("\n");
        }
    }
    
    rc = sqlite3_finalize(res);
    checkRC(rc, "sqlite3_finalize failed", db);
    rc = sqlite3_close(db);
    checkRC(rc, "sqlite3_close failed", db);
}


TEST(example_libversion)
{
    std::stringstream ss;
    ss << sqlite3_libversion();

    eckit::Log::info() << "sqlite3_libversion: " << ss.str() << std::endl;
}

int main(int argc, char** argv)
{
    using namespace odb;
    using namespace odb::tool;
    using namespace eckit;

	Tool::registerTools();
    static ContextBehavior* behavior (0);
    if (behavior == 0)
        Context::instance().behavior(behavior = new ODBBehavior());
    odb::tool::test::TestRunnerApplication testRunner(argc, argv);
    testRunner.start();
}

