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
#include "eckit/geo/grid/FESOM.h"

#include "eckit/geo/spec/Custom.h"
#include "mir/key/grid/GridPattern.h"
#include "mir/key/grid/NamedGrid.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/other/UnstructuredGrid.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/Log.h"


namespace mir::repres {


// order is important for makeName()
static const std::vector<std::pair<std::string, std::string>> GRIB_KEYS{
    {"fesom_arrangement", "unstructuredGridSubtype"}, {"fesom_uid", "uuidOfHGrid"}};


FESOM::FESOM(const std::string& uid) : spec_(eckit::geo::SpecByUID::instance().get(uid).spec()) {
    ASSERT(spec_);
    ASSERT(spec_->get_string("type") == "FESOM");
    for (const std::string& key : {"fesom_arrangement", "fesom_uid", "shape"}) {
        ASSERT(spec_->has(key));
    }

    grid_ = std::make_unique<eckit::geo::grid::FESOM>(*spec_);
    ASSERT(grid_);
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
    for (const auto& key : GRIB_KEYS) {
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

    for (const auto& key : GRIB_KEYS) {
        auto value = spec_->get_string(key.first);
        info.extra_set(key.second.c_str(), value.c_str());
    }
}


void FESOM::fillJob(api::MIRJob& job) const {
    // Just an empty function for now
}


Iterator* FESOM::iterator() const {
    ASSERT(spec_);

    struct FESOMIterator : Iterator {
        explicit FESOMIterator(const eckit::geo::grid::FESOM& grid) : grid_{grid}, n_(grid.size()) {}

        void print(std::ostream& out) const override {
            out << "FESOMIterator[";
            Iterator::print(out);
            out << ",i=" << i_ << ",n=" << n_ << "]";
        }

        bool next(Latitude& lat, Longitude& lon) override {
            if (!container_) {
                container_ = grid_.container();
                ASSERT(container_);
            }
            else if (++i_ >= n_) {
                return false;
            }

            const auto p  = container_->get(i_);
            const auto& q = std::get<eckit::geo::PointLonLat>(p);

            lat = q.lat;
            lon = q.lon;

            return true;
        }

        size_t index() const override { return i_; }

        const eckit::geo::grid::FESOM& grid_;
        std::shared_ptr<eckit::geo::Container> container_;
        size_t i_ = 0;
        size_t n_;
    };

    ASSERT(grid_);
    return new FESOMIterator(*grid_);
}


size_t FESOM::numberOfPoints() const {
    return spec_->get_unsigned("shape");
}


void FESOM::validate(const MIRValuesVector& values) const {
    auto count = numberOfPoints();

    Log::debug() << "FESOM::validate checked " << Log::Pretty(values.size(), {"value"}) << ", iterator counts "
                 << Log::Pretty(count) << "." << std::endl;

    ASSERT_VALUES_SIZE_EQ_ITERATOR_COUNT("FESOM", values.size(), count);
}


struct FESOMPattern : key::grid::GridPattern {
    explicit FESOMPattern(const std::string& name) : GridPattern(name) {}

    void print(std::ostream& out) const override { out << "FESOMPattern[pattern=" << pattern_ << "]"; }

    const key::grid::Grid* make(const std::string& name) const override {
        struct NamedFESOM : key::grid::NamedGrid {
            explicit NamedFESOM(const std::string& name) : NamedGrid(name) {}

            void print(std::ostream& out) const override { out << "NamedFESOM[key=" << key_ << "]"; }
            size_t gaussianNumber() const override { return default_gaussian_number(); }

            const Representation* representation() const override {
                eckit::geo::spec::Custom grid{{{"grid", key_}}};
                std::unique_ptr<eckit::geo::Spec> spec{eckit::geo::GridFactory::make_spec(grid)};

                // key is either a recognized name, or a uid
                return new FESOM(spec->has("fesom_uid") ? spec->get_string("fesom_uid") : key_);
            }

            const Representation* representation(const util::Rotation&) const override { NOTIMP; }
        };

        return new NamedFESOM(name);
    }

    std::string canonical(const std::string& name, const param::MIRParametrisation&) const override {
        auto n = name;
        std::transform(n.begin(), n.end(), n.begin(), [](auto c) { return std::toupper(c); });

        if (n == "PI") {
            n = "pi";
        }

        return n;
    }
};


const Representation* FESOM::croppedRepresentation(const util::BoundingBox& bbox) const {
    auto container = grid_->container();
    ASSERT(container);

    std::vector<double> lat;
    std::vector<double> lon;

    for (size_t i = 0; i < container->size(); ++i) {
        const auto p = container->get(i);

        if (const auto& q = std::get<eckit::geo::PointLonLat>(p); bbox.contains(Point2{q.lat, q.lon})) {
            lat.push_back(q.lat);
            lon.push_back(q.lon);
        }
    }

    return new other::UnstructuredGrid(lat, lon);
}


static const RepresentationBuilder<FESOM> __grid("FESOM");

static const FESOMPattern __pattern1("^[pP][iI]$");
static const FESOMPattern __pattern2("^[dD][aA][rR][tT]$");


}  // namespace mir::repres
