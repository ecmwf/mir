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


#include "mir/method/PixelStatistics.h"

#include <algorithm>
#include <numeric>
#include <typeinfo>
#include <utility>

#include "eckit/linalg/Triplet.h"
#include "eckit/log/JSON.h"
#include "eckit/types/Fraction.h"
#include "eckit/utils/MD5.h"
#include "eckit/utils/StringTools.h"

#include "mir/method/solver/Statistics.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/reorder/HEALPix.h"
#include "mir/repres/proxy/HEALPix.h"
#include "mir/repres/unsupported/HEALPixNested.h"
#include "mir/stats/Field.h"
#include "mir/util/Exceptions.h"


namespace mir::method {


static const MethodBuilder<PixelStatistics> __builder("pixel-statistics");


namespace {


struct Biplet : std::pair<size_t, size_t> {
    using pair::pair;
    operator WeightMatrix::Triplet() const { return {first, second, 1. /*non-zero*/}; }
    bool operator<(const Biplet& other) const {
        return first < other.first || (first == other.first && second < other.second);
    }
};


}  // namespace


PixelStatistics::PixelStatistics(const param::MIRParametrisation& param) : MethodWeighted(param) {
    std::string interpolationStatistics_ = "mean";
    param.get("interpolation-statistics", interpolationStatistics_);

    setSolver(new solver::Statistics(param, stats::FieldFactory::build(interpolationStatistics_, param)));

    std::string grid;
    if (param.userParametrisation().get("grid", grid) && eckit::StringTools::endsWith(grid, "_nested")) {
        setReorderRows(new reorder::HEALPixNestedToRing);
    }

    std::string orderingConvention;
    if (param.fieldParametrisation().get("orderingConvention", orderingConvention) && orderingConvention == "nested") {
        setReorderCols(new reorder::HEALPixNestedToRing);
    }
}


void PixelStatistics::json(eckit::JSON& j) const {
    j.startObject();
    j << "interpolation-statistics" << interpolationStatistics_;
    MethodWeighted::json(j);
    j.endObject();
}


const char* PixelStatistics::name() const {
    return "pixel-statistics";
}


void PixelStatistics::assemble(util::MIRStatistics&, WeightMatrix& W, const repres::Representation& in,
                               const repres::Representation& out) const {
    using eckit::linalg::Size;
    using eckit::linalg::Triplet;

    struct healpix_info_t {
        healpix_info_t(const repres::Representation& r, const std::string& which) : Npoints(r.numberOfPoints()) {
            std::string msg;
            try {
                Nside     = dynamic_cast<const repres::proxy::HEALPix&>(r).Nside();
                nested    = false;
                to_nested = reorder::HEALPixRingToNested{}.reorder(Npoints);
                to_ring   = reorder::HEALPixNestedToRing{}.reorder(Npoints);
                return;
            }
            catch (std::bad_cast& e) {
                msg += " (" + std::string{e.what()} + ")";
            }

            try {
                Nside  = dynamic_cast<const repres::unsupported::HEALPixNested&>(r).Nside();
                nested = true;
                return;
            }
            catch (std::bad_cast& e) {
                msg += " (" + std::string{e.what()} + ")";
            }

            throw exception::UserError{"PixelStatistics: " + which + " representation is not HEALPix with Nside=2^k" +
                                       msg};
        }

        size_t native_to_nested(size_t i) { return nested ? i : to_nested.at(i); }
        size_t nested_to_native(size_t i) { return nested ? i : to_ring.at(i); }

        Size Npoints = 0;
        Size Nside   = 0;
        bool nested  = false;
        reorder::Renumber to_nested;
        reorder::Renumber to_ring;
    };


    // Assemble (nested-to-nested)
    healpix_info_t hin(in, "input");
    healpix_info_t hout(out, "output");

    auto Nc = static_cast<Size>(hin.Npoints);
    auto Nr = static_cast<Size>(hout.Npoints);

    std::vector<Triplet> triplets;
    triplets.reserve(std::max(hin.Npoints, hout.Npoints));

    if (hin.Nside == hout.Nside) {
        // Permute
        for (Size i = 0; i < Nr; ++i) {
            triplets.emplace_back(i, i, 1.);
        }

        W.setFromTriplets(triplets);
    }

    ASSERT(false);
}


void PixelStatistics::hash(eckit::MD5& h) const {
    h << "PixelStatistics";
    h << interpolationStatistics_;
    MethodWeighted::hash(h);
}


bool PixelStatistics::sameAs(const Method& other) const {
    const auto* o = dynamic_cast<const PixelStatistics*>(&other);
    return (o != nullptr) && interpolationStatistics_ == o->interpolationStatistics_ && MethodWeighted::sameAs(other);
}


void PixelStatistics::print(std::ostream& out) const {
    out << "PixelStatistics["
        << "InterpolationStatistics=" << interpolationStatistics_ << ",";
    MethodWeighted::print(out);
    out << "]";
}


}  // namespace mir::method
