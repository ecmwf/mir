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


#include "mir/method/ProxyMethod.h"

#include <algorithm>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "eckit/utils/MD5.h"

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/method/nonlinear/NonLinear.h"
#include "mir/method/solver/Statistics.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/stats/Field.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"
#include "mir/util/Trace.h"


namespace mir {
namespace method {


struct StructuredBicubic final : public ProxyMethod {
    explicit StructuredBicubic(const param::MIRParametrisation& param) : ProxyMethod(param, "structured-bicubic") {}
    void validateMatrix(WeightMatrix& W) const override { W.validateMatrixEntryBounds(false); }
};


struct StructuredBilinear final : public ProxyMethod {
    explicit StructuredBilinear(const param::MIRParametrisation& param) : ProxyMethod(param, "structured-bilinear") {}
    void validateMatrix(WeightMatrix& W) const override { W.validateMatrixEntryBounds(false); }
};


struct StructuredBiquasicubic final : public ProxyMethod {
    explicit StructuredBiquasicubic(const param::MIRParametrisation& param) :
        ProxyMethod(param, "structured-biquasicubic") {}
    void validateMatrix(WeightMatrix& W) const override { W.validateMatrixEntryBounds(false); }
};


struct GridBoxAverage final : public ProxyMethod {
    explicit GridBoxAverage(const param::MIRParametrisation& param) : ProxyMethod(param, "grid-box-average") {}
    void validateMatrix(WeightMatrix& W) const override {
        // W.validateMatrixEntryBounds(false);
        W.validateMatrixRowSum(false);
    }
};


struct GridBoxStatistics final : public ProxyMethod {
    explicit GridBoxStatistics(const param::MIRParametrisation& param) : ProxyMethod(param, "grid-box-average") {
        std::string stats = "maximum";
        param.get("interpolation-statistics", stats);

        // statistics methods are matrix-based
        setSolver(new solver::Statistics(param, stats::FieldFactory::build(stats, param)));
        options().set("matrix_free", false);
    }
    void validateMatrix(WeightMatrix& W) const override {
        // W.validateMatrixEntryBounds(false);
        W.validateMatrixRowSum(false);
    }
};


static MethodBuilder<StructuredBicubic> __method1("structured-bicubic");
static MethodBuilder<StructuredBilinear> __method2("structured-bilinear");
static MethodBuilder<StructuredBiquasicubic> __method3("structured-biquasicubic");
static MethodBuilder<GridBoxAverage> __method4("grid-box-average");
static MethodBuilder<GridBoxStatistics> __method5("grid-box-statistics");


static eckit::Hash::digest_t atlasOptionsDigest(const ProxyMethod::atlas_config_t& options) {
    eckit::MD5 h;
    options.hash(h);
    return h.digest();
}


ProxyMethod::ProxyMethod(const param::MIRParametrisation& param, std::string type) :
    MethodWeighted(param), type_(std::move(type)) {
    options_.set("type", type_);

    bool matrixFree = false;
    param.get("interpolation-matrix-free", matrixFree);
    options_.set("matrix_free", matrixFree);

    nonLinear_ = "missing-if-heaviest-missing";
    parametrisation_.get("non-linear", nonLinear_);

    bool vod2uv = false;
    bool uv2uv  = false;
    param.get("vod2uv", vod2uv);
    param.get("uv2uv", uv2uv);
    vectorField_ = vod2uv || uv2uv;
}


ProxyMethod::~ProxyMethod() = default;


void ProxyMethod::hash(eckit::MD5& h) const {
    h.add(options_);
    h.add(vectorField_);
    MethodWeighted::hash(h);
}


int ProxyMethod::version() const {
    return 1;
}


void ProxyMethod::execute(context::Context& ctx, const repres::Representation& in,
                          const repres::Representation& out) const {
    if (!options_.getBool("matrix_free")) {
        MethodWeighted::execute(ctx, in, out);
        return;
    }

    auto& log = Log::debug();

    struct Helper {
        Helper(size_t numberOfPoints, atlas::FunctionSpace fspace) : n(numberOfPoints), fs(fspace) {}

        atlas::Field appendFieldCopy(const MIRValuesVector& values, bool vector) {
            ASSERT(n == values.size());
            auto f = vector ? fs.createField<double>(atlas::option::name("vector")) : fs.createField<double>();

            auto view = atlas::array::make_view<double, 1>(fields.add(f));
            ASSERT(view.contiguous());
            ASSERT(values.size() <= size_t(view.size()));
            std::copy_n(values.begin(), n, view.data());

            return f;
        }

        void appendFieldWrapped(MIRValuesVector& values) {
            ASSERT(n == values.size());
            auto field = fields.add(atlas::Field("?", values.data(), atlas::array::make_shape(n)));
            field.set_functionspace(fs);
        }

        const size_t n;
        atlas::FunctionSpace fs;
        atlas::FieldSet fields;
    };

    trace::Timer timer("ProxyMethod::execute", log);
    auto report = [](trace::Timer& timer, const std::string& msg) {
        timer.report(msg);
        timer.stop();
        timer.start();
    };


    // set interpolation, considering field with/without missing values (different options)
    auto& field = ctx.field();
    decltype(options_) options(options_);
    if (field.hasMissing()) {
        ASSERT(!nonLinear_.empty());
        options.set("non_linear", nonLinear_);
    }

    atlas::Interpolation interpol(options, in.atlasGrid(), out.atlasGrid());
    Helper input(in.numberOfPoints(), interpol.source());
    Helper output(out.numberOfPoints(), interpol.target());
    report(timer, type_ + ": set interpolation");

    for (size_t i = 0; i < field.dimensions(); ++i) {
        auto f = input.appendFieldCopy(field.values(i), vectorField_);

        if (field.hasMissing()) {
            f.metadata().set("missing_value_type", "equals");
            f.metadata().set("missing_value", field.missingValue());
        }
    }
    report(timer, type_ + ": copy input");


    std::vector<MIRValuesVector> result(field.dimensions(), MIRValuesVector(output.n));
    for (auto& v : result) {
        output.appendFieldWrapped(v);
    }
    report(timer, type_ + ": allocate output");


    interpol.execute(input.fields, output.fields);
    for (size_t i = 0; i < field.dimensions(); ++i) {
        field.update(result[i], i);
    }
    report(timer, type_ + ": interpolate");
}


void ProxyMethod::assemble(util::MIRStatistics& statistics, WeightMatrix& W, const repres::Representation& in,
                           const repres::Representation& out) const {
    auto& log = Log::debug();

    auto timing(statistics.computeMatrixTimer());

    trace::Timer timer("ProxyMethod::execute", log);
    auto report = [](trace::Timer& timer, const std::string& msg) {
        timer.report(msg);
        timer.stop();
        timer.start();
    };

    log << "ProxyMethod::assemble (input: " << in << ", output: " << out << ")" << std::endl;


    // generate matrix (number of columns might include halo)
    atlas::Interpolation interpol(options_, in.atlasGrid(), out.atlasGrid());
    {
        auto M = atlas::interpolation::MatrixCache(interpol).matrix();
        ASSERT(M.rows() == out.numberOfPoints());

        W.swap(M);
        validateMatrix(W);
    }
    report(timer, type_ + ": generate matrix");

    // adjust matrix to remove halo presence (this is done in-place for performance)
    auto& fs       = interpol.source();
    auto cols      = in.numberOfPoints();
    auto cols_halo = size_t(fs.size());

    if (cols == cols_halo) {
        return;
    }

    auto remote = atlas::array::make_view<atlas::idx_t, 1>(fs.remote_index());
    ASSERT(cols_halo == remote.size());
    ASSERT(cols_halo > cols);

    auto inn   = const_cast<WeightMatrix::Index*>(W.inner());
    auto Ncols = WeightMatrix::Index(cols);
    W.cols(Ncols);

    size_t fix = 0;
    for (WeightMatrix::Size i = 0; i < W.nonZeros(); ++i) {
        auto& col = inn[i];
        if (col >= Ncols) {
            col = remote[col];
            ASSERT(col < Ncols);
            ++fix;
        }
    }

    report(timer, type_ + ": adjust matrix to remove halo presence ");
    log << "ProxyMethod: adjusted " << Log::Pretty(fix) << " out of "
        << Log::Pretty(W.nonZeros(), {"matrix entry", "matrix entries"}) << std::endl;
}


bool ProxyMethod::sameAs(const Method& other) const {
    auto o = dynamic_cast<const ProxyMethod*>(&other);
    return (o != nullptr) && atlasOptionsDigest(options_) == atlasOptionsDigest(o->options_);
}


void ProxyMethod::print(std::ostream& out) const {
    out << "ProxyMethod[";
    MethodWeighted::print(out);
    out << "options=" << options_ << "]";
}


}  // namespace method
}  // namespace mir
