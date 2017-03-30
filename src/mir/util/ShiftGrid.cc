/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/util/ShiftGrid.h"
#include "eckit/types/FloatCompare.h"


namespace mir {
namespace util {


ShiftGrid::ShiftGrid(Grid *grid, const Shift& shift):
    grid_(grid),
    shift_(shift) {
}


ShiftGrid::~ShiftGrid() {
}


size_t ShiftGrid::npts() const {
    computePoints();
    return points_.size();
}

void ShiftGrid::lonlat(std::vector<Point>& pts) const {
    computePoints();
    pts = points_;
}

void ShiftGrid::computePoints() const {


    const double eastwards = shift_.west_east();
    const double northwards = shift_.south_north();


    if (points_.empty()) {

        std::vector<Point> original;
        grid_->lonlat(original);

        for (std::vector<Point>::const_iterator j = original.begin(); j != original.end(); ++j) {

            double lon = (*j).lon() + eastwards;
            double lat = (*j).lat() + northwards;

            if (eckit::types::is_strictly_greater(lat, 90.) || eckit::types::is_strictly_greater(-90., lat)) {
                continue;
            }

            while (lon < 0) { lon += 360; }
            while (lon > 360) { lon -= 360; }

            points_.push_back(Grid::Point( lon, lat ));
        }
    }

}


std::string ShiftGrid::gridType() const {
    NOTIMP;
}


eckit::Properties ShiftGrid::spec() const {
    NOTIMP;
}


std::string ShiftGrid::shortName() const {
    if (shortName_.empty()) {
        shortName_ = "shift." + grid_->shortName();
    }
    return shortName_;
}


void ShiftGrid::hash(eckit::MD5& md5) const {
    md5.add("shift.");
    grid_->hash(md5);
    md5.add(shift_.west_east());
    md5.add(shift_.south_north());
}


const atlas::grid::Domain& ShiftGrid::domain() const {
    return grid_->domain();
}


void ShiftGrid::print(std::ostream& out) const {
    out << "ShiftGrid["
        <<  "shift="       << shift_
        << "," << *grid_
        << "]";
}


} // namespace util
} // namespace mir
