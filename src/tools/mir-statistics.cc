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
#include "mir/stats/Statistics.h"
#include "mir/config/MIRConfiguration.h"
#include "mir/input/GribFileInput.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/param/MIRCombinedParametrisation.h"
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
        options_.push_back(new FactoryOption<mir::stats::StatisticsFactory>("stats", "Statistics methods for interpreting field values"));
    }
};


void MIRStatistics::usage(const std::string &tool) const {
    eckit::Log::info()
            << "\nUsage: " << tool << " [--stats=option] file.grib [file.grib [...]]"
               "\nExamples:"
               "\n  % " << tool << " file.grib"
               "\n  % " << tool << " --stats=scalar file1.grib file2.grib file3.grib"
               "\n  % " << tool << " --stats=spectral file.grib"
            << std::endl;
}


void MIRStatistics::execute(const eckit::option::CmdArgs& args) {
    using namespace mir::input;
    using namespace mir::param;
    using namespace mir::stats;


    // runtime statistics
    mir::util::MIRStatistics mir_statistics;

    const ConfigurationWrapper args_wrap(const_cast<eckit::option::CmdArgs&>(args));
    const mir::config::MIRConfiguration& config = mir::config::MIRConfiguration::instance();


    for (size_t i = 0; i < args.count(); ++i) {
        GribFileInput input(args(i));


        size_t count = 0;
        while (input.next()) {
            eckit::Log::info() << "\n'" << args(i) << "' #" << ++count << std::endl;

            // Build MIRCombinedParametrisation from a few parts:
            // - wrap the arguments, so that they behave as a MIRParametrisation
            // - get the input as a MIRParametrisation, so to get the paramId
            // - lookup configuration for this paramId/metadata, to get specific "stats" parameter

            eckit::ScopedPtr<const MIRParametrisation> defaults(config.lookup(static_cast<const MIRInput&>(input).parametrisation()));
            MIRCombinedParametrisation parametrisation(args_wrap, input, *defaults);


            // Get paramId/metadata-specific "stats" method
            std::string stats;
            static_cast<const MIRParametrisation&>(parametrisation).get("stats", stats);


            // Calculate and show statistics
            Statistics::Results results;

            eckit::ScopedPtr<const Statistics> s(StatisticsFactory::build(stats, parametrisation));
            mir::context::Context ctx(input, mir_statistics);
            s->execute(ctx, results);

            eckit::Log::info() << results << std::endl;
        }
    }

}


int main(int argc, char **argv) {
    MIRStatistics tool(argc, argv);
    return tool.start();
}

