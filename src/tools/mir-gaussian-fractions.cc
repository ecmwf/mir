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
#include "eckit/option/SimpleOption.h"
#include "eckit/types/Fraction.h"

#include "mir/util/Atlas.h"

#include "mir/param/ConfigurationWrapper.h"
#include "mir/stats/detail/PNormsT.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Log.h"
#include "mir/util/Types.h"


namespace mir::tools {


using statistics_t = stats::detail::PNormsT<double>;


struct MIRGaussianFractions : MIRTool {
    MIRGaussianFractions(int argc, char** argv) : MIRTool(argc, argv) {
        using eckit::option::SimpleOption;

        options_.push_back(new SimpleOption<size_t>("N", "Gaussian N, default 1280"));
        options_.push_back(new SimpleOption<size_t>("Nmin", "Gaussian N range minimum, default 0"));
        options_.push_back(new SimpleOption<size_t>("Nmax", "Gaussian N range maximum, default 0"));
    }

    int numberOfPositionalArguments() const override { return 0; }

    void usage(const std::string& tool) const override {
        Log::info() << "\nStatistics of the difference beween Gaussian coordinates converted using Fractions."
                       "\n"
                       "\nUsage: "
                    << tool << " [--N=ordinal]" << std::endl;
    }

    void execute(const eckit::option::CmdArgs& /*args*/) override;
};


statistics_t* evaluateGaussianN(const size_t N, const param::MIRParametrisation& /*unused*/) {

    // This returns the Gaussian latitudes of the North hemisphere
    std::vector<double> latitudes(N);
    atlas::util::gaussian_latitudes_npole_equator(N, latitudes.data());

    auto* stats = new statistics_t;
    for (const double& l : latitudes) {
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

    const param::ConfigurationWrapper param(args);
    std::unique_ptr<statistics_t> statistics(new statistics_t);


    if (Nmin <= Nmax) {

        std::unique_ptr<statistics_t> worse(statistics.get());

        bool first = true;
        for (size_t n = Nmin; n <= Nmax; n += 2) {
            Log::info() << "Evaluating N=" << n << std::endl;

            std::unique_ptr<statistics_t> stats(evaluateGaussianN(n, param));

            if (first || stats->normL1() > worse->normL1()) {
                worse.reset(stats.get());
                first = false;
            }
        }

        Log::info() << "\n"
                       "Δlat: ";
        worse->print(Log::info());
        Log::info() << std::endl;
    }
    else {

        evaluateGaussianN(N, param);
        Log::info() << "\n"
                       "Δlat: ";
        statistics->print(Log::info());
        Log::info() << std::endl;
    }
}


}  // namespace mir::tools


int main(int argc, char** argv) {
    mir::tools::MIRGaussianFractions tool(argc, argv);
    return tool.start();
}
