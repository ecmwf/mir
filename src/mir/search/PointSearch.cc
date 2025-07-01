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


#include "mir/search/PointSearch.h"

#include "eckit/geo/spec/Custom.h"

#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"


namespace mir::search {


namespace {


double pole_displacement(const param::MIRParametrisation& param) {
    double displacement = 0;
    param.get("pole-displacement-in-degree", displacement);
    return displacement;
}


eckit::geo::spec::Custom spec_from_parametrisation(const param::MIRParametrisation& param) {
    auto loader = LibMir::cacheLoader(LibMir::cache_loader::POINT_SEARCH);
    param.get("search-trees", loader);

    auto caching = LibMir::caching();
    param.get("caching", caching);

    return {{"eckit-geo-search-trees", loader}, {"caching", caching}};
}


}  // namespace


PointSearch::PointSearch(const repres::Representation& r, const param::MIRParametrisation& param) :
    eckit::geo::Search(r.grid(), spec_from_parametrisation(param)), to_xyz_(r, pole_displacement(param)) {}


void PointSearch::closestNPoints(const PointXYZ& p, size_t n, std::vector<PointValueType>& closest) const {
    Search::closestNPoints(PointType(p), n, closest);
}


void PointSearch::closestWithinRadius(const PointXYZ& p, double radius, std::vector<PointValueType>& closest) const {
    Search::closestWithinRadius(PointType(p), radius, closest);
}


eckit::geo::Search::PointValueType PointSearch::closestPoint(const PointXYZ& p) const {
    return Search::closestPoint(PointType(p));
}


void PointSearch::closestNPoints(const PointLonLat& p, size_t n, std::vector<PointValueType>& closest) const {
    Search::closestNPoints(PointType(to_xyz_.fwd(p)), n, closest);
}


void PointSearch::closestWithinRadius(const PointLonLat& p, double radius, std::vector<PointValueType>& closest) const {
    Search::closestWithinRadius(PointType(to_xyz_.fwd(p)), radius, closest);
}


eckit::geo::Search::PointValueType PointSearch::closestPoint(const PointLonLat& p) const {
    return Search::closestPoint(PointType(to_xyz_.fwd(p)));
}


}  // namespace mir::search
