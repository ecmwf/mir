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
/// @date Jul 2016

#include "mir/config/LibMir.h"

#include "mir/param/DefaultParametrisation.h"


namespace mir {
namespace param {


DefaultParametrisation::DefaultParametrisation() {
    set("style", "ecmwf");
    set("executor", "simple");

    set("interpolation", "linear"); // The word 'method' is used in grib
    set("space", "1d.linear");

    bool caching = LibMir::caching();
    set("caching", caching);

    set("prune-epsilon", 1e-10);
    set("nclosest", 4L);

    set("lsm", false);
    set("lsm-selection", "named");
    set("lsm-named", "1km");
    set("lsm-interpolation", "nearest-neighbour");
    set("lsm-weight-adjustment", 0.2);
    set("lsm-value-threshold", 0.5);

    set("spectral-order", "linear");

    set("compare", "scalar");
    set("absolute-error", 1e-10);
    set("relative-error-min", 0.01);
    set("relative-error-max", 0.01);
}


DefaultParametrisation::~DefaultParametrisation() = default;


}  // namespace param
}  // namespace mir

