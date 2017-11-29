/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @date Jul 2016


#include "mir/param/DefaultParametrisation.h"


namespace mir {
namespace param {

DefaultParametrisation::DefaultParametrisation() {
    set("style", "ecmwf");
    set("executor", "simple");

    set("interpolation", "linear"); // The word 'method' is used in grib
    set("dimension", "1d.linear");

    set("caching", true);
    set("kd-trees.caching", true);

    set("prune-epsilon", 1e-10);
    set("nclosest", 4L);

    set("lsm", false);
    set("lsm-selection", "named");
    set("lsm-named", "1km");
    set("lsm-interpolation", "nearest-neighbour");
    set("lsm-weight-adjustment", 0.2);
    set("lsm-value-threshold", 0.5);

    set("resol", "automatic-resolution");
    set("spectral-order", "linear");

    set("absolute-error", 1e-10);
}

DefaultParametrisation::~DefaultParametrisation() {
}


}  // namespace param
}  // namespace mir

