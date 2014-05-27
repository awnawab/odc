/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// File FunctionNULL.h
// Baudouin Raoult - ECMWF Dec 03

#ifndef FunctionNULL_H
#define FunctionNULL_H

#include "odb_api/FunctionExpression.h"

namespace odb {
namespace sql {
namespace expression {
namespace function {

class FunctionNULL : public FunctionExpression {
public:
	FunctionNULL(const std::string&,const expression::Expressions&);
	FunctionNULL(const FunctionNULL&);
	~FunctionNULL(); 

	SQLExpression* clone() const;

private:
// No copy allowed
	virtual const odb::sql::type::SQLType* type() const;
	FunctionNULL& operator=(const FunctionNULL&);

// -- Overridden methods
	virtual double eval(bool& missing) const;
// -- Friends
	//friend std::ostream& operator<<(std::ostream& s,const FunctionNULL& p)
	//	{ p.print(s); return s; }
};

} // namespace function
} // namespace expression 
} // namespace sql
} // namespace odb 

#endif