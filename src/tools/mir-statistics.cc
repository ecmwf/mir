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
#include "eckit/option/CmdArgs.h"
#include "eckit/option/FactoryOption.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/utils/StringTools.h"
#include "eckit/utils/Tokenizer.h"

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/output/GribFileOutput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/stats/Method.h"
#include "mir/stats/Statistics.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/MIRStatistics.h"
#include "mir/util/Pretty.h"


class MIRStatistics : public mir::tools::MIRTool {
private:

    void execute(const eckit::option::CmdArgs&);

    void usage(const std::string& tool) const;

    int minimumPositionalArguments() const {
        return 1;
    }

    struct PerPointStatistics {
        static void list(std::ostream& out) {
            out << eckit::StringTools::join(", ", perPointStats())<< std::endl;
        }
        static const std::vector<std::string> perPointStats() {
            return {"mean", "variance", "stddev"};
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
    if (args_wrap.get("output", output)) {

        // read in first field to reset statistics and reference representation
        mir::input::GribFileInput firstGribFile(args(0));
        ASSERT(firstGribFile.next());

        const mir::input::MIRInput& firstInput(firstGribFile);
        mir::repres::RepresentationHandle reference(firstInput.field().representation());

        size_t Nfirst = firstInput.field().values(0).size();
        ASSERT(Nfirst > 0);
        eckit::Log::info() << "Using " << mir::util::Pretty(Nfirst, "grid point") << std::endl;

        // set paramId/metadata-specific method per-point statistics
        std::string statistics = "scalar";
        args_wrap.get("statistics", statistics);

        // per-point statistics
        std::unique_ptr<mir::param::MIRParametrisation> param(new mir::param::CombinedParametrisation(args_wrap, firstGribFile, defaults));
        std::unique_ptr<mir::stats::Method> pps(mir::stats::MethodFactory::build(statistics, *param));
        pps->resize(Nfirst);

        for (auto& arg : args) {
            mir::input::GribFileInput grib(arg);
            const mir::input::MIRInput& input = grib;

            size_t count = 0;
            while (grib.next()) {
                eckit::Log::info() << "\n'" << arg << "' #" << ++count << std::endl;

                mir::repres::RepresentationHandle repres(input.field().representation());
                if (!repres->sameAs(*reference)) {
                    eckit::Log::error() << "Input not expected,"
                                           "\n" "expected " << *reference
                                        << "\n" "but got " << *repres;
                    throw eckit::UserError("input not of the expected format");
                }

                pps->execute(input.field());
            }
        }

        // Write statistics
        eckit::Tokenizer parse("/");
        std::vector<std::string> outputs;
        parse(output, outputs);

        for (auto& j : outputs) {
            eckit::Log::info() << "Writing '" << j << "'" << std::endl;

            mir::util::MIRStatistics stats;
            mir::context::Context ctx(firstGribFile, stats);

            auto& f = ctx.field();
            j == "mean" ? pps->mean(f) :
            j == "variance" ? pps->variance(f) :
            j == "stddev" ? pps->stddev(f) :
            throw eckit::UserError("Output " + j + "' not supported");

            std::unique_ptr<mir::output::MIROutput> out(new mir::output::GribFileOutput(j));
            out->save(*param, ctx);
        }

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
            std::string statistics = "scalar";
            args_wrap.get("statistics", statistics);

            // Calculate and show statistics
            std::unique_ptr<mir::param::MIRParametrisation> param(new mir::param::CombinedParametrisation(args_wrap, grib, defaults));
            std::unique_ptr<mir::stats::Statistics> stats(mir::stats::StatisticsFactory::build(statistics, *param));
            stats->execute(input.field());

            eckit::Log::info() << *stats << std::endl;
        }
    }
}


int main(int argc, char **argv) {
    MIRStatistics tool(argc, argv);
    return tool.start();
}

