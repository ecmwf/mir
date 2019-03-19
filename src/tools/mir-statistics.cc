/*
 * (C) Copyright 1996- ECMWF.
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
#include "eckit/option/SimpleOption.h"

#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/stats/Statistics.h"
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
        options_.push_back(new eckit::option::FactoryOption<mir::stats::StatisticsFactory>("statistics", "Statistics methods for interpreting field values"));
        options_.push_back(new eckit::option::SimpleOption< double >("lower-limit", "count lower limit (count-outside-range)"));
        options_.push_back(new eckit::option::SimpleOption< double >("upper-limit", "count upper limit (count-outside-range)"));
    }
};


void MIRStatistics::usage(const std::string &tool) const {
    eckit::Log::info()
            << "\nUsage: " << tool << " [--statistics=option] file.grib [file.grib [...]]"
               "\nExamples:"
               "\n  % " << tool << " file.grib"
               "\n  % " << tool << " --statistics=scalar file1.grib file2.grib file3.grib"
               "\n  % " << tool << " --statistics=spectral file.grib"
            << std::endl;
}


void MIRStatistics::execute(const eckit::option::CmdArgs& args) {
    using namespace mir::param;

    // Build CombinedParametrisation from a few parts:
    // - wrap the arguments, so that they behave as a MIRParametrisation
    // - input grib metadata
    // - lookup configuration for input metadata-specific parametrisation

    const ConfigurationWrapper args_wrap(args);
    const DefaultParametrisation defaults;

    for (size_t i = 0; i < args.count(); ++i) {
        mir::input::GribFileInput grib(args(i));
        const mir::input::MIRInput& input = grib;

        size_t count = 0;
        while (grib.next()) {
            eckit::Log::info() << "\n'" << args(i) << "' #" << ++count << std::endl;

            // Metadata-specific defaults
            // const MIRParametrisation& parameter = mir::config::MIRConfiguration::instance().pick(input.parametrisation());
            CombinedParametrisation combined(args_wrap, grib, defaults);


            // Get paramId/metadata-specific "stats" method
            std::string statistics = "scalar";
            const MIRParametrisation& c = combined;
            c.get("statistics", statistics);


            // Calculate and show statistics
            eckit::ScopedPtr<mir::stats::Statistics> stats(mir::stats::StatisticsFactory::build(statistics, combined));
            stats->execute(input.field());

            eckit::Log::info() << *stats << std::endl;
        }
    }

}


int main(int argc, char **argv) {
    MIRStatistics tool(argc, argv);
    return tool.start();
}

