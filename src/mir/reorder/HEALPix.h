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


#include "mir/reorder/Reorder.h"


namespace mir::reorder {


class HEALPixReorder : public Reorder {
public:
    explicit HEALPixReorder(size_t N);

    size_t N() const { return Npix_; }
    size_t Nside() const { return Nside_; }

    int nest_to_ring(int n) const;
    int ring_to_nest(int r) const;

private:
    const int Nside_;  // up to 2^13
    const int Npix_;
    const int Ncap_;
    const int k_;
};


class HEALPixRingToNested final : public HEALPixReorder {
public:
    using HEALPixReorder::HEALPixReorder;

    Renumber reorder() const override {
        Renumber map(N());
        for (size_t i = 0; i < N(); ++i) {
            map[i] = static_cast<size_t>(ring_to_nest(static_cast<int>(i)));
        }
        return map;
    }
};


class HEALPixNestedToRing final : public HEALPixReorder {
public:
    using HEALPixReorder::HEALPixReorder;

    Renumber reorder() const override {
        Renumber map(N());
        for (size_t i = 0; i < N(); ++i) {
            map[i] = static_cast<size_t>(nest_to_ring(static_cast<int>(i)));
        }
        return map;
    }
};


}  // namespace mir::reorder
