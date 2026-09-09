// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/param/DefaultParametrisation.h"

#include "mir/config/LibMir.h"


namespace mir::param {


DefaultParametrisation::DefaultParametrisation() {
    set("style", "ecmwf");
    set("executor", "simple");

    set("interpolation", "linear");
    set("vector-space", "1d-linear");

    set("caching", LibMir::caching());

    set("prune-epsilon", 1e-10);
    set("nclosest", 4L);
    set("distance", 1.);
    set("distance-tolerance", 1.);
    set("pole-displacement-in-degree", 0.);

    set("lsm", false);
    set("lsm-selection", "named");
    set("lsm-named", LibMir::lsmNamed());
    set("lsm-interpolation", "nearest-neighbour");
    set("lsm-weight-adjustment", 0.2);
    set("lsm-value-threshold", 0.5);

    set("spectral-order", "linear");

    set("compare", "scalar");
}


}  // namespace mir::param
