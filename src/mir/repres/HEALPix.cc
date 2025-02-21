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


#include "mir/repres/HEALPix.h"

#include <cctype>
#include <memory>
#include <ostream>

#include "eckit/geo/spec/Custom.h"

#include "mir/api/MIRJob.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/util/Atlas.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
// #include "mir/util/GridBox.h"
#include "mir/util/MeshGeneratorParameters.h"


namespace mir::repres {


namespace {


const RepresentationBuilder<HEALPix> __repres("healpix");


std::unique_ptr<eckit::geo::spec::Custom> spec_from_parametrisation(const param::MIRParametrisation& param) {
    long Nside = 0;
    param.get("Nside", Nside);

    std::string ordering;
    param.get("ordering", ordering);

    auto spec = std::make_unique<eckit::geo::spec::Custom>();
    spec->set("type", "HEALPix");
    spec->set("Nside", Nside);
    spec->set("ordering", ordering);

    return spec;
}


}  // namespace


HEALPix::HEALPix(size_t Nside, Ordering ordering) : grid_(Nside, ordering) {}


HEALPix::HEALPix(const param::MIRParametrisation& param) : grid_(*spec_from_parametrisation(param)) {}


void HEALPix::json(eckit::JSON& j) const {
    grid_.Grid::spec().json(j);
}


void HEALPix::print(std::ostream& out) const {
    out << "HEALPix[spec=" << grid_.spec_str() << "]";
}


void HEALPix::makeName(std::ostream& out) const {
    out << "H" << std::to_string(grid_.Nside()) << (grid_.ordering() == Ordering::healpix_nested ? "_nested" : "");
}


void HEALPix::fillGrib(grib_info& info) const {
    info.grid.grid_type = GRIB_UTIL_GRID_SPEC_HEALPIX;
    info.grid.N         = static_cast<long>(grid_.Nside());

    info.grid.longitudeOfFirstGridPointInDegrees = 45.;

    info.extra_set("orderingConvention", grid_.ordering() == Ordering::healpix_nested ? "nested" : "ring");
}


void HEALPix::fillJob(api::MIRJob& job) const {
    job.set("grid", grid_.spec_str());
}


void HEALPix::fillMeshGen(util::MeshGeneratorParameters& params) const {
    if (params.meshGenerator_.empty()) {
        params.meshGenerator_ = "structured";
    }
}


bool mir::repres::HEALPix::sameAs(const Representation& other) const {
    const auto* o = dynamic_cast<const HEALPix*>(&other);
    return (o != nullptr) && grid_ == o->grid_;
}


void HEALPix::validate(const MIRValuesVector& values) const {
    ASSERT_VALUES_SIZE_EQ_ITERATOR_COUNT("HEALPix", values.size(), numberOfPoints());
}


::atlas::Grid HEALPix::atlasGrid() const {
    const auto* ordering = grid_.ordering() == Ordering::healpix_nested ? "nested" : "ring";
    return {::atlas::Grid::Spec("type", "healpix").set("N", grid_.Nside()).set("ordering", ordering)};
}


std::vector<util::GridBox> mir::repres::HEALPix::gridBoxes() const {
    NOTIMP;
}


size_t HEALPix::numberOfPoints() const {
    return grid_.size();
}


Iterator* HEALPix::iterator() const {
    struct GeoIterator : Iterator {
        explicit GeoIterator(const eckit::geo::Grid& grid) : it_(grid.begin()), end_(grid.end()), size_(grid.size()) {}

    private:
        eckit::geo::Grid::iterator it_;
        const eckit::geo::Grid::iterator end_;
        const size_t size_;

        void print(std::ostream& out) const override {
            out << "GeoIterator[";
            Iterator::print(out);
            out << ",count=" << index() << ",size=" << size_ << "]";
        }

        bool next(Latitude& _lat, Longitude& _lon) override {
            if (it_ != end_) {
                const auto p  = *it_;
                const auto& q = std::get<PointLonLat>(p);
                point_[0]     = q.lat;
                point_[1]     = q.lon;
                _lat          = q.lat;
                _lon          = q.lon;

                ++it_;
                return true;
            }

            return false;
        }

        size_t index() const override {
            ASSERT(it_ != end_);
            return it_->index();
        }
    };

    return new GeoIterator(grid_);
}


}  // namespace mir::repres
