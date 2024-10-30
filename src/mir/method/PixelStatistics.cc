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
#include <bitset>
#include <numeric>
#include <typeinfo>
#include <utility>

#include "eckit/log/JSON.h"
#include "eckit/types/Fraction.h"``
#include "eckit/utils/MD5.h"
#include "eckit/utils/StringTools.h"

#include "mir/linalg/spm/OwnedAllocator.h"
#include "mir/method/solver/Statistics.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/reorder/HEALPix.h"
#include "mir/repres/proxy/HEALPix.h"
#include "mir/repres/unsupported/HEALPixNested.h"
#include "mir/stats/Field.h"
#include "mir/util/Exceptions.h"


namespace mir::method {


static const MethodBuilder<PixelStatistics> __builder("pixel-statistics");


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
    using linalg::spm::OwnedAllocator;

    struct healpix_info_t {
        healpix_info_t(const repres::Representation& r, const std::string& which) {
            auto is_power_of_2 = [](size_t n) -> bool { return std::bitset<sizeof(size_t) * 8>(n).count() == 1; };

            std::string msg;

            try {
                if (Nside = dynamic_cast<const repres::proxy::HEALPix&>(r).Nside(); is_power_of_2(Nside)) {
                    nested = false;
                    return;
                }
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

        size_t Nside = 0;
        bool nested  = false;
    };


    // Assemble (nested-to-nested)
    OwnedAllocator::Size Nr = out.numberOfPoints();
    OwnedAllocator::Size Nc = in.numberOfPoints();

    healpix_info_t hin(in, "input");
    healpix_info_t hout(out, "output");

    if (hin.Nside == hout.Nside) {
        // Permute
        std::vector<OwnedAllocator::Index> ia(Nr + 1);
        std::iota(ia.begin(), ia.end(), 0);

        auto ja(ia);
        ja.pop_back();

        WeightMatrix M(new OwnedAllocator{Nr, Nc, ja.size(), std::move(ia), std::move(ja),
                                          std::vector<OwnedAllocator::Scalar>(ja.size(), 1.)});
        W.swap(M);
    }
    else if (hin.Nside < hout.Nside) {
        // Super-sample
        const auto N = (hout.Nside / hin.Nside) * (hout.Nside / hin.Nside);

        std::vector<OwnedAllocator::Index> ia(Nr + 1);
        std::iota(ia.begin(), ia.end(), 0);

        std::vector<OwnedAllocator::Index> ja;
        ja.reserve(Nr * N);
        for (OwnedAllocator::Index i = 0; i < Nc; ++i) {
            ja.insert(ja.end(), N, i);
        }

        WeightMatrix M(new OwnedAllocator{Nr, Nc, ja.size(), std::move(ia), std::move(ja),
                                          std::vector<OwnedAllocator::Scalar>(ja.size(), 1.)});
        W.swap(M);
    }
    else if (hin.Nside > hout.Nside) {
        // Coarsen
        const auto N = (hin.Nside / hout.Nside) * (hin.Nside / hout.Nside);

        std::vector<OwnedAllocator::Index> ia(Nr + 1);
        for (OwnedAllocator::Index i = 0; i <= Nr; ++i) {
            ia[i] = i * N;
        }

        std::vector<OwnedAllocator::Index> ja(Nc);
        std::iota(ja.begin(), ja.end(), 0);

        WeightMatrix M(new OwnedAllocator{Nr, Nc, ja.size(), std::move(ia), std::move(ja),
                                          std::vector<OwnedAllocator::Scalar>(ja.size(), 1. / N)});
        W.swap(M);
    }
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
