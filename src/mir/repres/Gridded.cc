/*
 * (C) Copyright 1996-2016 ECMWF.
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


#include "mir/repres/Gridded.h"

#include "atlas/grid/Domain.h"
#include "mir/config/LibMir.h"
#include "mir/util/Grib.h"


namespace mir {
namespace repres {


Gridded::Gridded() {}


Gridded::Gridded(const param::MIRParametrisation &parametrisation) {}


Gridded::~Gridded() {}


void Gridded::setComplexPacking(grib_info &info) const {
    info.packing.packing_type = GRIB_UTIL_PACKING_TYPE_GRID_COMPLEX;
}


void Gridded::setSimplePacking(grib_info &info) const {
    info.packing.packing_type = GRIB_UTIL_PACKING_TYPE_GRID_SIMPLE;
}


void Gridded::setSecondOrderPacking(grib_info &info) const {
    info.packing.packing_type = GRIB_UTIL_PACKING_TYPE_GRID_SECOND_ORDER;
}


void Gridded::cropToDomain(const param::MIRParametrisation &parametrisation, context::Context & ctx) const {
    if (!atlasDomain().isGlobal()) {
        Representation::cropToDomain(parametrisation, ctx); // This will throw an exception
    }
}


}  // namespace repres
}  // namespace mir

