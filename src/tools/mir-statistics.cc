/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Oct 2016


#include "eckit/log/Log.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/FactoryOption.h"
#include "mir/action/context/Context.h"
#include "mir/action/statistics/Statistics.h"
#include "mir/input/GribFileInput.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/MIRStatistics.h"


class MIRStatistics : public mir::tools::MIRTool {
private:

    void execute(const eckit::option::CmdArgs&);

    void usage(const std::string& tool) const;

    int minimumPositionalArguments() const {
        return 1;
    }

public:

    MIRStatistics(int argc, char **argv) : mir::tools::MIRTool(argc, argv) {
        using namespace eckit::option;

        options_.push_back(new FactoryOption<mir::action::statistics::StatisticsFactory>("statistics", "Statistics methods for interpreting field values"));
    }
};


void MIRStatistics::usage(const std::string &tool) const {
    eckit::Log::info()
            << "\nUsage: " << tool << " [--statistics=option] file.grib [file.grib [...]]"
               "\nExamples:"
               "\n  % " << tool << " file.grib"
               "\n  % " << tool << " --statistics=ScalarStatistics file1.grib file2.grib file3.grib"
               "\n  % " << tool << " --statistics=SHStatistics file.grib"
            << std::endl;
}


void MIRStatistics::execute(const eckit::option::CmdArgs& args) {
    using namespace mir::action::statistics;

    std::string opt;
    args.get("statistics", opt);

    mir::util::MIRStatistics stats;

    for (size_t i = 0; i < args.count(); ++i) {
        mir::input::GribFileInput input(args(i));
        mir::context::Context ctx(input, stats);

        size_t count = 0;
        while (input.next()) {

            eckit::Log::info() << "\n'" << args(i) << "' field #" << ++count << std::endl;
            eckit::ScopedPtr<const Statistics> statistics(StatisticsFactory::build(opt, input));
            statistics->execute(ctx);
            eckit::Log::info() << statistics->results() << std::endl;

        }
    }

}


int main(int argc, char **argv) {
    MIRStatistics tool(argc, argv);
    return tool.start();
}

