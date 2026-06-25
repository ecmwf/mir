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


#include "mir/method/HEALPixMethod.h"

#include <sstream>

#include "eckit/geo/order/HEALPix.h"
#include "eckit/linalg/allocator/StandardContainerAllocator.h"
#include "eckit/log/JSON.h"
#include "eckit/utils/MD5.h"

#include "mir/method/solver/Statistics.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/HEALPix.h"
#include "mir/stats/Field.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Trace.h"


namespace mir::method {


void HEALPixMethod::hash(eckit::MD5& md5) const {
    MethodWeighted::hash(md5);
    std::ostringstream str;
    print(str);
    md5.add(str.str());
}


void HEALPixMethod::assemble(util::MIRStatistics& /*ignored*/, WeightMatrix& W, const repres::Representation& in,
                             const repres::Representation& out) const {
    trace::Timer time("assemble: fill sparse matrix");


    auto nested_reorder = [](const repres::Representation& r) {
        if (auto const* h = dynamic_cast<repres::HEALPix const*>(&r); h != nullptr) {
            using Order = eckit::geo::order::HEALPix;
            return Order(Order::NESTED).reorder(h->order(), h->Nside());
        }

        throw exception::SeriousBug("HEALPixMethod: representation must be HEALPix");
    };

    const auto rows = nested_reorder(out);
    const auto Nr   = rows.size();
    ASSERT(Nr == out.numberOfPoints());

    const auto cols = nested_reorder(in);
    const auto Nc   = cols.size();
    ASSERT(Nc == in.numberOfPoints());

    using Allocator = eckit::linalg::allocator::StandardContainerAllocator;
    Allocator::container_type mat(Nr);

    if (Nc > Nr) {
        const auto N = Nc / Nr;
        const auto w = 1. / static_cast<WeightMatrix::Scalar>(N);

        for (size_t r = 0, c = 0; r < Nr; ++r) {
            for (size_t n = 0; n < N; ++n, ++c) {
                mat[rows[r]][cols[c]] = w;
            }
        }
    }
    else {
        const auto N = Nr / Nc;
        const auto w = 1.;

        for (size_t r = 0, c = 0; c < Nc; ++c) {
            for (size_t n = 0; n < N; ++n, ++r) {
                mat[rows[r]][cols[c]] = w;
            }
        }
    }

    WeightMatrix M(new Allocator(Nr, Nc, mat));
    M.swap(W);
}


bool HEALPixMethod::sameAs(const Method& other) const {
    const auto* o = dynamic_cast<const HEALPixMethod*>(&other);
    return (o != nullptr) && type() == o->type() && MethodWeighted::sameAs(*o);
}


WeightMatrix::Check HEALPixMethod::validateMatrixWeights() const {
    return {false, false, false};
}


int HEALPixMethod::version() const {
    return 0;
}


namespace healpix {


struct HEALPixResample final : HEALPixMethod {
    using HEALPixMethod::HEALPixMethod;

    void print(std::ostream& out) const override {
        out << "HEALPixResample[";
        HEALPixMethod::print(out);
        out << "]";
    }

    const char* type() const override { return "healpix-resample"; }
};


struct HEALPixStatistics final : HEALPixMethod {
    explicit HEALPixStatistics(const param::MIRParametrisation& param) :
        HEALPixMethod(param), interpolationStatistics_("maximum") {
        param.get("interpolation-statistics", interpolationStatistics_);

        setSolver(new solver::Statistics(param, stats::FieldFactory::build(interpolationStatistics_, param)));
    }

    bool sameAs(const Method& other) const override {
        const auto* o = dynamic_cast<const HEALPixStatistics*>(&other);
        return (o != nullptr) && o->interpolationStatistics_ == interpolationStatistics_ &&
               HEALPixMethod::sameAs(other);
    }

    void print(std::ostream& out) const override {
        out << "HEALPixStatistics["
               "interpolationStatistics="
            << interpolationStatistics_ << ",";
        HEALPixMethod::print(out);
        out << "]";
    }

    const char* type() const override { return "healpix-statistics"; }

    void json(eckit::JSON& j) const override {
        HEALPixMethod::json(j);
        j << "interpolation-statistics" << interpolationStatistics_;
    }

private:
    std::string interpolationStatistics_;
};


static const MethodBuilder<HEALPixResample> HEALPIX_RESAMPLE("healpix-resample");
static const MethodBuilder<HEALPixStatistics> HEALPIX_STATISTICS("healpix-statistics");


}  // namespace healpix
}  // namespace mir::method
