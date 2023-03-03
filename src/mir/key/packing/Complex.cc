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


namespace mir::key::packing {


static const PackingBuilder<Complex> __packing("complex", "co", true, false);


Complex::Complex(const std::string& name, const param::MIRParametrisation& param) : Packing(name, param) {
    ASSERT(!gridded());
}


void Complex::fill(const repres::Representation* /*unused*/, grib_info& info) const {
    Packing::fill(info, CODES_UTIL_PACKING_TYPE_SPECTRAL_COMPLEX);
}


void Complex::set(const repres::Representation* /*unused*/, grib_handle* handle) const {
    Packing::set(handle, "spectral_complex");
}


}  // namespace mir::key::packing
