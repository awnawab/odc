/*
 * (C) Copyright 2019- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <functional>

#include "eckit/exception/Exceptions.h"
#include "eckit/runtime/Main.h"

#include "odc/api/odc.h"

#include "odc/api/Odb.h"

using namespace odc::api;

extern "C" {

//----------------------------------------------------------------------------------------------------------------------

// DOCUMENTATION comments

// i) By default this API throws exceptions. This will result in hard termination
//    of applications on failure. The routine:
//
//      void odc_error_handling(handling_type);
//
//    changes the behaviour with three options:
//
//      ODC_THROW          - Throws exceptions
//      ODC_ERRORS_CHECKED - Enforces that error values are checked. If an API call
//                           is made and the previous error has not been reset, then
//                           std::terminate will be called.
//      ODC_ERRORS_REPORT  - All exceptions will be caught and odc_errno set
//                           accordingly.

//----------------------------------------------------------------------------------------------------------------------

struct odb_t {
    odb_t(const char* f) : internal(f) {}
    Odb internal;
};

struct odb_table_t {
    odb_table_t(const Table& t) : internal(t) {}
    const Table internal;
};

//----------------------------------------------------------------------------------------------------------------------

// Types for lookup

const int ODC_NUM_TYPES = NUM_TYPES;

const char* ODC_TYPE_NAMES[] = {
    OdbTypes<ColumnType(0)>::name,
    OdbTypes<ColumnType(1)>::name,
    OdbTypes<ColumnType(2)>::name,
    OdbTypes<ColumnType(3)>::name,
    OdbTypes<ColumnType(4)>::name,
    OdbTypes<ColumnType(5)>::name
};

//----------------------------------------------------------------------------------------------------------------------

/* Error handling */

static std::string g_current_error_str;
static int g_odc_error_behaviour = ODC_THROW;
int odc_errno = 0;

void odc_reset_error() {
    g_current_error_str.clear();
    odc_errno = 0;
}

void odc_error_handling(int handling_type) {
    ASSERT(handling_type >= ODC_THROW && handling_type <= ODC_ERRORS_REPORT);
    g_odc_error_behaviour = handling_type;
    odc_reset_error();
}

const char* odc_error_string() {
    if (g_current_error_str.empty()) {
        return 0;
    } else {
        return g_current_error_str.c_str();
    }
}

static void set_error(const std::string& s, int err) {
    g_current_error_str = s;
    odc_errno = err;
}

} // extern "C"

// Template can't have C linkage

namespace {

template <typename T>
T DefaultConstructed() { return T(); }

template <>
void DefaultConstructed<void>() {}

template <typename FN>
auto wrapApiFunction(FN f) -> decltype(f()) {

    if (g_odc_error_behaviour == ODC_ERRORS_REPORT) {
        odc_reset_error();
    }
    if (g_odc_error_behaviour == ODC_ERRORS_CHECKED && odc_errno != 0) {
        std::stringstream ss;
        ss << "APi call being made after unchecked error: ("
           << odc_errno << "): " << odc_error_string()
           << ". SeriousBug in calling code";
        throw eckit::SeriousBug(ss.str(), Here());
    }

    try {
        return f();
    } catch (eckit::Exception& e) {
        if (g_odc_error_behaviour == ODC_THROW) throw;
        set_error(e.what(), 1);
    } catch (std::exception& e) {
        if (g_odc_error_behaviour == ODC_THROW) throw;
        set_error(e.what(), 2);
    } catch (...) {
        if (g_odc_error_behaviour == ODC_THROW) throw;
        set_error("Unexpected exception caught", 3);
    }

    // Return default constructed value.
    return DefaultConstructed<decltype(f())>();
}

}

extern "C" {

//----------------------------------------------------------------------------------------------------------------------

// TODO: In a sensible, templated, way catch all exceptions.
//       --> We ought to have a standardised error return process.


/*
 * Initialise API
 * @note This is only required if being used from a context where eckit::Main()
 *       is not otherwise initialised
*/

void odc_initialise_api() {
    return wrapApiFunction([] {
        static bool initialised = false;

        if (!initialised) {
            const char* argv[2] = {"odc-api", 0};
            eckit::Main::initialise(1, const_cast<char**>(argv));
            initialised = true;
        }

        ASSERT(false);
    });
}

/* Basic READ objects */

odb_t* odc_open_for_read(const char* filename) {
    return wrapApiFunction([filename] {
        return new odb_t {filename};
    });
}


void odc_close(odb_t* o) {
    return wrapApiFunction([o]{
        ASSERT(o);
        delete o;
    });
}

/*
 * Table handling
 */

int odc_num_tables(struct odb_t* o) {
    return wrapApiFunction([o] {
        return o->internal.numTables();
    });
}

odb_table_t* odc_get_table(odb_t* o, int n) {
    return wrapApiFunction([o, n] {
        ASSERT(n >= 0 && n < o->internal.numTables());
        return new odb_table_t(o->internal.tables()[n]);
    });
}

void odc_free_table(odb_table_t* t) {
    return wrapApiFunction([t] {
        ASSERT(t);
        delete(t);
    });
}

int odc_table_num_rows(struct odb_table_t* t) {
    return wrapApiFunction([t] {
        ASSERT(t);
        return t->internal.numRows();
    });
}

int odc_table_num_columns(struct odb_table_t* t) {
    return wrapApiFunction([t] {
        ASSERT(t);
        return t->internal.numColumns();
    });
}

int odb_table_column_type(struct odb_table_t* t, int col) {
    return wrapApiFunction([t, col] {
        ASSERT(t);
        return t->internal.columnType(col);
    });
}

const char* odb_table_column_name(struct odb_table_t* t, int col) {
    return wrapApiFunction([t, col] {
        ASSERT(t);
        return t->internal.columnName(col).c_str();
    });
}


//----------------------------------------------------------------------------------------------------------------------

} // extern "C"
