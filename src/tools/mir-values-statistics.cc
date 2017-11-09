/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date July 2017


#include "eckit/log/Log.h"
#include "eckit/log/Plural.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/output/GribFileOutput.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/stats/Results.h"
#include "mir/stats/detail/ScalarCentralMomentsFn.h"
#include "mir/stats/detail/ScalarMinMaxFn.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Compare.h"
#include "mir/util/MIRStatistics.h"


class MIRValuesStatistics : public mir::tools::MIRTool {
private:
    void execute(const eckit::option::CmdArgs&);
    void usage(const std::string& tool) const;
    int minimumPositionalArguments() const { return 1; }
public:
    MIRValuesStatistics(int argc, char **argv) : mir::tools::MIRTool(argc, argv) {
        using namespace eckit::option;
        options_.push_back(new SimpleOption< std::string >("output-mean", "output per-value mean to GRIB file"));
        options_.push_back(new SimpleOption< std::string >("output-variance", "output per-value variance to GRIB file"));
        options_.push_back(new SimpleOption< std::string >("output-stddev", "output per-value standard deviation to GRIB file"));
    }
};


void MIRValuesStatistics::usage(const std::string &tool) const {
    eckit::Log::info()
            << "\nUsage: " << tool << " [--output-mean=<file>] [--output-variance=<file>] [--output-stddev=<file>] 1.grib 2.grib [3.grib [...]]"
               "\nExamples:"
               "\n  % " << tool << " 1.grib 2.grib"
               "\n  % " << tool << " --output-mean=mean.grib 1.grib 2.grib 3.grib"
               "\n  % " << tool << " --output-mean=mean.grib --output-stddev=stddev.grib 1.grib 2.grib"
            << std::endl;
}


typedef mir::stats::detail::ScalarCentralMomentsFn<double> statistics_t;


void getStatisticsValues(mir::context::Context& ctx, mir::output::MIROutput& out, const std::string& name, const std::vector<statistics_t>& stats) {
    const size_t N = stats.size();
    ASSERT(N);

    std::vector<double> values(N);
    size_t i = 0;

    mir::param::SimpleParametrisation dummy;

    if (name == "mean") {
        for (const auto& s : stats) {
            ASSERT(i < N);
            values[i++] = s.mean();
        }
    } else if (name == "variance") {
        for (const auto& s : stats) {
            ASSERT(i < N);
            values[i++] = s.variance();
        }
    } else if (name == "stddev") {
        for (const auto& s : stats) {
            ASSERT(i < N);
            values[i++] = s.standardDeviation();
        }
    } else {
        throw eckit::SeriousBug("unknown statistics '" + name + "'");
    }

    ctx.field().update(values, 0);
    out.save(dummy, ctx);
}


void MIRValuesStatistics::execute(const eckit::option::CmdArgs& args) {

    // setup per-value statistics vector
    mir::input::GribFileInput firstGribFile(args(0));
    ASSERT(firstGribFile.next());

    size_t N = 0;
    {
        mir::data::MIRField field = static_cast<const mir::input::MIRInput&>(firstGribFile).field();
        N = field.values(0).size();
        ASSERT(N > 0);
    }

    eckit::Log::info() << "Using " << eckit::Plural(N, "grid point") << std::endl;;
    std::vector< statistics_t > statistics(N);


    // - for each GRIB file,
    //   - for each GRIB message,
    //     - for each field (dimension),
    //       - for each value,
    // calculate statistics
    for (size_t i = 0; i < args.count(); ++i) {
        mir::input::GribFileInput grib(args(i));

        size_t count = 0;
        while (grib.next()) {
            eckit::Log::info() << "\n'" << args(i) << "' #" << ++count << std::endl;

            const mir::data::MIRField field = static_cast<const mir::input::MIRInput&>(grib).field();
            const mir::util::compare::IsMissingFn missingValue(field.hasMissing()? field.missingValue() : std::numeric_limits<double>::quiet_NaN());
            ASSERT(N == field.values(0).size());

            for (size_t d = 0, i = 0; d < field.dimensions(); ++d, i = 0) {
                for (double v : field.values(d)) {
                    ASSERT(i < N);
                    if (!missingValue(v)) {
                        statistics[i++](v);
                    }
                }
            }

        }
    }


    // Write to GRIB file(s) or display each statistics
    if (args.has("output-mean") || args.has("output-variance") || args.has("output-stddev")) {

        std::string output;
        mir::util::MIRStatistics stats;
        mir::context::Context ctx(firstGribFile, stats);

        if (args.get("output-mean", output) && output.length()) {
            mir::output::GribFileOutput out(output);
            getStatisticsValues(ctx, out, "mean", statistics);
        }

        if (args.get("output-variance", output) && output.length()) {
            mir::output::GribFileOutput out(output);
            getStatisticsValues(ctx, out, "variance", statistics);
        }

        if (args.get("output-stddev", output) && output.length()) {
            mir::output::GribFileOutput out(output);
            getStatisticsValues(ctx, out, "stddev", statistics);
        }


    } else {

        // Show statistics results
        for (const auto& stats : statistics) {
            mir::stats::Results results;

            results.absoluteQuantity ("mean")     = stats.mean();
            results.absoluteQuantity2("variance") = stats.variance();
            results.absoluteQuantity ("stddev")   = stats.standardDeviation();

            eckit::Log::info() << results << std::endl;
        }

    }
}


int main(int argc, char **argv) {
    MIRValuesStatistics tool(argc, argv);
    return tool.start();
}

