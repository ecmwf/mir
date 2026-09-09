// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/repres/regular/Mercator.h"

#include "mir/util/Exceptions.h"


namespace mir::repres::regular {


static const RepresentationBuilder<Mercator> __builder("mercator");


Mercator::Mercator(const param::MIRParametrisation& param) : RegularGrid(param, make_proj_spec(param)) {}


void Mercator::fillGrib(grib_info& /*info*/) const {
    NOTIMP;
}


void Mercator::fillSpec(CustomSpec&) const {
    NOTIMP;
}


}  // namespace mir::repres::regular
