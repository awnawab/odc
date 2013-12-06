/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// \file TestRunner.h
///
/// @author Piotr Kuchta, ECMWF, Feb 2009

//#include <sstream>

//#include "eckit/filesystem/PathName.h"
//#include "eckit/utils/Timer.h"
//#include "eckit/utils/StringTools.h"

//#include "odblib/CommandLineParser.h"
//#include "odblib/Tool.h"
//#include "odblib/TestCase.h"
//#include "odblib/ToolFactory.h"
#include "odblib/TestRunner.h"

using namespace eckit;

namespace odb {
namespace tool {
namespace test {

TestRunner::TestRunner (CommandLineParser& clp)
: clp_(clp),
  mars_sms_label_(false),
  label_()
{
	if (getenv("MARS_SMS_LABEL"))
	{
		mars_sms_label_ = true;
		label_ = getenv("MARS_SMS_LABEL");
	}
}

TestRunner::~TestRunner () {}

void TestRunner::run()
{
	ASSERT(getenv("ODB_API_TEST_DATA_PATH") && "ODB_API_TEST_DATA_PATH must be set");

	stringstream totalRunningTime;
	auto_ptr<Timer> allTestsTimer(new Timer("Total", totalRunningTime));
	auto_ptr<TestCases> tests(0);
	
	failed_.clear();

	if (clp_.parameters().size() == 1)
	{
		tests.reset(AbstractToolFactory::testCases());
		runTests(*tests);
	}
	else
	{
		// TODO: keep the config in the ODB_API_TEST_DATA_PATH
		//readConfig("../../../odb_api/src/odb/TestRunnerApplication.cfg");
		readConfig("/tmp/Dropbox/work/odb_api/src/odb/TestRunnerApplication.cfg");
		tests.reset(new TestCases());
		for (size_t i = 1; i < clp_.parameters().size(); ++i)
		{
			std::string suiteName = clp_.parameters()[i];
			ASSERT("Suite does not exist" && suites_.find(suiteName) != suites_.end());
			std::vector<std::string>& suite = suites_[suiteName];
			auto_ptr<TestCases> tsts(AbstractToolFactory::testCases(suite));
			runTests(*tsts);
			tests->insert(tests->end(), tsts->begin(), tsts->end());
		}
	}

	allTestsTimer.reset();

	ofstream xmlf("testresults.xml");
	xmlf << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
	xmlf << "<testsuite name=\"unittests\" time=\"" << StringTools::split(" ", totalRunningTime.str())[1] << "\">" << std::endl;
	xmlf << xml_.str();
	xmlf << "</testsuite>" << std::endl;

	size_t nTests = tests->size();
	for (size_t i = 0; i < nTests; ++i)
		delete (*tests)[i];

	if (failed_.size() == 0) {
		Log::info() << endl << "+- Phew, made it! All " << nTests << " tests passed successfully. " << endl << std::endl;
		Log::info() << runningTimes_.str() << std::endl;;
		Log::info() << totalRunningTime.str() << std::endl;
	}
	else
	{
		Log::error() << endl << "+- Summary: " << failed_.size() << " test(s) failed." << std::endl;
		for (std::vector<FailedTest>::iterator it = failed_.begin(); it != failed_.end(); ++it) {
			const std::string& name = it->first;
			const std::string& what = it->second;
			Log::error() << "\t" << name << ": " << endl << what;
		}
		Log::error() << std::endl;

		stringstream ss;
		ss << " " << failed_.size() << " test(s) failed";
		throw eckit::SeriousBug(ss.str());
	}
}

void TestRunner::runTests(const TestCases& tests)
{
	for (TestCases::const_iterator it = tests.begin(); it != tests.end(); ++it)
	{
		bool exceptionThrown = false;
		std::string what;
		TestCase *tst = *it;
		const std::string& name = tst->name();

		Log::info() << "+- Running " << name << " ..." << std::endl;
		smslabel(name);

		stringstream runningTime;
		auto_ptr<Timer> timer(new Timer(name, runningTime));
		try {
			tst->setUp();
			tst->test();
		} catch (std::exception &e) {
			Log::warning() << "+- FAILED" << std::endl;
			exceptionThrown = true;
			what += std::string(e.what()) + '\n';
		} catch (...) {
			Log::warning() << "+- FAILED: unknown exception!" << std::endl;
			exceptionThrown = true;
			what += std::string("Uknown exception") + '\n';
		}
		try {
			tst->tearDown();
		} catch (std::exception &e) {
			Log::warning() << "+- Exception thrown from tearDown." << std::endl;
			exceptionThrown = true;
			what += std::string("[In tearDown:]") + std::string(e.what()) + '\n';
		} catch (...) {
			Log::warning() << "+- FAILED: unknown exception!" << std::endl;
			exceptionThrown = true;
			what += std::string("Uknown exception") + '\n';
		}

		if (exceptionThrown) {
			failed_.push_back(make_pair(name, what));
			xml_ << "<testcase classname=\"test\" name=\"" << name << "\">" << std::endl;
			xml_ << "	<failure type=\"exception\"><![CDATA[" << what << "]]></failure>" << std::endl;
			xml_ << "</testcase>" << std::endl;
		}
		else {
			timer.reset();
			runningTimes_ << runningTime.str();
			Log::info() << "+- Passed." << endl << std::endl;
		 	xml_ << "<testcase classname=\"test\" name=\"" << name 
				<< "\" time=\"" << StringTools::split(" ", runningTime.str())[1] << "\"/>" << std::endl;
		}
	}
}

void TestRunner::readConfig(const PathName fileName)
{
	Log::debug() << "TestRunner::readConfig: reading file '" << fileName << "'" << std::endl;
	suites_.clear();

    std::vector<std::string> lines = StringTool::readLines(fileName);
    for (size_t i = 0; i < lines.size(); ++i)
	{
		std::vector<std::string> words = StringTools::split(":", lines[i]);
		if (words.size() == 0)
			continue;
		ASSERT("Each line of config file should be like: '<suite_name> : TestPattern1 TestPattern2 ...'" && words.size() == 2);

		suites_[words[0]] = StringTools::split(" \t", words[1]);
		Log::debug() << "TestRunner::readConfig(\"" << fileName << "\"): "
			<< words[0] << ": "
			<< suites_[words[0]].size() << " entries." << std::endl;
	}
}

void TestRunner::smslabel(const std::string &s)
{
	if (! mars_sms_label_)
		return;
	std::string cmd = "smslabel ";
	cmd += label_ + " " + s;
    system(cmd.c_str());
}

} // namespace test
} // namespace tool 
} // namespace odb 

