/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date September 2017


#include "mir/lsm/NamedLSM.h"

#include <iostream>
#include "eckit/utils/MD5.h"
#include "mir/config/LibMir.h"
#include "mir/lsm/GribFileMaskFromMIR.h"
#include "mir/lsm/MappedMask.h"
#include "mir/lsm/TenMinutesMask.h"


namespace mir {
namespace lsm {


namespace {
static NamedLSM __lsm_selection("named");
}


NamedLSM::NamedLSM(const std::string& name) :
    LSMSelection(name),
    format_(name == "1km" ?   FormatIs1km
          : name == "10min" ? FormatIs10min
          :                   FormatIsGrib),
    path_("~mir/share/mir/masks/lsm." + name + (format_ == FormatIsGrib ? ".grib" : ".mask")) {
}


NamedLSM::~NamedLSM() {
}


void NamedLSM::print(std::ostream& out) const {
    out << "NamedLSM["
            "name=" << name_
        << ",path=" << path_
        << "]";
}


Mask* NamedLSM::create(const std::string& name,
                       const param::MIRParametrisation& param,
                       const repres::Representation& representation,
                       const std::string& which) const {
    Mask* mask =
            format_ == FormatIs1km ?   static_cast<Mask*>(new MappedMask(name, path_, param, representation, which))
          : format_ == FormatIs10min ? static_cast<Mask*>(new TenMinutesMask(name, path_, param, representation, which))
          :                            static_cast<Mask*>(new GribFileMaskFromMIR(name, path_, param, representation, which));

    eckit::Log::debug<LibMir>() << "NamedLSM::create => " << *mask << std::endl;
    return mask;
}


std::string NamedLSM::cacheKey(const std::string& name,
                               const param::MIRParametrisation& param,
                               const repres::Representation& representation,
                               const std::string& which) const {

    eckit::MD5 md5;
    Mask::hashCacheKey(md5, path_, param, representation, which);
    return "named." + name + "." + md5.digest();
}


}  // namespace lsm
}  // namespace mir

