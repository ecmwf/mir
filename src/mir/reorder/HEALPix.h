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


class HEALPixReorder : public Reorder {
public:
    explicit HEALPixReorder(size_t N);

protected:
    size_t N() const { return Npix_; }
    size_t Nside() const { return Nside_; }

    int nest_to_ring(int) const;
    int ring_to_nest(int) const;

private:
    const int Nside_;  // up to 2^13
    const int Npix_;
    const int Ncap_;
    const int k_;
};


struct HEALPixRingToNested final : HEALPixReorder {
    using HEALPixReorder::HEALPixReorder;
    Renumber reorder() const override;
};


struct HEALPixNestedToRing final : HEALPixReorder {
    using HEALPixReorder::HEALPixReorder;
    Renumber reorder() const override;
};


}  // namespace mir::reorder
