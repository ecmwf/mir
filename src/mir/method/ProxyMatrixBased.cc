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


#include "mir/method/ProxyMatrixBased.h"

#include "eckit/utils/MD5.h"

#include "atlas/interpolation.h"

#include "mir/method/solver/Statistics.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/stats/Field.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Trace.h"


namespace mir {
namespace method {


struct StructuredBicubic final : public ProxyMatrixBased {
    explicit StructuredBicubic(const param::MIRParametrisation& param) :
        ProxyMatrixBased(param, "structured-bicubic") {}
};


struct StructuredBilinear final : public ProxyMatrixBased {
    explicit StructuredBilinear(const param::MIRParametrisation& param) :
        ProxyMatrixBased(param, "structured-bilinear") {}
};


struct StructuredBiquasicubic final : public ProxyMatrixBased {
    explicit StructuredBiquasicubic(const param::MIRParametrisation& param) :
        ProxyMatrixBased(param, "structured-biquasicubic") {}
};


struct GridBoxAverage final : public ProxyMatrixBased {
    explicit GridBoxAverage(const param::MIRParametrisation& param) : ProxyMatrixBased(param, "grid-box-average") {}
};


struct GridBoxStatistics final : public ProxyMatrixBased {
    explicit GridBoxStatistics(const param::MIRParametrisation& param) : ProxyMatrixBased(param, "grid-box-average") {
        std::string stats = "maximum";
        param.get("interpolation-statistics", stats);

        setSolver(new solver::Statistics(param, stats::FieldFactory::build(stats, param)));
    }
};


template <int ORDER>
struct ConservativeOrder final : public ProxyMatrixBased {
    explicit ConservativeOrder(const param::MIRParametrisation& param) : ProxyMatrixBased(param, "conservative") {
        options().set("order", ORDER);
        options().set("src_cell_data", false);
        options().set("tgt_cell_data", false);
    }
};


static const MethodBuilder<StructuredBicubic> __method1("structured-bicubic");
static const MethodBuilder<StructuredBilinear> __method2("structured-bilinear");
static const MethodBuilder<StructuredBiquasicubic> __method3("structured-biquasicubic");
static const MethodBuilder<GridBoxAverage> __method4("grid-box-average");
static const MethodBuilder<GridBoxStatistics> __method5("grid-box-statistics");
static const MethodBuilder<ConservativeOrder<1>> __method6("conservative-order-1");
static const MethodBuilder<ConservativeOrder<2>> __method7("conservative-order-2");


static eckit::Hash::digest_t atlasOptionsDigest(const ProxyMatrixBased::atlas_config_t& options) {
    eckit::MD5 h;
    options.hash(h);
    return h.digest();
}


ProxyMatrixBased::ProxyMatrixBased(const param::MIRParametrisation& param, const std::string& type) :
    MethodWeighted(param), name_(type) {
    options_.set("type", type);
    options_.set("matrix_free", false);
    options_.set("matrix_global", true);
}


void ProxyMatrixBased::hash(eckit::MD5& md5) const {
    MethodWeighted::hash(md5);
    md5.add(options_);
}


int ProxyMatrixBased::version() const {
    return 0;
}


void ProxyMatrixBased::assemble(util::MIRStatistics&, MethodWeighted::WeightMatrix& W, const repres::Representation& in,
                                const repres::Representation& out) const {
    trace::Timer timer("ProxyMatrixBased::assemble");

    atlas::Interpolation interpol(options_, in.atlasGrid(), out.atlasGrid());

    atlas::interpolation::MatrixCache mc(interpol);
    W.swap(const_cast<eckit::linalg::SparseMatrix&>(mc.matrix()));  // removing constness is necessary for the swap
}


bool ProxyMatrixBased::sameAs(const Method& other) const {
    const auto* o = dynamic_cast<const ProxyMatrixBased*>(&other);
    return (o != nullptr) && atlasOptionsDigest(options_) == atlasOptionsDigest(o->options_) &&
           MethodWeighted::sameAs(other);
}


void ProxyMatrixBased::print(std::ostream& out) const {
    out << "ProxyMatrixBased[options=" << options_ << ",";
    MethodWeighted::print(out);
    out << "]";
}


}  // namespace method
}  // namespace mir
