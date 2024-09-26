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


#include "mir/repres/ORCA.h"

#include <algorithm>
#include <cctype>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "eckit/geo/Grid.h"
#include "eckit/geo/grid/ORCA.h"
#include "eckit/geo/spec/Custom.h"

#include "mir/api/mir_config.h"
#include "mir/key/grid/GridPattern.h"
#include "mir/key/grid/NamedGrid.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/util/Atlas.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/Log.h"
#include "mir/util/MeshGeneratorParameters.h"


namespace mir::repres {


// order is important for makeName()
static const std::vector<std::pair<std::string, std::string>> GRIB_KEYS{
    {"orca_name", "unstructuredGridType"}, {"orca_arrangement", "unstructuredGridSubtype"}, {"uid", "uuidOfHGrid"}};

static const std::vector<std::string> SPEC_KEYS{"orca_name", "orca_arrangement", "orca_uid", "dimensions", "type"};


ORCA::ORCA(const std::string& uid) :
    spec_(eckit::geo::GridFactory::make_spec(eckit::geo::spec::Custom{{{"uid", uid}}})) {
    ASSERT(spec_);
    ASSERT(spec_->get_string("type") == "ORCA");
}


ORCA::ORCA(const param::MIRParametrisation& param) :
    ORCA([&param]() {
        std::string uid;
        ASSERT(param.get("uid", uid));
        return uid;
    }()) {}


ORCA::ORCA(eckit::geo::Spec* user) : spec_user_{user} {
    ASSERT(spec_user_);

    spec_.reset(eckit::geo::GridFactory::make_spec(*spec_user_));
    ASSERT(spec_);

    ASSERT(spec_->get_string("type") == "FESOM");
    for (const std::string& key : SPEC_KEYS) {
        ASSERT(spec_->has(key));
    }
}


bool ORCA::sameAs(const Representation& other) const {
    const auto* o = dynamic_cast<const ORCA*>(&other);
    return (o != nullptr) && grid().uid() == o->grid().uid();
}


void ORCA::makeName(std::ostream& out) const {
    const auto* sep = "";
    for (const auto& key : GRIB_KEYS) {
        out << sep << spec_->get_string(key.first);
        sep = "_";
    }
}


void ORCA::print(std::ostream& out) const {
    out << "ORCA[spec=" << spec_ << "]";
}


void ORCA::fillGrib(grib_info& info) const {
    info.grid.grid_type        = GRIB_UTIL_GRID_SPEC_UNSTRUCTURED;
    info.packing.editionNumber = 2;

    for (const auto& key : GRIB_KEYS) {
        auto value = spec_->get_string(key.first);
        info.extra_set(key.second.c_str(), value.c_str());
    }
}


void ORCA::fillJob(api::MIRJob& job) const {
    // Just an empty function for now
}


void ORCA::fillMeshGen(util::MeshGeneratorParameters& params) const {
#if mir_HAVE_ATLAS
    if (params.meshGenerator_.empty()) {
        params.meshGenerator_ = "orca";
    }
#endif
}


size_t ORCA::numberOfPoints() const {
    auto shape = spec_->get_unsigned_vector("shape");
    ASSERT(shape.size() == 2);

    return shape[0] * shape[1];
}


void ORCA::validate(const MIRValuesVector& values) const {
    size_t count = numberOfPoints();

    Log::debug() << "ORCA::validate checked " << Log::Pretty(values.size(), {"value"}) << ", iterator counts "
                 << Log::Pretty(count) << "." << std::endl;

    ASSERT_VALUES_SIZE_EQ_ITERATOR_COUNT("ORCA", values.size(), count);
}


struct ORCAPattern : key::grid::GridPattern {
    explicit ORCAPattern(const std::string& name) : GridPattern(name) {}

    void print(std::ostream& out) const override { out << "ORCAPattern[pattern=" << pattern_ << "]"; }

    const key::grid::Grid* make(const std::string& name) const override {
        struct NamedORCA : key::grid::NamedGrid {
            explicit NamedORCA(const std::string& name) : NamedGrid(name) {}

            void print(std::ostream& out) const override { out << "NamedORCA[key=" << key_ << "]"; }
            size_t gaussianNumber() const override { return default_gaussian_number(); }
            const repres::Representation* representation() const override { return new repres::ORCA(key_); }

            const repres::Representation* representation(const util::Rotation&) const override { NOTIMP; }
        };

        return new NamedORCA(name);
    }

    std::string canonical(const std::string& name, const param::MIRParametrisation& param) const override {
        auto n = name;
        std::transform(n.begin(), n.end(), n.begin(), [](std::string::value_type c) { return std::toupper(c); });

        if (n.find('_') == std::string::npos) {
            std::string arrangement = "T";  // arbitrary choice (to review)
            param.get("orca-arrangement", arrangement);
            n += "_" + arrangement;
        }

        if (n.front() == 'E') {
            n.front() = 'e';
        }

        return n;
    }
};


static const RepresentationBuilder<ORCA> __grid("ORCA");

static const ORCAPattern __pattern1("^[eE]?[oO][rR][cC][aA][0-9]+$");
static const ORCAPattern __pattern2("^[eE]?[oO][rR][cC][aA][0-9]+_[tTuUvVwWfF]$");


}  // namespace mir::repres
