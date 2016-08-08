/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <sstream>

#include "eckit/log/Log.h"
#include "eckit/runtime/Tool.h"
#include "eckit/runtime/Context.h"

#include "ecml/core/ExecutionContext.h"
#include "ecml/core/Environment.h"
#include "ecml/prelude/REPLHandler.h"

using namespace std;
using namespace eckit;

namespace ecml {

class TestECML : public Tool {
public:
    TestECML (int argc, char **argv) : Tool(argc, argv) {}
    ~TestECML() {}

    virtual void run();

protected:
    virtual void runScript(const std::string& pathName);
};

void TestECML::run()
{
    int argc = Context::instance().argc();
    if (argc < 2)
    {
        //throw UserError("Command line required (name(s) of file(s) with ECML script");
        ExecutionContext context;
        REPLHandler::repl(context);
    }

    for (int  i = 1; i < argc; ++i) {
        runScript(Context::instance().argv(i));
    }
}

void TestECML::runScript(const string& pathName)
{
    Log::info() << endl << " ** ecml_test: running " << pathName << endl;
    ExecutionContext context;
    context.executeScriptFile(pathName);
}

} // namespace ecml

int main(int argc,char **argv)
{
    //TODO: set a behaviour?
    //Context::instance().behavior( new odb::ODBBehavior() );
    // TODO: enable $DEBUG (Log::debug)

    ecml::TestECML runner(argc, argv);
    return runner.start();
}