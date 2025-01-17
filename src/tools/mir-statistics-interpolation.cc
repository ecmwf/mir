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


#include "eckit/option/CmdArgs.h"

#include <cmath>
#include <sstream>

#include "eckit/option/SimpleOption.h"
#include "eckit/serialisation/FileStream.h"

#include "mir/tools/MIRTool.h"
#include "mir/util/Log.h"


namespace mir::tools {


double combinedVariance(size_t nA, double meanA, double varA, size_t nB, double meanB, double varB) {
    auto d = [](auto v) { return static_cast<double>(v); };

    return ((nA - 1) * varA + (nB - 1) * varB + d(nA * nB) / d(nA + nB) * (meanA - meanB) * (meanA - meanB)) /
           d(nA + nB - 1);
}


double combinedStandardDeviation(size_t nA, double meanA, double varA, size_t nB, double meanB, double varB) {
    return std::sqrt(combinedVariance(nA, meanA, varA, nB, meanB, varB));
}


struct MIRStatisticsInterpolation : MIRTool {
    MIRStatisticsInterpolation(int argc, char** argv, const char* homeenv = nullptr) : MIRTool(argc, argv) {
        options_.push_back(new eckit::option::SimpleOption<std::string>("mode", "writing mode"));
    }

    void usage(const std::string& tool) const override {
        Log::info() << "\n"
                       "Usage: "
                    << tool << std::endl;
    }

    void execute(const eckit::option::CmdArgs&) override;
};


template <typename T>
void write_value(eckit::Stream& stream, const std::string& str) {
    T value{};
    std::istringstream(str) >> value;
    stream << value;
}


void MIRStatisticsInterpolation::execute(const eckit::option::CmdArgs& args) {
    struct stats_t {
        size_t n;
        double mean;
        double var;
    } A{100, 50., 25.}, B{150, 55., 30.};

    double variance = combinedVariance(A.n, A.mean, A.var, B.n, B.mean, B.var);
    double stddev   = combinedStandardDeviation(A.n, A.mean, A.var, B.n, B.mean, B.var);

    std::cout << "Combined Variance: " << variance << std::endl;
    std::cout << "Combined Standard Deviation: " << stddev << std::endl;
}


}  // namespace mir::tools


int main(int argc, char** argv) {
    mir::tools::MIRStatisticsInterpolation tool(argc, argv);
    return tool.start();
}
