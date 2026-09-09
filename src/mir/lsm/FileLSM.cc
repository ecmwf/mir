// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/lsm/FileLSM.h"

#include <ostream>
#include <sstream>

#include "eckit/utils/MD5.h"

#include "mir/lsm/GribFileMaskFromUser.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir::lsm {


static const FileLSM __lsm_selection("file");


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


}  // namespace mir::lsm
