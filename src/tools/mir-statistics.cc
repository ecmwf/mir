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
        using namespace eckit::option;
        options_.push_back(new FactoryOption<mir::stats::StatisticsFactory>("stats", "Statistics methods for interpreting field values"));

        options_.push_back(new SimpleOption< double >("lower-limit", "count lower limit (count-outside-range)"));
        options_.push_back(new SimpleOption< double >("upper-limit", "count upper limit (count-outside-range)"));
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
            std::string stats = "scalar";
            const MIRParametrisation& c = combined;
            c.get("stats", stats);


            // Calculate and show statistics
            eckit::ScopedPtr<const mir::stats::Statistics> s(mir::stats::StatisticsFactory::build(stats, combined));
            eckit::Log::info() << s->calculate(input.field()) << std::endl;

        }
    }

}


int main(int argc, char **argv) {
    MIRStatistics tool(argc, argv);
    return tool.start();
}

