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


#pragma once

#include "mir/reorder/Reorder.h"


namespace mir::reorder {


struct HEALPix {
    explicit HEALPix(size_t N);

    struct fij_t {
        int f;
        int i;
        int j;
    };

    int size() const;

    fij_t nest_to_fij(int) const;
    fij_t ring_to_fij(int) const;

    int fij_to_nest(const fij_t&) const;
    int fij_to_ring(const fij_t&) const;

    int nest_to_ring(int n) const { return fij_to_ring(nest_to_fij(n)); }
    int ring_to_nest(int r) const { return fij_to_nest(ring_to_fij(r)); }

private:
    const int Nside_;  // up to 2^13
    const int Npix_;
    const int Ncap_;
    const int k_;
};


struct HEALPixRingToNested final : Reorder {
    Renumber reorder(size_t N) const override;

private:
    void print(std::ostream&) const override;
};


struct HEALPixNestedToRing final : Reorder {
    Renumber reorder(size_t N) const override;

private:
    void print(std::ostream&) const override;
};


}  // namespace mir::reorder
