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


#pragma once

#include "eckit/geo/Search.h"

#include "mir/util/PointLonLatToPointXYZ.h"
#include "mir/util/Types.h"


namespace mir {
namespace param {
class MIRParametrisation;
}
namespace repres {
class Representation;
}
}  // namespace mir


namespace mir::search {


class PointSearch : public eckit::geo::Search {
public:
    PointSearch(const repres::Representation&, const param::MIRParametrisation&);

    PointValueType closestPoint(const PointXYZ&) const;
    void closestNPoints(const PointXYZ&, size_t n, std::vector<PointValueType>& closest) const;
    void closestWithinRadius(const PointXYZ&, double radius, std::vector<PointValueType>& closest) const;

    PointValueType closestPoint(const PointLonLat&) const;
    void closestNPoints(const PointLonLat&, size_t n, std::vector<PointValueType>& closest) const;
    void closestWithinRadius(const PointLonLat&, double radius, std::vector<PointValueType>& closest) const;

    PointXYZ to_xyz(const PointLonLat& p) const { return to_xyz_.fwd(p); }
    PointLonLat to_lonlat(const PointXYZ& p) const { return to_xyz_.inv(p); }

private:
    util::PointLonLatToPointXYZ to_xyz_;
};


using PointSearchFactory = eckit::geo::search::TreeFactory;


}  // namespace mir::search
