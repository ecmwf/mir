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


#include "mir/key/packing/Complex.h"

#include "mir/util/Grib.h"


namespace mir {
namespace key {
namespace packing {


static const PackingBuilder<Complex> __packing("complex", "co", true, true);


Complex::Complex(const std::string& name, const param::MIRParametrisation& param) : Packing(name, param) {
    if (gridded()) {
        requireEdition(param, 2);
    }
}


void Complex::fill(const repres::Representation*, grib_info& info) const {
    Packing::fill(info, gridded() ? CODES_UTIL_PACKING_TYPE_GRID_COMPLEX : CODES_UTIL_PACKING_TYPE_SPECTRAL_COMPLEX);
}


void Complex::set(const repres::Representation*, grib_handle* handle) const {
    Packing::set(handle, gridded() ? "grid_complex" : "spectral_complex");
}


}  // namespace packing
}  // namespace key
}  // namespace mir
