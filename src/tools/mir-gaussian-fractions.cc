/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Jun 2017


#include <cmath>

#include "eckit/log/Log.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/types/Fraction.h"

#include "atlas/util/GaussianLatitudes.h"

#include "mir/param/ConfigurationWrapper.h"
#include "mir/stats/StatisticsT.h"
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


using statistics_t = mir::stats::StatisticsT<mir::stats::detail::Scalar>;


statistics_t* evaluateGaussianN(const size_t N, const mir::param::MIRParametrisation& param) {

    // This returns the Gaussian latitudes of the North hemisphere
    std::vector<double> latitudes(N);
    atlas::util::gaussian_latitudes_npole_equator(N, latitudes.data());

    statistics_t* stats = new statistics_t(param);
    for (const double& l: latitudes) {
        const double f = double(eckit::Fraction(l));
        stats->operator()(f - l);
    }

    return stats;
}


void MIRGaussianFractions::execute(const eckit::option::CmdArgs& args) {

    size_t N = 1280;
    args.get("N", N);
    ASSERT(N);

    size_t Nmin = 2;
    args.get("Nmin", Nmin);

    size_t Nmax = 0;
    args.get("Nmax", Nmax);

    const mir::param::ConfigurationWrapper param(args);
    eckit::ScopedPtr<statistics_t> stats(new statistics_t(param));


    if (Nmin <= Nmax) {

        eckit::ScopedPtr<statistics_t> worse(stats.get());

        bool first = true;
        for (size_t N = Nmin; N <= Nmax; N+=2) {
            eckit::Log::info() << "Evaluating N=" << N << std::endl;

            eckit::ScopedPtr<statistics_t> stats(evaluateGaussianN(N, param));

            if (first || stats->normL1() > worse->normL1()) {
                worse.reset(stats.get());
                first = false;
            }
        }

        eckit::Log::info() << "\n" "Δlat:" << *worse << std::endl;

    } else {

        evaluateGaussianN(N, param);
        eckit::Log::info() << "\n" "Δlat:" << stats << std::endl;

    }
}


int main(int argc, char **argv) {
    MIRGaussianFractions tool(argc, argv);
    return tool.start();
}

