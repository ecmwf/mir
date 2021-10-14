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
#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"
#include "mir/util/Trace.h"


namespace mir {
namespace method {


struct StructuredBicubic final : public ProxyMethod {
    explicit StructuredBicubic(const param::MIRParametrisation& param) : ProxyMethod(param, "structured-bicubic") {}
    const char* name() const /*override*/ { return "structured-bicubic"; }
};


struct StructuredBilinear final : public ProxyMethod {
    explicit StructuredBilinear(const param::MIRParametrisation& param) : ProxyMethod(param, "structured-bilinear") {}
    const char* name() const /*override*/ { return "structured-bilinear"; }
};


struct StructuredBiquasicubic final : public ProxyMethod {
    explicit StructuredBiquasicubic(const param::MIRParametrisation& param) :
        ProxyMethod(param, "structured-biquasicubic") {}
    const char* name() const /*override*/ { return "structured-biquasicubic"; }
};


#if 0
struct GridBoxAverage final : public ProxyMethod {
    explicit GridBoxAverage(const param::MIRParametrisation& param) : ProxyMethod(param, "grid-box-average") {}
    const char* name() const /*override*/ { return "grid-box-average"; }
};
#endif


#if 0
struct GridBoxMaximum final : public ProxyMethod {
    explicit GridBoxMaximum(const param::MIRParametrisation& param) : ProxyMethod(param, "grid-box-maximum") {}
    const char* name() const /*override*/ { return "grid-box-maximum"; }
};
#endif


static MethodBuilder<StructuredBicubic> __method1("structured-bicubic");
static MethodBuilder<StructuredBilinear> __method2("structured-bilinear");
static MethodBuilder<StructuredBiquasicubic> __method3("structured-biquasicubic");
#if 0
static MethodBuilder<GridBoxAverage> __method4("grid-box-average");
static MethodBuilder<GridBoxMaximum> __method5("grid-box-maximum");
#endif


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

    options_.set("matrix_free", LibMir::caching() ? matrixFree : true);
}


ProxyMethod::~ProxyMethod() = default;


void ProxyMethod::hash(eckit::MD5& h) const {
    h.add(options_);
    MethodWeighted::hash(h);
}


int ProxyMethod::version() const {
    return 1;
}


void ProxyMethod::execute(context::Context& ctx, const repres::Representation& in,
                          const repres::Representation& out) const {
    auto& log = Log::debug();

    struct Helper {
        Helper(size_t numberOfPoints, atlas::FunctionSpace fspace) : n(numberOfPoints), fs(fspace) {}

        void appendFieldCopy(const MIRValuesVector& values) {
            ASSERT(n == values.size());
            auto view = atlas::array::make_view<double, 1>(fields.add(fs.createField<double>()));
            ASSERT(view.contiguous());
            ASSERT(values.size() <= size_t(view.size()));
            std::copy_n(values.begin(), n, view.data());
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

    auto& field = ctx.field();

    if (options_.getBool("matrix_free")) {
        atlas::Interpolation interpol(options_, in.atlasGrid(), out.atlasGrid());
        Helper input(in.numberOfPoints(), interpol.source());
        Helper output(out.numberOfPoints(), interpol.target());
        report(timer, type_ + ": set interpolation");


        for (size_t i = 0; i < field.dimensions(); ++i) {
            input.appendFieldCopy(field.values(i));
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
        return;
    }

    MethodWeighted::execute(ctx, in, out);
}


void ProxyMethod::assemble(util::MIRStatistics& statistics, WeightMatrix& W, const repres::Representation& in,
                           const repres::Representation& out) const {
    auto timing(statistics.computeMatrixTimer());

    auto& log = Log::debug();
    log << "ProxyMethod::assemble (input: " << in << ", output: " << out << ")" << std::endl;

    atlas::Interpolation interpol(options_, in.atlasGrid(), out.atlasGrid());
    auto M = atlas::interpolation::MatrixCache(interpol).matrix();

    // fix

    W.swap(M);
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
