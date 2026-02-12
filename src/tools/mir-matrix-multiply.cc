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


#include "eckit/linalg/LinearAlgebraSparse.h"
#include "eckit/linalg/SparseMatrix.h"
#include "eckit/linalg/Vector.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "mir/tools/MIRTool.h"
#include "mir/util/Log.h"
#include "mir/util/Trace.h"


namespace mir::tools {


struct MIRMatrixMultiply : MIRTool {
    MIRMatrixMultiply(int argc, char** argv) : MIRTool(argc, argv) {
        options_.push_back(
            new eckit::option::SimpleOption<std::string>("backend", "LinearAlgebraSparse backend", "generic"));
        options_.push_back(new eckit::option::SimpleOption<size_t>("loop", "loop number of times", 1));
    }

    int numberOfPositionalArguments() const override { return 1; }

    void usage(const std::string& tool) const override {
        Log::info() << "\n"
                       "Usage: time env MIR_TRACE_RESOURCE_USAGE=1 "
                    << tool << " [--backend=...]"
                    << " [--loop=100]"
                       " input-matrix"
                    << std::endl;
    }

    void execute(const eckit::option::CmdArgs&) override;
};


void MIRMatrixMultiply::execute(const eckit::option::CmdArgs& args) {
    const auto& la = eckit::linalg::LinearAlgebraSparse::backend(args.getString("backend"));
    Log::info() << la << std::endl;

    eckit::linalg::SparseMatrix A;
    A.load(args(0));
    Log::info() << A << std::endl;

    eckit::linalg::Vector y(A.rows());
    eckit::linalg::Vector x(A.cols());
    x.fill(2.);

    {
        trace::Timer trace("spmv");
        for (size_t i = 0, n(args.getUnsigned("loop", 1)); i < n; ++i) {
            la.spmv(A, x, y);
        }
    }
}


}  // namespace mir::tools


int main(int argc, char** argv) {
    mir::tools::MIRMatrixMultiply tool(argc, argv);
    return tool.start();
}
