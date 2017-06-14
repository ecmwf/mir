/*
 * (C) Copyright 1996-2017 ECMWF.
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


#include "mir/style/MARSStyle.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "eckit/memory/ScopedPtr.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/namedgrids/NamedGrid.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/style/IntermediateGrid.h"
#include "mir/style/Mapping.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Increments.h"


namespace mir {
namespace style {


MARSStyle::MARSStyle(const param::MIRParametrisation &parametrisation):
    ECMWFStyle(parametrisation) {
}


MARSStyle::~MARSStyle() {
}


void MARSStyle::print(std::ostream &out) const {
    out << "MARSStyle[]";
}


long MARSStyle::getTargetGaussianNumber() const {

    // get N from number of points in half-meridian (uses only grid[1] South-North increment)
    std::vector<double> grid;
    if (parametrisation_.get("user.grid", grid)) {
        ASSERT(grid.size() == 2);

        util::BoundingBox bbox(90, 0, 0, 360);
        long N = long(bbox.computeNj(util::Increments(eckit::Fraction(grid[0]), eckit::Fraction(grid[1])))) - 1;
        return N;
    }

    // get Gaussian N directly
    long N = 0;
    if (parametrisation_.get("user.reduced", N) ||
        parametrisation_.get("user.regular", N) ||
        parametrisation_.get("user.octahedral", N)) {
        return N;
    }

    // get Gaussian N given a gridname
    std::string gridname;
    if (parametrisation_.get("user.gridname", gridname)) {
        N = long(namedgrids::NamedGrid::lookup(gridname).gaussianNumber());
        return N;
    }

    std::ostringstream os;
    os << "MARSStyle: cannot calculate Gaussian number (N) from target grid";
    throw eckit::SeriousBug(os.str());
}


long MARSStyle::getIntendedTruncation() const {

    // TODO: this is temporary, no support yet for unstuctured grids
    if (parametrisation_.has("griddef")) {
        return 63L;
    }

    // Set truncation based on target grid's equivalent Gaussian N and spectral mapping
    bool autoresol = true;
    parametrisation_.get("autoresol", autoresol);

    if (autoresol) {

        long Tin = 0L;
        ASSERT(parametrisation_.get("field.truncation", Tin));

        std::string spectralMapping = "linear";
        parametrisation_.get("spectral-mapping", spectralMapping);

        // get truncation from points-per-latitude, limited to input
        long N = getTargetGaussianNumber();
        ASSERT(N > 0);

        eckit::ScopedPtr<Mapping> map(MappingFactory::build(spectralMapping));
        ASSERT(map);

        long T = map->getTruncationFromGaussianNumber(N);
        if (T > Tin) {
            eckit::Log::warning() << "Automatic truncation " << T << " ('autoresol') limited by input truncation " << Tin << std::endl;
            return Tin;
        }
        return T;
    }

    // Set truncation if manually specified
    long T = 0;
    parametrisation_.get("user.truncation", T);

    return T;
}


void MARSStyle::sh2grid(action::ActionPlan& plan) const {

    long truncation = getIntendedTruncation();
    if (truncation) {
        plan.add("transform.sh-truncate", "truncation", truncation);
    }

    bool vod2uv = false;
    parametrisation_.get("vod2uv", vod2uv);
    std::string transform = vod2uv? "sh-vod-to-uv-" : "sh-scalar-to-";  // completed later

    // set an intermediate Gaussian grid with intended truncation
    std::string intermediate_grid;
    parametrisation_.get("spectral-intermediate-grid", intermediate_grid);

    if (intermediate_grid.length()) {
        param::RuntimeParametrisation runtime(parametrisation_);
        if (truncation) {
            runtime.set("truncation", truncation);
        }
        plan.add("transform." + transform + "namedgrid", "gridname", IntermediateGridFactory::build(intermediate_grid, runtime));
        grid2grid(plan);
        return;
    }

    if (parametrisation_.has("user.grid")) {
        plan.add("transform." + transform + "regular-ll");

        if (parametrisation_.has("user.rotation")) {
            plan.add("interpolate.grid2rotated-regular-ll");

            bool wind = false;
            parametrisation_.get("wind", wind);

            if (wind || vod2uv) {
                plan.add("filter.adjust-winds-directions");
                selectWindComponents(plan);
            }
        }

    }

    if (parametrisation_.has("user.reduced")) {
        plan.add("transform." + transform + "reduced-gg");
    }

    if (parametrisation_.has("user.regular")) {
        plan.add("transform." + transform + "regular-gg");
    }

    if (parametrisation_.has("user.octahedral")) {
        plan.add("transform." + transform + "octahedral-gg");
    }

    if (parametrisation_.has("user.pl")) {
        plan.add("transform." + transform + "reduced-gg-pl-given");
    }

    if (parametrisation_.has("user.gridname")) {
        std::string gridname;
        ASSERT(parametrisation_.get("gridname", gridname));
        plan.add("transform." + transform + "namedgrid");
    }

    if (parametrisation_.has("user.griddef")) {
        std::string griddef;
        ASSERT(parametrisation_.get("griddef", griddef));
        // TODO: this is temporary
        plan.add("transform." + transform + "octahedral-gg", "octahedral", 64L);
        plan.add("interpolate.grid2griddef");
    }

    if (isWindComponent()) {
        plan.add("filter.adjust-winds-scale-cos-latitude");
    }

    if (!parametrisation_.has("user.rotation")) {
        selectWindComponents(plan);
    }
}


namespace {
static MIRStyleBuilder<MARSStyle> __style("mars");
}


}  // namespace style
}  // namespace mir
