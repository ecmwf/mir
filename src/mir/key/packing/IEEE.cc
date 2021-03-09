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

#include <ostream>
#include <sstream>

#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/Log.h"


namespace mir {
namespace key {
namespace packing {


static PackingBuilder<IEEE> __packing("ieee", true, true);


IEEE::IEEE(const param::MIRParametrisation& param) : Packing(param) {
    constexpr long L32  = 32;
    constexpr long L64  = 64;
    constexpr long L128 = 128;

    long bits = L32;
    if (!param.userParametrisation().get("accuracy", bits)) {
        if (param.get("accuracy", bits)) {
            bits = bits < L32 ? L32 : bits < L64 ? L64 : L128;
        }
    }

    bitsPerValue_ = bits;
    precision_    = bits == L32 ? 1 : bits == L64 ? 2 : bits == L128 ? 3 : 0;

    if (precision_ == 0) {
        std::ostringstream msg;
        msg << *this << ": only supports accuracy 32, 64 and 128";
        Log::error() << msg.str() << std::endl;
        throw exception::UserError(msg.str());
    }
}


void IEEE::fill(grib_info& info) const {
    savePacking(info, CODES_UTIL_PACKING_TYPE_IEEE);
    saveAccuracy(info);
    saveEdition(info);

    info.extra_set("precision", precision_);
}


void IEEE::set(grib_handle* handle) const {
    // set bitsPerValue before re-packing, in case current setting is not suitable
    GRIB_CALL(codes_set_long(handle, "bitsPerValue", bitsPerValue_));

    setPacking(handle, gridded() ? "grid_ieee" : "spectral_ieee");

    GRIB_CALL(codes_set_long(handle, "precision", precision_));

    setEdition(handle);
}


void IEEE::print(std::ostream& out) const {
    out << "IEEE[precision=" << precision_ << "]";
}


}  // namespace packing
}  // namespace key
}  // namespace mir
