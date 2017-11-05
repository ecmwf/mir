/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include "mir/lsm/FileLSM.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "eckit/utils/MD5.h"
#include "mir/lsm/GribFileMask.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace lsm {


namespace {
static FileLSM __lsm_selection("file");
}


FileLSM::FileLSM(const std::string& name) :
    LSMSelection(name) {
}


FileLSM::~FileLSM() {
}


void FileLSM::print(std::ostream& out) const {
    out << "FileLSM["
        << name_
//        << ",which=" << which_
        << "]";
}


std::string FileLSM::path(const param::MIRParametrisation& param, const std::string& which_) const {
    std::string path;
    if (param.get("lsm-file-" + which_, path) || param.get("lsm-file", path)) {
        return path;
    }

    std::ostringstream os;
    os << *this << " no path specified";
    throw eckit::UserError(os.str());
}


Mask* FileLSM::create(
        const std::string& name,
        const param::MIRParametrisation& param,
        const repres::Representation& representation,
        const std::string& which) const {
    return new GribFileMask(name, path(param, which), param, representation, which);
}


std::string FileLSM::cacheKey(
        const std::string& name,
        const param::MIRParametrisation& param,
        const repres::Representation& representation,
        const std::string& which) const {
    eckit::MD5 md5;
    GribFileMask::hashCacheKey(md5, path(param, which), param, representation, which);

    return "file." + md5.digest();
}


}  // namespace lsm
}  // namespace mir

