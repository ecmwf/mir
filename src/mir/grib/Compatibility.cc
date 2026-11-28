// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/grib/Compatibility.h"

#include <ostream>
#include <sstream>
#include <vector>

#include "eckit/utils/Tokenizer.h"

#include "mir/grib/OutputKeys.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"


namespace mir::grib {


static const std::string NO_LOCAL_EXTENSION{"no-local-extension"};
static const std::string ADJUST_SPECTRAL_PACKING{"adjust-spectral-packing"};


Compatibility::Compatibility(const param::MIRParametrisation& param) {
    param.userParametrisation().get("compatibility", name_);

    std::vector<std::string> names;
    eckit::Tokenizer("/")(name_, names);

    for (const auto& name : names) {
        if (name == NO_LOCAL_EXTENSION) {
            noLocalExtension_ = true;
        }
        else if (name == ADJUST_SPECTRAL_PACKING) {
            adjustSpectralPacking_ = true;
        }
        else {
            std::ostringstream msg;
            msg << "Compatibility: unknown '" << name << "', choices are: ";
            list(msg);
            throw exception::UserError(msg.str());
        }
    }
}


void Compatibility::keys(long edition, OutputKeys& keys) const {
    if (noLocalExtension_ && edition == 1) {
        keys.add_after("deleteLocalDefinition", 1L);
    }
}


std::string Compatibility::packing_type(const std::string& type) const {
    return adjustSpectralPacking_ && type == "spectral_simple" ? "spectral_complex" : type;
}


long Compatibility::packing_type(long type) const {
    return adjustSpectralPacking_ && type == CODES_UTIL_PACKING_TYPE_SPECTRAL_SIMPLE
               ? CODES_UTIL_PACKING_TYPE_SPECTRAL_COMPLEX
               : type;
}


void Compatibility::list(std::ostream& out) {
    out << NO_LOCAL_EXTENSION << ", " << ADJUST_SPECTRAL_PACKING;
}


}  // namespace mir::grib
