// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/util/GlobaliseUnstructured.h"

#include <memory>
#include <variant>

#include "eckit/geo/Grid.h"
#include "eckit/geo/Search.h"

#include "mir/key/grid/Grid.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/repres/UnstructuredGrid.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Search.h"


namespace mir::util {


GlobaliseUnstructured::GlobaliseUnstructured(const param::MIRParametrisation& parametrisation) :
    parametrisation_(parametrisation) {

    globaliseGridname_ = "O16";
    parametrisation_.get("globalise-gridname", globaliseGridname_);

    // Earth great-circle distance of approx. 5 degree
    constexpr double EARTH_GC_5DEG = 555975.;
    parametrisation_.get("globalise-missing-radius", globaliseMissingRadius_ = EARTH_GC_5DEG);

    ASSERT(globaliseMissingRadius_ > 0.);
}


GlobaliseUnstructured::~GlobaliseUnstructured() = default;


size_t GlobaliseUnstructured::appendGlobalPoints(std::vector<double>& latitudes, std::vector<double>& longitudes) {
    if (globaliseGridname_.empty()) {
        return 0;
    }

    // TODO: cache me


    // setup k-d tree on temporary unstructured grid
    repres::UnstructuredGrid unstructuredGrid(latitudes, longitudes);
    const auto tree = util::make_search(parametrisation_, unstructuredGrid);


    // setup global grid
    repres::RepresentationHandle globe(key::grid::Grid::lookup(globaliseGridname_).representation());
    size_t nbExtraPoints = 0;


    // insert global grid points when distant enough from provided grid points
    for (const auto& point : globe->canonicalGrid()) {
        if (const auto p = std::get<PointLonLat>(point); tree->search_nn(p).distance > globaliseMissingRadius_) {
            latitudes.push_back(p.lat());
            longitudes.push_back(p.lon());
            ++nbExtraPoints;
        }
    }

    return nbExtraPoints;
}


}  // namespace mir::util
