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
#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/geo/Grid.h"
#include "eckit/geo/grid/ORCA.h"
#include "eckit/geo/spec/Custom.h"

#include "mir/api/mir_config.h"
#include "mir/key/grid/GridPattern.h"
#include "mir/key/grid/NamedGrid.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/other/UnstructuredGrid.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/Log.h"
#include "mir/util/MeshGeneratorParameters.h"


namespace mir::repres {


struct key_t {
    std::string geo;
    std::string atlas;
    std::string grib;
};


enum key_index_t
{
    GEO,
    ATLAS,
    GRIB
};

// order is important for makeName()
static const std::vector<key_t> KEYS{{"orca_name", "orca_name", "unstructuredGridType"},
                                     {"orca_arrangement", "orca_arrangement", "unstructuredGridSubtype"},
                                     {"orca_uid", "uid", "uuidOfHGrid"}};


ORCA::ORCA(const std::string& uid) : spec_(eckit::geo::SpecByUID::instance().get(uid).spec()) {
    ASSERT(spec_);
    ASSERT(spec_->get_string("type") == "ORCA");
    for (const std::string& key : {"orca_name", "orca_arrangement", "orca_uid", "dimensions"}) {
        ASSERT(spec_->has(key));
    }

    grid_ = std::make_unique<eckit::geo::grid::ORCA>(*spec_);
    ASSERT(grid_);
}


ORCA::ORCA(const param::MIRParametrisation& param) :
    ORCA([&param]() {
        std::string uid;
        ASSERT(param.get("uid", uid));
        return uid;
    }()) {}


bool ORCA::sameAs(const Representation& other) const {
    const auto* o = dynamic_cast<const ORCA*>(&other);
    return (o != nullptr) && grid_->uid() == o->grid_->uid();
}


void ORCA::makeName(std::ostream& out) const {
    const auto* sep = "";
    for (const auto& key : KEYS) {
        out << sep << spec_->get_string(key.geo);
        sep = "_";
    }
}


void ORCA::print(std::ostream& out) const {
    out << "ORCA[spec=" << spec_ << "]";
}


void ORCA::fillGrib(grib_info& info) const {
    info.grid.grid_type        = GRIB_UTIL_GRID_SPEC_UNSTRUCTURED;
    info.packing.editionNumber = 2;

    for (const auto& key : KEYS) {
        auto value = spec_->get_string(key.geo);
        info.extra_set(key.grib.c_str(), value.c_str());
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


Iterator* ORCA::iterator() const {
    struct ORCAIterator : Iterator {
        explicit ORCAIterator(const eckit::geo::grid::ORCA& grid) : grid_{grid}, n_(grid.size()) {}

        void print(std::ostream& out) const override {
            out << "ORCAIterator[";
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

        const eckit::geo::grid::ORCA& grid_;
        std::shared_ptr<eckit::geo::Container> container_;
        size_t i_ = 0;
        size_t n_;
    };

    ASSERT(grid_);
    return new ORCAIterator(*grid_);
}


size_t ORCA::numberOfPoints() const {
    auto dimensions = spec_->get_unsigned_vector("dimensions");
    ASSERT(dimensions.size() == 2);

    return dimensions[0] * dimensions[1];
}


void ORCA::validate(const MIRValuesVector& values) const {
    auto count = numberOfPoints();

    Log::debug() << "ORCA::validate checked " << Log::Pretty(values.size(), {"value"}) << ", iterator counts "
                 << Log::Pretty(count) << "." << std::endl;

    ASSERT_VALUES_SIZE_EQ_ITERATOR_COUNT("ORCA", values.size(), count);
}


atlas::Grid ORCA::atlasGrid() const {
    eckit::LocalConfiguration config;
    for (const auto& key : KEYS) {
        config.set(key.atlas, spec_->get_string(key.geo));
    }

    return ::atlas::Grid(config);
}


struct ORCAPattern : key::grid::GridPattern {
    explicit ORCAPattern(const std::string& name) : GridPattern(name) {}

    void print(std::ostream& out) const override { out << "ORCAPattern[pattern=" << pattern_ << "]"; }

    const key::grid::Grid* make(const std::string& name) const override {
        struct NamedORCA : key::grid::NamedGrid {
            explicit NamedORCA(const std::string& name) : NamedGrid(name) {}

            void print(std::ostream& out) const override { out << "NamedORCA[key=" << key_ << "]"; }
            size_t gaussianNumber() const override { return default_gaussian_number(); }

            const Representation* representation() const override {
                eckit::geo::spec::Custom grid{{{"grid", key_}}};
                std::unique_ptr<eckit::geo::Spec> spec{eckit::geo::GridFactory::make_spec(grid)};

                // key is either a recognized name, or a uid
                return new ORCA(spec->has("orca_uid") ? spec->get_string("orca_uid") : key_);
            }

            const Representation* representation(const util::Rotation&) const override { NOTIMP; }
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


const Representation* ORCA::croppedRepresentation(const util::BoundingBox& bbox) const {
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


static const RepresentationBuilder<ORCA> __grid("ORCA");

static const ORCAPattern __pattern1("^[eE]?[oO][rR][cC][aA][0-9]+$");
static const ORCAPattern __pattern2("^[eE]?[oO][rR][cC][aA][0-9]+_[tTuUvVwWfF]$");


}  // namespace mir::repres
