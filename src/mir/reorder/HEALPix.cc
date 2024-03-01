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
#include <string>

#include "mir/reorder/Reorder.h"
#include "mir/repres/proxy/HEALPix.h"
#include "mir/util/Exceptions.h"


namespace mir::reorder {


struct HEALPix : Reorder {
    explicit HEALPix(size_t N) :
        N_(N),
        Nside_([N] {
            auto Nside = static_cast<size_t>(std::sqrt(static_cast<double>(N) / 12.));
            ASSERT_MSG(12 * Nside * Nside == N, "Expected N = 12 * Nside ** 2, got N=" + std::to_string(N));
            return Nside;
        }()),
        healpix_(static_cast<int>(Nside_)) {}

    size_t N() const { return N_; }
    size_t Nside() const { return Nside_; }
    const repres::proxy::HEALPix::Reorder& healpix() const { return healpix_; }

private:
    const size_t N_;
    const size_t Nside_;
    const repres::proxy::HEALPix::Reorder healpix_;
};


struct HEALPixRingToNested final : HEALPix {
    using HEALPix::HEALPix;

    Renumber reorder() const override {
        Renumber map(N());
        for (size_t i = 0; i < N(); ++i) {
            map[i] = static_cast<size_t>(healpix().ring_to_nest(static_cast<int>(i)));
        }
        return map;
    }
};


struct HEALPixNestedToRing final : HEALPix {
    using HEALPix::HEALPix;

    Renumber reorder() const override {
        Renumber map(N());
        for (size_t i = 0; i < N(); ++i) {
            map[i] = static_cast<size_t>(healpix().nest_to_ring(static_cast<int>(i)));
        }
        return map;
    }
};


static const ReorderBuilder<HEALPixRingToNested> __reorder1("healpix-ring-to-nested");
static const ReorderBuilder<HEALPixNestedToRing> __reorder2("healpix-nested-to-ring");


}  // namespace mir::reorder
