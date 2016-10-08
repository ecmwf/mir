/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/util/OffsetGrid.h"


namespace mir {
namespace util {


OffsetGrid::OffsetGrid(Grid *grid, double northwards, double eastwards):
    grid_(grid),
    northwards_(northwards),
    eastwards_(eastwards) {
}


OffsetGrid::~OffsetGrid() {
}


size_t OffsetGrid::npts() const {
    computePoints();
    return points_.size();
}

void OffsetGrid::lonlat(std::vector<Point>& pts) const {
    computePoints();
    pts = points_;
}

void OffsetGrid::computePoints() const {

    if (points_.empty()) {

        std::vector<Point> original;
        grid_->lonlat(original);

        for (std::vector<Point>::const_iterator j = original.begin(); j != original.end(); ++j) {
            double lon = (*j).lon() + eastwards_;
            double lat = (*j).lat() + northwards_;

            if (lat > 90 || lat < -90) {
                continue;
            }

            while (lon < 0) { lon += 360; }
            while (lon > 360) { lon -= 360; }

            points_.push_back(Grid::Point( lon, lat ));
        }
    }

}


std::string OffsetGrid::gridType() const {
    NOTIMP;
}


eckit::Properties OffsetGrid::spec() const {
    NOTIMP;
}


std::string OffsetGrid::shortName() const {
    if (shortName_.empty()) {
        shortName_ = "offset." + grid_->shortName();
    }
    return shortName_;
}


void OffsetGrid::hash(eckit::MD5& md5) const {
    md5.add("offset.");
    grid_->hash(md5);
    md5.add(eastwards_);
    md5.add(northwards_);
}


const atlas::grid::Domain& OffsetGrid::domain() const {
    return grid_->domain();
}


void OffsetGrid::print(std::ostream& out) const {
    out << "OffsetGrid["
        <<  "northwards="       << northwards_
        << ",eastwards="      << eastwards_
        << "," << *grid_
        << "]";
}


} // namespace util
} // namespace mir
