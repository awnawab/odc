/// @file   DataSelectOutput.h
/// @author Tomas Kral

#ifndef DataSelectOutput_H
#define DataSelectOutput_H

#include "eckit/sql/SQLOutput.h"

namespace odc {
namespace internal {

class DataSelectIterator;

class DataSelectOutput
  : public odc::sql::SQLOutput
{
public:
	DataSelectOutput(DataSelectIterator& it);
   ~DataSelectOutput();

protected:
	virtual void print(std::ostream&) const {}

private:
	DataSelectOutput(const DataSelectOutput&);
	DataSelectOutput& operator=(const DataSelectOutput&);

    DataSelectIterator& it_;
    unsigned long long count_;

	virtual void size(int) {}
	virtual void reset() { count_ = 0; }
    virtual void flush() {}
    virtual bool output(const odc::sql::expression::Expressions&);
	virtual void prepare(odc::sql::SQLSelect&) {}
	virtual void cleanup(odc::sql::SQLSelect&) {}
	virtual unsigned long long count() { return count_; }

	virtual void outputReal(double, bool) {}
	virtual void outputDouble(double, bool) {}
	virtual void outputInt(double, bool) {}
	virtual void outputUnsignedInt(double, bool) {}
	virtual void outputString(double, bool) {}
	virtual void outputBitfield(double, bool) {}
};

} // namespace internal
} // namespace odc

#endif // DataSelectOutput_H