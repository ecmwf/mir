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


#include "mir/repres/Geo.h"

#include "mir/api/MIRJob.h"
#include "mir/repres/Iterator.h"
#include "mir/util/Exceptions.h"


namespace mir::repres {


Geo::Geo(const Spec& spec) : grid_(eckit::geo::GridFactory::build(spec)) {}


const Geo::Grid& Geo::grid() const {
    ASSERT(grid_);
    return *grid_;
}


void Geo::json(eckit::JSON& j) const {
    grid_->spec().json(j);
}


void Geo::print(std::ostream& out) const {
    out << "Grid[spec=" << grid_->spec_str() << "]";
}


void Geo::fillJob(api::MIRJob& job) const {
    job.set("grid", grid_->spec_str());
}


bool Geo::sameAs(const Representation& other) const {
    const auto* o = dynamic_cast<const Geo*>(&other);
    return (o != nullptr) && grid() == o->grid();
}


void Geo::validate(const MIRValuesVector& values) const {
    ASSERT_VALUES_SIZE_EQ_ITERATOR_COUNT("Geo", values.size(), numberOfPoints());
}


size_t Geo::numberOfPoints() const {
    return grid_->size();
}


Iterator* Geo::iterator() const {
    struct GeoIterator : Iterator {
        explicit GeoIterator(const Grid& grid) : it_(grid.begin()), end_(grid.end()), size_(grid.size()) {}

    private:
        Grid::iterator it_;
        const Grid::iterator end_;
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

    return new GeoIterator(*grid_);
}


}  // namespace mir::repres
