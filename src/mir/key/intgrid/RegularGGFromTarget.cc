// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/key/intgrid/RegularGGFromTarget.h"

#include "mir/key/grid/Grid.h"


namespace mir::key::intgrid {


static const IntgridBuilder<RegularGGFromTarget> __intgrid1("regular-gg-from-target");
static const IntgridBuilder<RegularGGFromTargetCompatible> __intgrid2("regular-gg-from-target-compatible");
static const IntgridBuilder<RegularGGFromTargetCompatible> __intgrid3("automatic");
static const IntgridBuilder<RegularGGFromTargetCompatible> __intgrid4("auto");
static const IntgridBuilder<RegularGGFromTargetCompatible> __intgrid5("AUTO");


static long gaussian_number(const param::MIRParametrisation& param, bool compatible) {
    const grid::Target target(param);
    return compatible || !target.gaussian || target.rotated ? target.gaussianNumber : 0;
}


RegularGGFromTargetCompatible::RegularGGFromTargetCompatible(const param::MIRParametrisation& param) :
    RegularGGFromTargetCompatible(param, gaussian_number(param, true)) {}


RegularGGFromTargetCompatible::RegularGGFromTargetCompatible(const param::MIRParametrisation& param, long N) :
    Intgrid(param), gridname_(N > 0 ? "F" + std::to_string(N) : "") {}


RegularGGFromTarget::RegularGGFromTarget(const param::MIRParametrisation& param) :
    RegularGGFromTargetCompatible(param, gaussian_number(param, false)) {}


}  // namespace mir::key::intgrid
