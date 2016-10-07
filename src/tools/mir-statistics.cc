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
#include "mir/param/MIRCombinedParametrisation.h"
#include "mir/param/MIRDefaults.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/MIRStatistics.h"
#include "mir/param/ConfigurationWrapper.h"


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
        options_.push_back(new FactoryOption<mir::action::statistics::StatisticsFactory>("stats", "Statistics methods for interpreting field values"));
    }
};


void MIRStatistics::usage(const std::string &tool) const {
    eckit::Log::info()
            << "\nUsage: " << tool << " [--stats=option] file.grib [file.grib [...]]"
               "\nExamples:"
               "\n  % " << tool << " file.grib"
               "\n  % " << tool << " --stats=ScalarStatistics file1.grib file2.grib file3.grib"
               "\n  % " << tool << " --stats=SHStatistics file.grib"
            << std::endl;
}


void MIRStatistics::execute(const eckit::option::CmdArgs& args) {
    using namespace mir::action::statistics;
    using namespace mir::param;

    std::string opt;
    args.get("stats", opt);

    mir::util::MIRStatistics stats;

    for (size_t i = 0; i < args.count(); ++i) {
        mir::input::GribFileInput input(args(i));

        const ConfigurationWrapper wrap(const_cast<eckit::option::CmdArgs&>(args));
        MIRCombinedParametrisation parametrisation(wrap, input, MIRDefaults::instance());

        size_t count = 0;
        while (input.next()) {
            eckit::Log::info() << "\n'" << args(i) << "' #" << ++count << std::endl;

            eckit::ScopedPtr<const Statistics> statistics(StatisticsFactory::build(opt, parametrisation));
            mir::context::Context ctx(input, stats);
            statistics->execute(ctx);

            eckit::Log::info() << statistics->results() << std::endl;
        }
    }

}


int main(int argc, char **argv) {
    MIRStatistics tool(argc, argv);
    return tool.start();
}

