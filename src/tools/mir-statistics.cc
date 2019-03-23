/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <memory>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"
#include "eckit/log/Plural.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/FactoryOption.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/utils/StringTools.h"

#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/stats/Statistics.h"
#include "mir/tools/MIRTool.h"


class MIRStatistics : public mir::tools::MIRTool {
private:

    void execute(const eckit::option::CmdArgs&);

    void usage(const std::string& tool) const;

    int minimumPositionalArguments() const {
        return 1;
    }

    struct PerPointStatistics : std::vector<mir::stats::Statistics*> {
        static void list(std::ostream& out) {
            out << eckit::StringTools::join(", ", perPointStats())<< std::endl;
        }
        static const std::vector<std::string> perPointStats() {
            return  {"mean", "variance", "stddev", "min", "max"};
        }

        PerPointStatistics(size_t N, const std::string& name, const mir::param::MIRParametrisation& param) :
            std::vector<mir::stats::Statistics*>(N) {
            for (auto& p : *this) {
                p = mir::stats::StatisticsFactory::build(name, param);
            }
        }

        ~PerPointStatistics() {
            for (auto& p : *this) {
                delete p;
            }
        }
    };

public:

    MIRStatistics(int argc, char **argv) : mir::tools::MIRTool(argc, argv) {
        using eckit::option::FactoryOption;
        using eckit::option::SimpleOption;

        options_.push_back(new FactoryOption<mir::stats::StatisticsFactory>("statistics", "Statistics methods for interpreting field values"));
        options_.push_back(new SimpleOption< double >("counter-lower-limit", "count lower limit"));
        options_.push_back(new SimpleOption< double >("counter-upper-limit", "count upper limit"));
        options_.push_back(new FactoryOption<PerPointStatistics>("output", "/-separated list of per-point statistics (output GRIB to <statistics>"));
    }
};


void MIRStatistics::usage(const std::string &tool) const {
    eckit::Log::info()
            << "\n" "Calculate field statistics, or per-point statistics when specifying output."
               "\n"
               "\n" "Usage: " << tool << " [--statistics=option] file.grib [file2.grib [...]]"
               "\n"
               "\n" "Examples:"
               "\n" "  % " << tool << " file.grib"
               "\n" "  % " << tool << " --statistics=scalar file1.grib file2.grib file3.grib"
               "\n" "  % " << tool << " --statistics=spectral file.grib"
               "\n" "  % " << tool << " --output=mean/min/max file1.grib file2.grib file3.grib"
            << std::endl;
}


void MIRStatistics::execute(const eckit::option::CmdArgs& args) {

    // Build CombinedParametrisation from a few parts:
    // - wrap the arguments, so that they behave as a MIRParametrisation
    // - input grib metadata
    // - lookup configuration for input metadata-specific parametrisation

    const mir::param::ConfigurationWrapper args_wrap(args);
    const mir::param::DefaultParametrisation defaults;

    // on 'output' option, calculate per-point statistics
    std::string output;
    if (args_wrap.get("output", output) && !output.empty()) {

        // read in first field to reset statistics and reference representation
        mir::input::GribFileInput firstGribFile(args(0));
        ASSERT(firstGribFile.next());

        const mir::input::MIRInput& input(firstGribFile);
        mir::repres::RepresentationHandle R(input.field().representation());

        size_t N = input.field().values(0).size();
        ASSERT(N > 0);
        eckit::Log::info() << "Using " << eckit::Plural(N, "grid point") << std::endl;

        // set paramId/metadata-specific method per-point statistics
        std::unique_ptr<mir::param::MIRParametrisation> param(new mir::param::CombinedParametrisation(args_wrap, firstGribFile, defaults));
        std::string statistics = "scalar";
        param->get("statistics", statistics);

        PerPointStatistics pps(N, statistics, *param);

        // TODO

        return;
    }

    // per-field statistics
    for (auto& arg : args) {
        mir::input::GribFileInput grib(arg);
        const mir::input::MIRInput& input = grib;

        size_t count = 0;
        while (grib.next()) {
            eckit::Log::info() << "\n'" << arg << "' #" << ++count << std::endl;

            // paramId/metadata-specific method
            std::unique_ptr<mir::param::MIRParametrisation> param(new mir::param::CombinedParametrisation(args_wrap, grib, defaults));
            std::string statistics = "scalar";
            param->get("statistics", statistics);

            // Calculate and show statistics
<<<<<<< HEAD
            std::unique_ptr<mir::stats::Statistics> stats(mir::stats::StatisticsFactory::build(statistics, combined));
=======
            std::unique_ptr<mir::stats::Statistics> stats(mir::stats::StatisticsFactory::build(statistics, *param));
>>>>>>> mir-statistics
            stats->execute(input.field());

            eckit::Log::info() << *stats << std::endl;
        }
    }
}


int main(int argc, char **argv) {
    MIRStatistics tool(argc, argv);
    return tool.start();
}

