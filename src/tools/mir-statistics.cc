/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <memory>

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
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"
#include "mir/util/Types.h"


using namespace mir;
using prec_t = decltype(std::cout.precision());


struct PerPointStatistics {
    static void list(std::ostream& out) { out << eckit::StringTools::join(", ", perPointStats()) << std::endl; }
    static std::vector<std::string> perPointStats() { return {"mean", "variance", "stddev"}; }
};


struct MIRStatistics : tools::MIRTool {
    MIRStatistics(int argc, char** argv) : MIRTool(argc, argv) {
        using namespace eckit::option;

        options_.push_back(new FactoryOption<stats::StatisticsFactory>(
            "statistics", "Statistics methods for interpreting field values"));
        options_.push_back(new SimpleOption<double>("counter-lower-limit", "count lower limit"));
        options_.push_back(new SimpleOption<double>("counter-upper-limit", "count upper limit"));
        options_.push_back(new FactoryOption<PerPointStatistics>(
            "output", "/-separated list of per-point statistics (output GRIB to <statistics>"));
        options_.push_back(new SimpleOption<prec_t>("precision", "Output precision"));
    }

    int minimumPositionalArguments() const override { return 1; }

    void usage(const std::string& tool) const override {
        Log::info() << "\n"
                       "Calculate field statistics, or per-point statistics when specifying output."
                       "\n"
                       "\n"
                       "Usage: "
                    << tool
                    << " [--statistics=option] file.grib [file2.grib [...]]"
                       "\n"
                       "\n"
                       "Examples:"
                       "\n"
                       "  % "
                    << tool
                    << " file.grib"
                       "\n"
                       "  % "
                    << tool
                    << " --statistics=scalar file1.grib file2.grib file3.grib"
                       "\n"
                       "  % "
                    << tool
                    << " --statistics=spectral file.grib"
                       "\n"
                       "  % "
                    << tool << " --output=mean/min/max file1.grib file2.grib file3.grib" << std::endl;
    }


    void execute(const eckit::option::CmdArgs&) override;
};


void MIRStatistics::execute(const eckit::option::CmdArgs& args) {

    // Build CombinedParametrisation from a few parts:
    // - wrap the arguments, so that they behave as a MIRParametrisation
    // - input grib metadata
    // - lookup configuration for input metadata-specific parametrisation

    const param::ConfigurationWrapper args_wrap(args);
    const param::DefaultParametrisation defaults;

    auto& log = Log::info();
    prec_t precision;
    auto old = args.get("precision", precision) ? log.precision(precision) : log.precision();

    // on 'output' option, calculate per-point statistics
    std::string output;
    if (args_wrap.get("output", output)) {

        // read in first field to reset statistics and reference representation
        input::GribFileInput firstGribFile(args(0));
        ASSERT(firstGribFile.next());

        const input::MIRInput& firstInput(firstGribFile);
        repres::RepresentationHandle reference(firstInput.field().representation());

        size_t Nfirst = firstInput.field().values(0).size();
        ASSERT(Nfirst > 0);
        log << "Using " << Log::Pretty(Nfirst, {"grid point"}) << std::endl;

        // set paramId/metadata-specific method per-point statistics
        std::string statistics = "scalar";
        args_wrap.get("statistics", statistics);

        // per-point statistics
        std::unique_ptr<param::MIRParametrisation> param(
            new param::CombinedParametrisation(args_wrap, firstGribFile, defaults));
        std::unique_ptr<stats::Method> pps(stats::MethodFactory::build(statistics, *param));
        pps->resize(Nfirst);

        for (auto& arg : args) {
            input::GribFileInput grib(arg);
            const input::MIRInput& input = grib;

            size_t count = 0;
            while (grib.next()) {
                log << "\n'" << arg << "' #" << ++count << std::endl;

                repres::RepresentationHandle repres(input.field().representation());
                if (!repres->sameAs(*reference)) {
                    Log::error() << "Input not expected,"
                                    "\n"
                                    "expected "
                                 << *reference
                                 << "\n"
                                    "but got "
                                 << *repres;
                    throw exception::UserError("input not of the expected format");
                }

                pps->execute(input.field());
            }
        }

        // Write statistics
        eckit::Tokenizer parse("/");
        std::vector<std::string> outputs;
        parse(output, outputs);

        for (auto& j : outputs) {
            log << "Writing '" << j << "'" << std::endl;

            util::MIRStatistics stats;
            context::Context ctx(firstGribFile, stats);

            auto& f = ctx.field();
            j == "mean"       ? pps->mean(f)
            : j == "variance" ? pps->variance(f)
            : j == "stddev"   ? pps->stddev(f)
                              : throw exception::UserError("Output " + j + "' not supported");

            std::unique_ptr<output::MIROutput> out(new output::GribFileOutput(j));
            out->save(*param, ctx);
        }

        log.precision(old);
        return;
    }

    // per-field statistics
    for (auto& arg : args) {
        input::GribFileInput grib(arg);
        const input::MIRInput& input = grib;

        size_t count = 0;
        while (grib.next()) {
            log << "\n'" << arg << "' #" << ++count << std::endl;

            // paramId/metadata-specific method
            std::string statistics = "scalar";
            args_wrap.get("statistics", statistics);

            // Calculate and show statistics
            std::unique_ptr<param::MIRParametrisation> param(
                new param::CombinedParametrisation(args_wrap, grib, defaults));
            std::unique_ptr<stats::Statistics> stats(stats::StatisticsFactory::build(statistics, *param));
            stats->execute(input.field());

            log << *stats << std::endl;
        }
    }

    log.precision(old);
}


int main(int argc, char** argv) {
    MIRStatistics tool(argc, argv);
    return tool.start();
}
