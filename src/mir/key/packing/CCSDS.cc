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


#include "mir/key/packing/CCSDS.h"

#include "eckit/config/Resource.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/Log.h"


namespace mir::key::packing {


static const PackingBuilder<CCSDS> __packing("ccsds", false, true);


CCSDS::CCSDS(const std::string& name, const param::MIRParametrisation& param) : Packing(name, param) {
    if (!gridded()) {
        std::string msg = "packing=ccsds: only supports gridded fields";
        Log::error() << msg << std::endl;
        throw exception::UserError(msg);
    }

    long required = 2;
    long edition  = 0;
    if (param.get("edition", edition) && edition != required) {
        static const bool grib_edition_conversion_default =
            eckit::Resource<bool>("$MIR_GRIB_EDITION_CONVERSION;mirGribEditionConversion", false);
        bool grib_edition_conversion = grib_edition_conversion_default;
        param.get("grib-edition-conversion", grib_edition_conversion);

        if (!grib_edition_conversion) {
            throw exception::UserError("Packing: edition conversion is required, but disabled");
        }
    }

    if (edition != required){
        edition_       = required;
        defineEdition_ = true;
    }
}


void CCSDS::fill(const repres::Representation* /*unused*/, grib_info& info) const {
    Packing::fill(info, CODES_UTIL_PACKING_TYPE_CCSDS);
}


void CCSDS::set(const repres::Representation* /*unused*/, grib_handle* handle) const {
    Packing::set(handle, "grid_ccsds");
}


}  // namespace mir::key::packing
