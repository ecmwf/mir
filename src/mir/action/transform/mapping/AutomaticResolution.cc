/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @date May 2017


#include "mir/action/transform/mapping/AutomaticResolution.h"

#include "eckit/exception/Exceptions.h"
#include "mir/config/LibMir.h"
#include "mir/namedgrids/NamedGrid.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Increments.h"


namespace mir {
namespace action {
namespace transform {
namespace mapping {


AutomaticResolution::AutomaticResolution(const param::MIRParametrisation& parametrisation) :
    parametrisation_(parametrisation) {

    std::string map;
    parametrisation_.get("spectral-mapping", map);

    mapping_.reset(action::transform::mapping::MappingFactory::build(map));
    ASSERT(mapping_);
}


long AutomaticResolution::computeNi(const param::MIRParametrisation& parametrisation) {
    long Ni = 0;

    std::vector<double> grid;
    if (parametrisation.get("user.grid", grid)) {
        ASSERT(grid.size() == 2);

        // get Nj: only South-North increment (grid[1]) is used
        util::BoundingBox bbox;
        ASSERT(bbox.contains( 90, 0));
        ASSERT(bbox.contains(-90, 0));
        long Nj = long(bbox.computeNj(util::Increments(grid[0], grid[1])));

        // get Ni(Nj): -1 corrects periodicity
        Ni = 2 * (Nj - 1);
    }

    long N = 0;
    if (parametrisation.get("user.reduced", N) ||
        parametrisation.get("user.regular", N) ||
        parametrisation.get("user.octahedral", N)) {
        Ni = 4 * N;
    }

    std::string gridname;
    if (parametrisation.get("user.gridname", gridname)) {
        N = long(namedgrids::NamedGrid::lookup(gridname).gaussianNumber());
        Ni = 4 * N;
    }

    return Ni;
}


bool AutomaticResolution::get(const std::string &name, long &value) const {
    ASSERT(name == "truncation");

    if (parametrisation_.has("griddef")) {
        // TODO: this is temporary, no support yet for unstuctured grids
        value = 63L;
        return true;
    }

    // get points-per-latitude
    long Ni = computeNi(parametrisation_);
    ASSERT(Ni > 0);

    value = mapping_->getTruncationFromPointsPerLatitude(Ni);
    return true;
}


bool AutomaticResolution::get(const std::string& name, size_t& value) const {
    long T;
    if (get(name, T)) {
        value = size_t(T);
        return true;
    }
    return false;
}


void AutomaticResolution::print(std::ostream& out) const {
    ASSERT(mapping_);
    out << "AutomaticResolution[";
    mapping_->print(out);
    out << "]";
}


}  // namespace mapping
}  // namespace transform
}  // namespace action
}  // namespace mir
