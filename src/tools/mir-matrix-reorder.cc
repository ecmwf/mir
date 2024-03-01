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


#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "eckit/linalg/SparseMatrix.h"
#include "eckit/linalg/Triplet.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/FactoryOption.h"
#include "eckit/option/SimpleOption.h"

#include "mir/reorder/Reorder.h"
#include "mir/repres/Representation.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::tools {


using Reorder = reorder::Reorder;


struct MIRMatrixReorder : MIRTool {
    MIRMatrixReorder(int argc, char** argv) : MIRTool(argc, argv) {
        options_.push_back(new eckit::option::FactoryOption<reorder::ReorderFactory>(
            "reorder-rows", "Reordering rows method", "identity"));
        options_.push_back(new eckit::option::FactoryOption<reorder::ReorderFactory>(
            "reorder-cols", "Reordering columns method", "identity"));
        options_.push_back(new eckit::option::SimpleOption<bool>("transpose", "Transpose matrix", false));
    }

    int numberOfPositionalArguments() const override { return 2; }

    void usage(const std::string& tool) const override {
        Log::info() << "\n"
                       "Usage: "
                    << tool
                    << " [--reorder-rows=...]"
                       " [--reorder-cols=...]"
                       " [--transpose=[0|1]]"
                       " input-matrix output-matrix"
                    << std::endl;
    }

    void execute(const eckit::option::CmdArgs&) override;
};


void MIRMatrixReorder::execute(const eckit::option::CmdArgs& args) {
    // load input matrix
    eckit::linalg::SparseMatrix M;
    M.load(args(0));


    // renumbering maps
    auto rows =
        std::unique_ptr<Reorder>(reorder::ReorderFactory::build(args.getString("reorder-rows")))->reorder(M.rows());
    ASSERT(rows.size() == M.rows());

    auto cols =
        std::unique_ptr<Reorder>(reorder::ReorderFactory::build(args.getString("reorder-cols")))->reorder(M.cols());
    ASSERT(cols.size() == M.cols());


    // expand triplets, renumbering directly
    std::vector<eckit::linalg::Triplet> trips;
    trips.reserve(M.nonZeros());

    auto transpose = args.getBool("transpose");
    for (auto i = M.begin(), end = M.end(); i != end; ++i) {
        if (transpose) {
            trips.emplace_back(cols.at(i.col()), rows.at(i.row()), *i);
        }
        else {
            trips.emplace_back(rows.at(i.row()), cols.at(i.col()), *i);
        }
    }


    // compress triplets, create output matrix
    std::sort(trips.begin(), trips.end());
    eckit::linalg::SparseMatrix W(M.rows(), M.cols(), trips);


    // create output matrix
    W.save(args(1));
}


}  // namespace mir::tools


int main(int argc, char** argv) {
    mir::tools::MIRMatrixReorder tool(argc, argv);
    return tool.start();
}
