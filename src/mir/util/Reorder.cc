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

#include <map>
#include <numeric>
#include <ostream>

#include "eckit/geo/order/HEALPix.h"
#include "eckit/log/JSON.h"

#include "mir/util/Exceptions.h"
#include "mir/util/Mutex.h"


namespace mir::util {


static std::map<std::string, Reorder::Builder*> BUILDERS;
static recursive_mutex MUTEX;


class lock_type {
    lock_guard<recursive_mutex> lock_guard_{MUTEX};
};


void Reorder::Builder::register_builder(const std::string& name, Reorder::Builder* builder) {
    lock_type lock;
    ASSERT(BUILDERS.insert({name, builder}).second);
}


class HEALPixRingToNested final : public Reorder {
public:
    using Reorder::Reorder;

    void print(std::ostream& s) const override { s << "HEALPixRingToNested[]"; }

    std::vector<size_t> reorder() override {
        using H = eckit::geo::order::HEALPix;
        return H(H::ring, size).reorder(H::nested);
    }
};


class HEALPixNestedToRing final : public Reorder {
public:
    using Reorder::Reorder;

    void print(std::ostream& s) const override { s << "HEALPixNestedToRing[]"; }

    std::vector<size_t> reorder() override {
        using H = eckit::geo::order::HEALPix;
        return H(H::nested, size).reorder(H::ring);
    }
};


class Identity final : public Reorder {
public:
    using Reorder::Reorder;

    void print(std::ostream& s) const override { s << "Identity[]"; }

    std::vector<size_t> reorder() override {
        std::vector<size_t> v(size);
        std::iota(v.begin(), v.end(), 0);
        return v;
    }
};


static const Reorder::BuilderT<HEALPixRingToNested> REORDER1("healpix-ring-to-nested");
static const Reorder::BuilderT<HEALPixNestedToRing> REORDER2("healpix-nested-to-ring");
static const Reorder::BuilderT<Identity> REORDER3("identity");


Reorder* Reorder::build(const std::string& name, size_t size) {
    lock_type lock;

    if (auto builder = BUILDERS.find(name); builder != BUILDERS.end()) {
        ASSERT(builder->second != nullptr);
        return builder->second->build(name, size);
    }

    throw exception::BadValue("Reorder: invalid method: '" + name + "'");
}


void Reorder::list(std::ostream& s) {
    lock_type lock;

    const auto* sep = "";
    for (const auto& [key, value] : BUILDERS) {
        s << sep << key;
        sep = ", ";
    }
}


void Reorder::json(eckit::JSON& j) const {
    j << name;
}


}  // namespace mir::util
