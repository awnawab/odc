/*
 * (C) Copyright 1996-2013 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// File ODBModule.h
// Piotr Kuchta - (c) ECMWF May 2015

#ifndef ODBModule_H
#define ODBModule_H

#include "eckit/ecml/core/Module.h"
#include "eckit/ecml/core/ExecutionContext.h"

namespace odb {

class ODBModule : public eckit::Module {
public:
    ODBModule();
    ~ODBModule();
    void importInto(eckit::ExecutionContext&);
};

}  // namespace odb

#endif