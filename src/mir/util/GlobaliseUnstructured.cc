/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include "mir/util/GlobaliseUnstructured.h"

#include <memory>

#include "eckit/exception/Exceptions.h"

#include "mir/namedgrids/NamedGrid.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/repres/other/UnstructuredGrid.h"
#include "mir/search/PointSearch.h"
#include "mir/util/Domain.h"


namespace mir {
namespace util {


GlobaliseUnstructured::GlobaliseUnstructured(const param::MIRParametrisation& parametrisation) :
    parametrisation_(parametrisation) {

    globaliseGridname_ = "O16";
    parametrisation_.get("globalise-gridname", globaliseGridname_);

    globaliseMissingRadius_ = 555975.;  // Earth great-circle distance of approx. 5 degree
    parametrisation_.get("globalise-missing-radius", globaliseMissingRadius_);

    ASSERT(globaliseMissingRadius_ > 0.);
}


GlobaliseUnstructured::~GlobaliseUnstructured() = default;


size_t GlobaliseUnstructured::appendGlobalPoints(
        std::vector<double>& latitudes,
        std::vector<double>& longitudes ) {
    if (globaliseGridname_.empty()) {
        return 0;
    }

    // TODO: cache me


    // setup k-d tree on temporary unstructured grid
    repres::other::UnstructuredGrid unstructuredGrid(latitudes, longitudes);
    const search::PointSearch tree(parametrisation_, unstructuredGrid);


    // setup global grid
    repres::RepresentationHandle globe(namedgrids::NamedGrid::lookup(globaliseGridname_).representation());
    size_t nbExtraPoints = 0;


    // insert global grid points when distant enough from provided grid points
    std::unique_ptr<repres::Iterator> it(globe->iterator());
    while (it->next()) {
        const Point3 p(it->point3D());
        if (Point3::distance(p, tree.closestPoint(p).point()) > globaliseMissingRadius_) {

            const auto& p = it->pointUnrotated();
            latitudes.push_back(p.lat().value());
            longitudes.push_back(p.lon().value());
            ++nbExtraPoints;

        }
    }

    return nbExtraPoints;
}


#if 0
size_t GlobaliseUnstructured::appendGlobalPoints(
        std::vector<double>& latitudes,
        std::vector<double>& longitudes,
        const Domain& domain ) {
    if (domain.isGlobal()) {
        return 0;
    }

    // TODO: cache me


    // setup global grid
    repres::RepresentationHandle globe(namedgrids::NamedGrid::lookup(globaliseGridname_).representation());
    size_t nbExtraPoints = 0;


    // insert global grid points when outside provided domain
    std::unique_ptr<repres::Iterator> it(globe->iterator());
    while (it->next()) {
        const auto& p = it->pointUnrotated();
        if (!domain.contains(p)) {

            latitudes.push_back(p.lat().value());
            longitudes.push_back(p.lon().value());
            ++nbExtraPoints;

        }
    }

    return nbExtraPoints;
}
#endif


}  // namespace util
}  // namespace mir

