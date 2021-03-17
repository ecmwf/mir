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

    // ECC-1219: on IEEE::set, never define bitsPerValue
    defineAccuracy_  = false;
    definePrecision_ = accuracy_ != bits || definePacking_ || !field.has("accuracy");
    precision_       = accuracy_ == L32 ? 1 : accuracy_ == L64 ? 2 : accuracy_ == L128 ? 3 : 0;

    if (precision_ == 0) {
        std::string msg = "packing=ieee: only supports accuracy=32, 64 and 128";
        Log::error() << msg << std::endl;
        throw exception::UserError(msg);
    }

    if (gridded()) {
        requireEdition(param, 2);
    }
}


void IEEE::fill(const repres::Representation*, grib_info& info) const {
    info.packing.packing = CODES_UTIL_PACKING_SAME_AS_INPUT;
    // (Representation can set edition, so it isn't reset)

    if (definePacking_) {
        info.packing.packing      = CODES_UTIL_PACKING_USE_PROVIDED;
        info.packing.packing_type = CODES_UTIL_PACKING_TYPE_IEEE;
    }

    if (defineEdition_) {
        info.packing.editionNumber = edition_;
    }

    if (definePrecision_) {
        info.extra_set("precision", precision_);
    }
}


void IEEE::set(const repres::Representation*, grib_handle* handle) const {
    Packing::set(handle, gridded() ? "grid_ieee" : "spectral_ieee");

    if (definePrecision_) {
        GRIB_CALL(codes_set_long(handle, "precision", precision_));
    }
}


bool IEEE::printParametrisation(std::ostream& out) const {
    auto sep = Packing::printParametrisation(out) ? "," : "";
    if (definePrecision_) {
        out << sep << "precision=" << precision_;
    }
    return true;
}


bool IEEE::empty() const {
    return Packing::empty() && !definePrecision_;
}


}  // namespace packing
}  // namespace key
}  // namespace mir
