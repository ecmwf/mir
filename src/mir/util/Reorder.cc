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


#include "mir/util/Reorder.h"

#include <numeric>
#include <ostream>

#include "eckit/geo/order/HEALPix.h"

#include "mir/util/Exceptions.h"


namespace mir::util {


using HEALPix = eckit::geo::order::HEALPix;


class HEALPixRingToNested final : public Reorder {
public:
    using Reorder::Reorder;

    static const std::string& name() {
        static const std::string NAME("healpix-ring-to-nested");
        return NAME;
    }

    void print(std::ostream& s) const override { s << "HEALPixRingToNested[]"; }
    void json(eckit::JSON& j) const override { j << name(); }

    std::vector<size_t> reorder() override { return HEALPix(HEALPix::ring, size).reorder(HEALPix::nested); }
};


class HEALPixNestedToRing final : public Reorder {
public:
    using Reorder::Reorder;

    static const std::string& name() {
        static const std::string NAME("healpix-ring-to-nested");
        return NAME;
    }

    void print(std::ostream& s) const override { s << "HEALPixNestedToRing[]"; }
    void json(eckit::JSON& j) const override { j << name(); }

    std::vector<size_t> reorder() override { return HEALPix(HEALPix::nested, size).reorder(HEALPix::ring); }
};


class Identity final : public Reorder {
public:
    using Reorder::Reorder;

    static const std::string& name() {
        static const std::string NAME("identity");
        return NAME;
    }

    void print(std::ostream& s) const override { s << "Identity[]"; }
    void json(eckit::JSON& j) const override { j << name(); }

    std::vector<size_t> reorder() override {
        std::vector<size_t> v(size);
        std::iota(v.begin(), v.end(), 0);
        return v;
    }
};


Reorder* Reorder::build(const std::string& name, size_t size) {
    if (name == HEALPixRingToNested::name()) {
        return new HEALPixRingToNested(size);
    }

    if (name == HEALPixNestedToRing::name()) {
        return new HEALPixNestedToRing(size);
    }

    if (name == Identity::name()) {
        return new Identity(size);
    }

    throw exception::BadValue("Reorder: invalid method: '" + name + "'");
}


void Reorder::list(std::ostream& s) {
    s << HEALPixRingToNested::name() << ", " << HEALPixNestedToRing::name() << ", " << Identity::name();
}


}  // namespace mir::util
