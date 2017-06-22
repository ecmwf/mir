/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Jun 2017


#include <cmath>
#include "atlas/util/GaussianLatitudes.h"
#include "eckit/log/Log.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/types/Fraction.h"
#include "mir/stats/detail/Scalar.h"
#include "mir/tools/MIRTool.h"


class MIRGaussianFractions : public mir::tools::MIRTool {
private:
    void execute(const eckit::option::CmdArgs&);
    void usage(const std::string& tool) const;
    int numberOfPositionalArguments() const { return 0; }
public:
    MIRGaussianFractions(int argc, char **argv) : mir::tools::MIRTool(argc, argv) {
        options_.push_back(new eckit::option::SimpleOption< size_t >("N", "Gaussian N, default 1280"));
        options_.push_back(new eckit::option::SimpleOption< size_t >("Nmin", "Gaussian N range minimum, default 0"));
        options_.push_back(new eckit::option::SimpleOption< size_t >("Nmax", "Gaussian N range maximum, default 0"));
    }
};


void MIRGaussianFractions::usage(const std::string &tool) const {
    eckit::Log::info()
            << "\nStatistics of the difference beween Gaussian coordinates converted using Fractions."
               "\n"
               "\nUsage: " << tool << " [--N=ordinal]"
            << std::endl;
}


typedef mir::stats::detail::Scalar<double> statistics_t;


statistics_t evaluateGaussianN(const size_t N) {

    // This returns the Gaussian latitudes of the North hemisphere
    std::vector<double> latitudes(N);
    atlas::util::gaussian_latitudes_npole_equator(N, latitudes.data());

    statistics_t stats;
    for (const double& l: latitudes) {
        const double f = double(eckit::Fraction(l));
        stats(f - l);
    }

    return stats;
}


eckit::Channel& operator<<(eckit::Channel& s, const statistics_t& stats) {
    s << "Δlat:"
      << "\n\t" "N = " << stats.count()
      << "\n\t" "min = " << stats.min() << "\tminIndex = " << stats.minIndex()
      << "\n\t" "max = " << stats.max() << "\tmaxIndex = " << stats.maxIndex()
      << "\n"
      << "\n\t" "mean     = " << stats.mean()
      << "\n\t" "variance = " << stats.variance()
      << "\n\t" "stdev    = " << stats.standardDeviation()
      << "\n\t" "skewness = " << stats.skewness()
      << "\n\t" "kurtosis = " << stats.kurtosis()
      << "\n"
      << "\n\t" "||L1|| = " << stats.normL1()
      << "\n\t" "||L2|| = " << stats.normL2()
      << "\n\t" "||L∞|| = " << stats.normLinfinity();
    return s;
}


void MIRGaussianFractions::execute(const eckit::option::CmdArgs& args) {

    size_t N = 1280;
    args.get("N", N);
    ASSERT(N);

    size_t Nmin = 2;
    args.get("Nmin", Nmin);

    size_t Nmax = 0;
    args.get("Nmax", Nmax);

    statistics_t stats;


    if (Nmin <= Nmax) {

        bool first = true;
        statistics_t worse;
        for (size_t N = Nmin; N <= Nmax; N+=2) {
            eckit::Log::info() << "Evaluating N=" << N << std::endl;

            statistics_t stats = evaluateGaussianN(N);

            if (first || stats.normL1() > worse.normL1()) {
                worse = stats;
                first = false;
            }
        }

        eckit::Log::info()
                << "\n" "Δlat:"
                << "\n\t" "N = " << worse.count()
                << "\n\t" "min = " << worse.min() << "\tminIndex = " << worse.minIndex()
                << "\n\t" "max = " << worse.max() << "\tmaxIndex = " << worse.maxIndex()
                << "\n"
                << "\n\t" "mean     = " << worse.mean()
                << "\n\t" "variance = " << worse.variance()
                << "\n\t" "stdev    = " << worse.standardDeviation()
                << "\n\t" "skewness = " << worse.skewness()
                << "\n\t" "kurtosis = " << worse.kurtosis()
                << "\n"
                << "\n\t" "||L1|| = " << worse.normL1()
                << "\n\t" "||L2|| = " << worse.normL2()
                << "\n\t" "||L∞|| = " << worse.normLinfinity()
                << std::endl;

    } else {

        evaluateGaussianN(N);
        eckit::Log::info()
                << "Δlat:"
                << "\n\t" "N = " << stats.count()
                << "\n\t" "min = " << stats.min() << "\tminIndex = " << stats.minIndex()
                << "\n\t" "max = " << stats.max() << "\tmaxIndex = " << stats.maxIndex()
                << "\n"
                << "\n\t" "mean     = " << stats.mean()
                << "\n\t" "variance = " << stats.variance()
                << "\n\t" "stdev    = " << stats.standardDeviation()
                << "\n\t" "skewness = " << stats.skewness()
                << "\n\t" "kurtosis = " << stats.kurtosis()
                << "\n"
                << "\n\t" "||L1|| = " << stats.normL1()
                << "\n\t" "||L2|| = " << stats.normL2()
                << "\n\t" "||L∞|| = " << stats.normLinfinity()
                << std::endl;

    }
}


int main(int argc, char **argv) {
    MIRGaussianFractions tool(argc, argv);
    return tool.start();
}

