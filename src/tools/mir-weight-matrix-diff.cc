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


#include <cmath>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/types/FloatCompare.h"

#include "mir/caching/matrix/MatrixLoader.h"
#include "mir/method/WeightMatrix.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/stats/detail/CounterBinary.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::tools {


struct MIRWeightMatrixDiff : MIRTool {
    MIRWeightMatrixDiff(int argc, char** argv) : MIRTool(argc, argv) {
        using eckit::option::SimpleOption;

        options_.push_back(new SimpleOption<std::string>("matrix-loader", "Matrix loader mechanism"));
        options_.push_back(new SimpleOption<bool>("matrix-validate", "Matrix validation after loading"));
        options_.push_back(
            new SimpleOption<double>("absolute-error", "Matrix comparison non-zero maximum allowed error"));
        options_.push_back(new SimpleOption<bool>("counter", "Use statistics counter"));
    }

    int numberOfPositionalArguments() const override { return 2; }

    void usage(const std::string& tool) const override {
        Log::info()
            << "\n"
            << "Usage: " << tool
            << " [--matrix-loader=<loader>] [--matrix-validate] [--absolute-error=eps] [--counter] <path> <path>"
            << std::endl;
    }

    void execute(const eckit::option::CmdArgs& /*args*/) override;
};


struct diff_t {
    diff_t()                                          = default;
    virtual ~diff_t()                                 = default;
    virtual bool operator()(double a, double b) const = 0;

    diff_t(const diff_t&)            = delete;
    diff_t(diff_t&&)                 = delete;
    diff_t& operator=(const diff_t&) = delete;
    diff_t& operator=(diff_t&&)      = delete;
};


struct strict_diff_t : diff_t {
    bool operator()(double a, double b) const override { return a != b; }
};


struct approximate_diff_t : diff_t {
    approximate_diff_t(double eps) : eps_(eps) {}
    bool operator()(double a, double b) const override { return !eckit::types::is_approximately_equal(a, b, eps_); }
    const double eps_;
};


void MIRWeightMatrixDiff::execute(const eckit::option::CmdArgs& args) {
    using caching::matrix::MatrixLoaderFactory;
    using method::WeightMatrix;

    struct shape_t : std::vector<WeightMatrix::Size> {
        shape_t(const WeightMatrix& m) : std::vector<WeightMatrix::Size>{m.nonZeros(), m.rows(), m.cols()} {}
    };

    const param::ConfigurationWrapper param(args);
    auto& log = Log::info();

    std::string matrixLoader = caching::matrix::MatrixLoader::default_loader();
    param.get("matrix-loader", matrixLoader);

    bool matrixValidate;
    param.get("matrix-validate", matrixValidate);

    bool counter;
    param.get("counter", counter);

    WeightMatrix a(MatrixLoaderFactory::build(matrixLoader, args(0)));
    if (matrixValidate) {
        a.validate(("load '" + args(0) + "'").c_str());
    }

    WeightMatrix b(MatrixLoaderFactory::build(matrixLoader, args(1)));
    if (matrixValidate) {
        b.validate(("load '" + args(1) + "'").c_str());
    }

    shape_t aShape(a);
    shape_t bShape(b);
    if (aShape != bShape) {
        log << "!= geometry: " << aShape << " != " << bShape << std::endl;
    }
    else if (counter) {
        stats::detail::CounterBinary counter(param, param);

        auto i = a.begin();
        auto j = b.begin();
        for (; i != a.end() && j != b.end(); ++i, ++j) {
            if (i.row() == j.row() && i.col() == j.col()) {
                counter.count(*i, *j, std::abs(*i - *j));
            }
            else {
                counter.count(*i, *j, std::numeric_limits<double>::infinity());
                log << "\n   " << i.row() << '\t' << i.col() << '\t' << (*i) << "\n !=" << j.row() << '\t' << j.col()
                    << '\t' << (*j) << std::endl;
            }
        }
        ASSERT(i == a.end());
        ASSERT(j == b.end());

        counter.print(log);
        log << std::endl;

        auto check = counter.check();
        if (!check.empty()) {
            throw exception::BadValue(check);
        }
    }
    else {
        size_t d   = 0;
        double eps = 0;
        std::unique_ptr<diff_t> diff(
            param.get("absolute-error", eps) ? static_cast<diff_t*>(new approximate_diff_t(eps)) : new strict_diff_t());

        auto i = a.begin();
        auto j = b.begin();
        for (; i != a.end() && j != b.end(); ++i, ++j) {
            if (i.row() != j.row() || i.col() != j.col() || (*diff)(*i, *j)) {
                log << "\n   " << i.row() << '\t' << i.col() << '\t' << (*i) << "\n !=" << j.row() << '\t' << j.col()
                    << '\t' << (*j) << std::endl;
                ++d;
            }
        }
        ASSERT(i == a.end());
        ASSERT(j == b.end());

        if (d > 0) {
            throw exception::BadValue(std::to_string(d) + " " + Log::Plural("difference")(d));
        }
    }
}


}  // namespace mir::tools


int main(int argc, char** argv) {
    mir::tools::MIRWeightMatrixDiff tool(argc, argv);
    return tool.start();
}
