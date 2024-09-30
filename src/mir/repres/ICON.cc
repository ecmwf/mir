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


#include "mir/repres/ICON.h"

#include <algorithm>
#include <cctype>
#include <ostream>
#include <utility>
#include <vector>

#include "eckit/geo/Grid.h"
#include "eckit/geo/grid/ICON.h"
#include "eckit/geo/spec/Custom.h"

#include "mir/api/mir_config.h"
#include "mir/key/grid/GridPattern.h"
#include "mir/key/grid/NamedGrid.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/other/UnstructuredGrid.h"
#include "mir/util/Atlas.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/Log.h"
#include "mir/util/MeshGeneratorParameters.h"


namespace mir::repres {


// order is important for makeName()
static const std::vector<std::pair<std::string, std::string>> GRIB_KEYS{
    // {"icon_arrangement", "unstructuredGridSubtype"},
    {"icon_uid", "uuidOfHGrid"},
};


ICON::ICON(const std::string& uid) : spec_(eckit::geo::SpecByUID::instance().get(uid).spec()) {
    ASSERT(spec_);
    ASSERT(spec_->get_string("type") == "ICON");
    for (const std::string& key : {// "icon_arrangement",
                                   "icon_uid", "shape"}) {
        ASSERT(spec_->has(key));
    }

    grid_ = std::make_unique<eckit::geo::grid::ICON>(*spec_);
    ASSERT(grid_);
}


ICON::ICON(const param::MIRParametrisation& param) :
    ICON([&param]() {
        std::string uid;
        ASSERT(param.get("uid", uid));
        return uid;
    }()) {}


bool ICON::sameAs(const Representation& other) const {
    const auto* o = dynamic_cast<const ICON*>(&other);
    return (o != nullptr) && grid_->uid() == o->grid_->uid();
}


void ICON::makeName(std::ostream& out) const {
    const auto* sep = "";
    for (const auto& key : GRIB_KEYS) {
        out << sep << spec_->get_string(key.first);
        sep = "_";
    }
}


void ICON::print(std::ostream& out) const {
    out << "ICON[spec=" << *spec_ << "]";
}


void ICON::fillGrib(grib_info& info) const {
    info.grid.grid_type        = GRIB_UTIL_GRID_SPEC_UNSTRUCTURED;
    info.packing.editionNumber = 2;

    for (const auto& key : GRIB_KEYS) {
        auto value = spec_->get_string(key.first);
        info.extra_set(key.second.c_str(), value.c_str());
    }
}


void ICON::fillJob(api::MIRJob& job) const {
    // Just an empty function for now
}


void ICON::fillMeshGen(util::MeshGeneratorParameters& params) const {
#if mir_HAVE_ATLAS
    if (params.meshGenerator_.empty()) {
        params.meshGenerator_ = "delaunay";
    }
#endif
}


atlas::Grid ICON::atlasGrid() const {
    std::vector<atlas::PointXY> data;
    data.reserve(numberOfPoints());

    auto container = grid_->container();
    ASSERT(container);

    for (size_t i = 0; i < container->size(); ++i) {
        const auto p  = container->get(i);
        const auto& q = std::get<eckit::geo::PointLonLat>(p);
        data.emplace_back(q.lon, q.lat);  // notice the order
    }

    return atlas::UnstructuredGrid(std::move(data));
}


Iterator* ICON::iterator() const {
    struct ICONIterator : Iterator {
        explicit ICONIterator(const eckit::geo::grid::ICON& grid) : grid_{grid}, n_(grid.size()) {}

        void print(std::ostream& out) const override {
            out << "ICONIterator[";
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

        const eckit::geo::grid::ICON& grid_;
        std::shared_ptr<eckit::geo::Container> container_;
        size_t i_ = 0;
        size_t n_;
    };

    ASSERT(grid_);
    return new ICONIterator(*grid_);
}


size_t ICON::numberOfPoints() const {
    return spec_->get_unsigned("shape");
}


void ICON::validate(const MIRValuesVector& values) const {
    auto count = numberOfPoints();

    Log::debug() << "ICON::validate checked " << Log::Pretty(values.size(), {"value"}) << ", iterator counts "
                 << Log::Pretty(count) << "." << std::endl;

    ASSERT_VALUES_SIZE_EQ_ITERATOR_COUNT("ICON", values.size(), count);
}


struct ICONPattern : key::grid::GridPattern {
    explicit ICONPattern(const std::string& name) : GridPattern(name) {}

    void print(std::ostream& out) const override { out << "ICONPattern[pattern=" << pattern_ << "]"; }

    const key::grid::Grid* make(const std::string& name) const override {
        struct NamedICON : key::grid::NamedGrid {
            explicit NamedICON(const std::string& name) : NamedGrid(name) {}

            void print(std::ostream& out) const override { out << "NamedICON[key=" << key_ << "]"; }
            size_t gaussianNumber() const override { return default_gaussian_number(); }

            const Representation* representation() const override {
                eckit::geo::spec::Custom grid{{{"grid", key_}}};
                std::unique_ptr<eckit::geo::Spec> spec{eckit::geo::GridFactory::make_spec(grid)};

                // key is either a recognized name, or a uid
                return new ICON(spec->has("icon_uid") ? spec->get_string("icon_uid") : key_);
            }

            const Representation* representation(const util::Rotation&) const override { NOTIMP; }
        };

        return new NamedICON(name);
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


const Representation* ICON::croppedRepresentation(const util::BoundingBox& bbox) const {
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


static const RepresentationBuilder<ICON> __grid("ICON");

static const ICONPattern __pattern("^[iI][cC][oO][nN]");  // NOTE: no $ at the end


}  // namespace mir::repres
