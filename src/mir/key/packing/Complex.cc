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

#include <ostream>

#include "mir/util/Grib.h"


namespace mir {
namespace key {
namespace packing {


static PackingBuilder<Complex> __packing("complex", "co", true, true);


void Complex::fill(grib_info& info) const {
    savePacking(info, gridded() ? CODES_UTIL_PACKING_TYPE_GRID_COMPLEX : CODES_UTIL_PACKING_TYPE_SPECTRAL_COMPLEX);
    saveAccuracy(info);
    saveEdition(info);
}


void mir::key::packing::Complex::set(grib_handle* handle) const {
    setPacking(handle, gridded() ? "grid_complex" : "spectral_complex");
    setAccuracy(handle);
    setEdition(handle);
}


void Complex::print(std::ostream& out) const {
    out << "Complex[]";
}


}  // namespace packing
}  // namespace key
}  // namespace mir
