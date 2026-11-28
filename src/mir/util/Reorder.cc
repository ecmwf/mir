// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/util/Reorder.h"

#include <cmath>
#include <map>
#include <memory>
#include <numeric>
#include <ostream>

#include "eckit/geo/Grid.h"
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
        auto Nside = static_cast<size_t>(std::sqrt(size / 12));
        ASSERT(12 * Nside * Nside == size);

        using H = eckit::geo::order::HEALPix;
        return reorder_healpix(Nside, H::RING, H::NESTED);
    }
};


class HEALPixNestedToRing final : public Reorder {
public:
    using Reorder::Reorder;

    void print(std::ostream& s) const override { s << "HEALPixNestedToRing[]"; }

    std::vector<size_t> reorder() override {
        auto Nside = static_cast<size_t>(std::sqrt(size / 12));
        ASSERT(12 * Nside * Nside == size);

        using H = eckit::geo::order::HEALPix;
        return reorder_healpix(Nside, H::NESTED, H::RING);
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


std::vector<size_t> reorder_healpix(size_t Nside, const std::string& from, const std::string& to) {
    // the grid's renumbering (its own order to another) is common to eckit::geo versions
    std::unique_ptr<const eckit::geo::Grid> grid(
        eckit::geo::GridFactory::make_from_string("{grid: H" + std::to_string(Nside) + ", order: " + from + "}"));
    return grid->reorder(to);
}


}  // namespace mir::util
