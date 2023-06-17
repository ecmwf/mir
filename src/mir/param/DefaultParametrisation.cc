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


#include "mir/param/DefaultParametrisation.h"

#include "mir/config/LibMir.h"


namespace mir::param {


DefaultParametrisation::DefaultParametrisation() {
    set("style", "ecmwf");
    set("executor", "simple");

    set("interpolation", "linear/k-nearest-neighbours");
    set("vector-space", "1d-linear");

    set("caching", LibMir::caching());

    set("prune-epsilon", 1e-10);
    set("nclosest", 4L);

    set("lsm", false);
    set("lsm-selection", "named");
    set("lsm-named", LibMir::lsmNamed());
    set("lsm-interpolation", "nearest-neighbour");
    set("lsm-weight-adjustment", 0.2);
    set("lsm-value-threshold", 0.5);

    set("spectral-order", "linear");

    set("compare", "scalar");
}


DefaultParametrisation::~DefaultParametrisation() = default;


}  // namespace mir::param
