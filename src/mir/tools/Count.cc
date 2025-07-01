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


#include "mir/tools/Count.h"

#include <cmath>
#include <memory>

#include "eckit/log/JSON.h"

#include "mir/key/grid/Grid.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Increments.h"
#include "mir/util/Log.h"


namespace mir::tools {


template <class T>
std::ostream& operator<<(std::ostream& s, const std::set<std::pair<T, T> >& x) {
    size_t i = 0;
    for (auto e : x) {
        s << ' ' << e.first << " (" << e.second << ")";
        if (++i >= 2) {
            break;
        }
    }
    return s;
}


util::BoundingBox get_bounding_box(const std::vector<double>& area) {
    if (area.empty()) {
        return {};
    }
    ASSERT_KEYWORD_AREA_SIZE(area.size());
    return {area[0], area[1], area[2], area[3]};
}


Count::Count(const util::BoundingBox& bbox) : bbox_(bbox) {}


Count::Count(std::vector<double>& area) : Count(get_bounding_box(area)) {}


void Count::reset() {
    first_      = true;
    count_      = 0;
    countTotal_ = 0;
    n_          = 0;
    s_          = 0;
    e_          = 0;
    w_          = 0;
    nn_.clear();
    ww_.clear();
    ss_.clear();
    ee_.clear();
}


void Count::count(const PointLonLat& point) {
    countTotal_++;

    auto distance = [](double a, double b) {
        return std::abs(PointLonLat::normalise_angle_to_minimum(a - b, -PointLonLat::FLAT_ANGLE));
    };

    nn_.emplace(distance(bbox_.north(), point.lat), point.lat);
    ss_.emplace(distance(bbox_.south(), point.lat), point.lat);
    ee_.emplace(distance(bbox_.east(), point.lon), point.lon);
    ww_.emplace(distance(bbox_.west(), point.lon), point.lon);

    // Log::info() << point.lat << " " << point.lon << " => " << bbox.contains(point.lat, point.lon) << std::endl;

    if (bbox_.contains(point)) {
        double lat = point.lat;
        double lon = PointLonLat::normalise_angle_to_minimum(point.lon, bbox_.west());

        if (first_) {
            n_ = s_ = lat;
            e_ = w_ = lon;
            first_  = false;
        }
        else {
            if (n_ < lat) {
                n_ = lat;
            }
            if (s_ > lat) {
                s_ = lat;
            }
            if (e_ < lon) {
                e_ = lon;
            }
            if (w_ > lon) {
                w_ = lon;
            }
        }

        count_++;
    }
}


void Count::print(std::ostream& out) const {
    out << Log::Pretty(count_) << " out of " << Log::Pretty(countTotal_) << ", north=" << n_ << " (bbox.n - n "
        << bbox_.north() - n_ << ")"
        << ", west=" << w_ << " (w - bbox.w " << w_ - bbox_.west() << ")"
        << ", south=" << s_ << " (s - bbox.s " << s_ - bbox_.south() << ")"
        << ", east=" << e_ << " (bbox.e - e " << bbox_.east() - e_ << ")"
        << "\n"
           "N "
        << bbox_.north() << ":" << nn_
        << "\n"
           "W "
        << bbox_.west() << ":" << ww_
        << "\n"
           "S "
        << bbox_.south() << ":" << ss_
        << "\n"
           "E "
        << bbox_.east() << ":" << ee_ << std::endl;
}


void Count::json(eckit::JSON& j, bool enclose) const {
    if (enclose) {
        j.startObject();
    }

    j << "count" << count_;
    j << "countTotal" << countTotal_;

    j << "point";
    j.startObject();
    j << "n" << n_;
    j << "w" << w_;
    j << "s" << s_;
    j << "e" << e_;
    j.endObject();

    j << "bbox";
    j.startObject();
    j << "n" << bbox_.north();
    j << "w" << bbox_.west();
    j << "s" << bbox_.south();
    j << "e" << bbox_.east();
    j.endObject();

    j << "distance_to_bbox";
    j.startObject();
    j << "n" << (bbox_.north() - n_);
    j << "w" << (w_ - bbox_.west());
    j << "s" << (s_ - bbox_.south());
    j << "e" << (bbox_.east() - e_);
    j.endObject();

    if (!nn_.empty() && !ww_.empty() && !ss_.empty() && !ee_.empty()) {
        j << "distance_to_closest";
        j.startObject();
        j << "n" << (nn_.begin()->first);
        j << "w" << (ww_.begin()->first);
        j << "s" << (ss_.begin()->first);
        j << "e" << (ee_.begin()->first);
        j.endObject();
    }

    if (enclose) {
        j.endObject();
    }
}


void Count::countOnNamedGrid(const std::string& grid) {
    reset();

    repres::RepresentationHandle rep(key::grid::Grid::lookup(grid).representation());
    countOnRepresentation(*rep);
}


void Count::countOnGridIncrements(const std::vector<double>& grid) {
    ASSERT_KEYWORD_GRID_SIZE(grid.size());
    reset();

    util::Increments inc(grid[0], grid[1]);
    repres::RepresentationHandle rep(new repres::latlon::RegularLL(inc));

    countOnRepresentation(*rep);
}


void Count::countOnRepresentation(const repres::Representation& rep) {
    reset();
    for (const std::unique_ptr<repres::Iterator> iter(rep.iterator()); iter->next();) {
        count(iter->pointUnrotated());
    }
}


}  // namespace mir::tools
