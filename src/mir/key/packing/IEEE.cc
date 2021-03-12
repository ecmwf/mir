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
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/Log.h"


namespace mir {
namespace key {
namespace packing {


static PackingBuilder<IEEE> __packing("ieee", true, true);


IEEE::IEEE(const std::string& name, const param::MIRParametrisation& param) : Packing(name, param) {
    auto& user  = param.userParametrisation();
    auto& field = param.fieldParametrisation();

    constexpr long L32  = 32;
    constexpr long L64  = 64;
    constexpr long L128 = 128;

    // Accuracy set by user, otherwise by field (rounded up to a supported precision)
    long bits = L32;
    field.get("accuracy", bits);

    if (!user.get("accuracy", accuracy_)) {
        accuracy_ = bits <= L32 ? L32 : bits <= L64 ? L64 : L128;
    }

    // Note: On IEEE::set, this should set bitsPerValue before re-packing
    defineAccuracy_ = accuracy_ != bits || !field.has("accuracy");
    precision_      = accuracy_ == L32 ? 1 : accuracy_ == L64 ? 2 : accuracy_ == L128 ? 3 : 0;

    if (precision_ == 0 || precision_ == 3) {
        std::string msg = "packing=ieee: ecCodes only supports accuracy 32 and 64";
        Log::error() << msg << std::endl;
        throw exception::UserError(msg);
    }

    if (gridded()) {
        requireEdition(param, 2);
    }
}


void IEEE::fill(const repres::Representation*, grib_info& info) const {
    Packing::fill(info, CODES_UTIL_PACKING_TYPE_IEEE);
    info.extra_set("precision", precision_);
}


void IEEE::set(const repres::Representation*, grib_handle* handle) const {
    Packing::set(handle, gridded() ? "grid_ieee" : "spectral_ieee");
    GRIB_CALL(codes_set_long(handle, "precision", precision_));
}


}  // namespace packing
}  // namespace key
}  // namespace mir
