/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "odblib/ShiftedBitColumnExpression.h"
#include "odblib/SQLSelect.h"
#include "odblib/SQLBit.h"
#include "odblib/SQLTable.h"

namespace odb {
namespace sql {
namespace expression {

ShiftedBitColumnExpression::ShiftedBitColumnExpression(const string& name, const string& field, SQLTable* table, int shift)
: BitColumnExpression(name, field, table),
  shift_(shift)
{}

ShiftedBitColumnExpression::ShiftedBitColumnExpression(const string& name, const string& field, const string& tableReference, int shift)
: BitColumnExpression(name, field, tableReference),
  shift_(shift)
{}

ShiftedBitColumnExpression::~ShiftedBitColumnExpression() {}

void ShiftedBitColumnExpression::prepare(SQLSelect& sql)
{
	BitColumnExpression::prepare(sql);
}

double ShiftedBitColumnExpression::eval(bool& missing) const
{
	if(value_->second) missing = true;
	unsigned long x = static_cast<unsigned long>(value_->first);
	return (x & mask_) >> bitShift_;
}

} // namespace expression
} // namespace sql
} // namespace odb
