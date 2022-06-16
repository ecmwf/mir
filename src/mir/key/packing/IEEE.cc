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


#include "mir/key/packing/IEEE.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/util/Grib.h"


namespace mir {
namespace key {
namespace packing {


static const PackingBuilder<IEEE> __packing("ieee", true, true);


IEEE::IEEE(const std::string& name, const param::MIRParametrisation& param) : Packing(name, param) {
    constexpr long L32  = 32;
    constexpr long L64  = 64;
    constexpr long L128 = 128;

    // Accuracy set by user, otherwise by field (rounded up to a supported precision)
    if (!defineAccuracy_) {
        long accuracy = L32;
        param.fieldParametrisation().get("accuracy", accuracy);

        accuracy_       = accuracy <= L32 ? L32 : accuracy <= L64 ? L64 : L128;
        defineAccuracy_ = accuracy != accuracy_;
    }

    if (gridded()) {
        requireEdition(param, 2);
    }
}


void IEEE::fill(const repres::Representation* /*unused*/, grib_info& info) const {
    Packing::fill(info, CODES_UTIL_PACKING_TYPE_IEEE);
}


void IEEE::set(const repres::Representation* /*unused*/, grib_handle* handle) const {
    Packing::set(handle, gridded() ? "grid_ieee" : "spectral_ieee");
}


}  // namespace packing
}  // namespace key
}  // namespace mir
