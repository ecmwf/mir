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


#include "mir/lsm/FileLSM.h"

#include <ostream>
#include <sstream>

#include "eckit/utils/MD5.h"

#include "mir/lsm/GribFileMaskFromUser.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace lsm {


static FileLSM __lsm_selection("file");


FileLSM::FileLSM(const std::string& name) : LSMSelection(name) {}


void FileLSM::print(std::ostream& out) const {
    out << "FileLSM[name=" << name_ << "]";
}


std::string FileLSM::path(const param::MIRParametrisation& param, const std::string& which) const {
    std::string path;
    if (param.get("lsm-file-" + which, path) || param.get("lsm-file", path)) {
        if (!path.empty()) {
            return path;
        }
    }

    std::ostringstream os;
    os << *this << ": no path specified";
    throw exception::UserError(os.str());
}


Mask* FileLSM::create(const param::MIRParametrisation& param, const repres::Representation& representation,
                      const std::string& which) const {
    return new GribFileMaskFromUser(path(param, which), param, representation, which);
}


std::string FileLSM::cacheKey(const param::MIRParametrisation& param, const repres::Representation& representation,
                              const std::string& which) const {
    eckit::MD5 md5;
    GribFileMaskFromUser::hashCacheKey(md5, path(param, which), param, representation, which);

    return "file." + md5.digest();
}


}  // namespace lsm
}  // namespace mir
