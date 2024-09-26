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


#include "mir/repres/FESOM.h"

#include <algorithm>
#include <cctype>
#include <ostream>
#include <utility>
#include <vector>

#include "eckit/geo/Grid.h"
#include "eckit/geo/spec/Custom.h"

#include "mir/key/grid/GridPattern.h"
#include "mir/key/grid/NamedGrid.h"
#include "mir/repres/Iterator.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::repres {


// order is important for makeName()
static const std::vector<std::pair<std::string, std::string>> grib_keys{
    {"fesom_arrangement", "unstructuredGridSubtype"}, {"uid", "uuidOfHGrid"}};


FESOM::FESOM(const std::string& uid) :
    spec_(eckit::geo::GridFactory::make_spec(eckit::geo::spec::Custom{{{"uid", uid}}})) {
    ASSERT(spec_);
}


FESOM::FESOM(const param::MIRParametrisation& param) :
    FESOM([&param]() {
        std::string uid;
        ASSERT(param.get("uid", uid));
        return uid;
    }()) {}


bool FESOM::sameAs(const Representation& other) const {
    const auto* o = dynamic_cast<const FESOM*>(&other);
    return (o != nullptr) && grid_->uid() == o->grid_->uid();
}


void FESOM::makeName(std::ostream& out) const {
    const auto* sep = "";
    for (const auto& key : grib_keys) {
        out << sep << spec_->get_string(key.first);
        sep = "_";
    }
}


void FESOM::print(std::ostream& out) const {
    out << "FESOM[spec=" << *spec_ << "]";
}


void FESOM::fillGrib(grib_info& info) const {
    info.grid.grid_type        = GRIB_UTIL_GRID_SPEC_UNSTRUCTURED;
    info.packing.editionNumber = 2;

    for (const auto& key : grib_keys) {
        auto value = spec_->get_string(key.first);
        info.extra_set(key.second.c_str(), value.c_str());
    }
}


void FESOM::fillJob(api::MIRJob& job) const {
    // Just an empty function for now
}


Iterator* FESOM::iterator() const {
    NOTIMP;
}


size_t FESOM::numberOfPoints() const {
    return spec_->get_unsigned("shape");
}


struct FESOMPattern : key::grid::GridPattern {
    explicit FESOMPattern(const std::string& name) : GridPattern(name) {}

    void print(std::ostream& out) const override { out << "FESOMPattern[pattern=" << pattern_ << "]"; }

    const key::grid::Grid* make(const std::string& name) const override {
        struct NamedFESOM : key::grid::NamedGrid {
            explicit NamedFESOM(const std::string& name) : NamedGrid(name) {}

            void print(std::ostream& out) const override { out << "NamedFESOM[key=" << key_ << "]"; }
            size_t gaussianNumber() const override { return default_gaussian_number(); }

            const Representation* representation() const override { return new FESOM(key_); }
            const Representation* representation(const util::Rotation&) const override { NOTIMP; }
        };

        return new NamedFESOM(name);
    }

    std::string canonical(const std::string& name, const param::MIRParametrisation&) const override {
        // "DART" or "pi"

        auto n = name;
        std::transform(n.begin(), n.end(), n.begin(), [](auto c) { return std::toupper(c); });

        if (n == "PI") {
            n = "pi";
        }

        return n;
    }
};


static const RepresentationBuilder<FESOM> __grid("fesom");

static const FESOMPattern __pattern1("^[pP][iI]$");
static const FESOMPattern __pattern2("^[dD][aA][rR][tT]$");


}  // namespace mir::repres
