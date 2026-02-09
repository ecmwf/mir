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


#include "mir/lsm/NamedLSM.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <memory>
#include <ostream>

#include "eckit/filesystem/PathName.h"
#include "eckit/geo/cache/Download.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/utils/MD5.h"
#include "eckit/utils/StringTools.h"

#include "mir/config/LibMir.h"
#include "mir/lsm/GribFileMaskFromMIR.h"
#include "mir/lsm/MappedMask.h"
#include "mir/lsm/TenMinutesMask.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"


namespace mir::lsm {


static NamedLSM const __lsm_selection("named");


static std::string sane(const std::string& insane) {
    std::string sane(insane);
    std::transform(insane.begin(), insane.end(), sane.begin(), tolower);
    return sane;
}


struct map_t : std::map<std::string, std::unique_ptr<NamedMaskFactory>> {
    map_t() {
        const auto path = LibMir::configFile(LibMir::config_file::LSM);
        if (!path.exists()) {
            return;
        }

        for (const auto& kv : eckit::ValueMap(eckit::YAMLParser::decodeFile(path))) {
            std::string name = sane(kv.first);
            std::string path = kv.second.as<eckit::ValueMap>()["path"];
            std::string type = kv.second.as<eckit::ValueMap>()["type"];

            if (!emplace(name, type == "grib"     ? mapped_type(new NamedMaskBuilder<GribFileMaskFromMIR>(name, path))
                               : type == "mapped" ? mapped_type(new NamedMaskBuilder<MappedMask>(name, path))
                               : type == "ten minutes" ? mapped_type(new NamedMaskBuilder<TenMinutesMask>(name, path))
                                                       : throw exception::UserError("NamedLSM: unknown type '" + type +
                                                                                    "' for '" + name + "'"))
                     .second) {
                throw exception::SeriousBug("NamedLSM: duplicate '" + name + "'");
            }
        }
    }
} static* m = nullptr;


static util::once_flag once;
static util::recursive_mutex* local_mutex = nullptr;


static void init() {
    local_mutex = new util::recursive_mutex();
    m           = new map_t();
}


NamedLSM::NamedLSM(const std::string& name) : LSMSelection(name) {}


eckit::PathName NamedMaskFactory::resolve_path(const param::MIRParametrisation& param, const std::string& path) {
    if (!eckit::PathName(path).exists()) {
        bool caching = LibMir::caching();
        param.get("caching", caching);

        if (!caching) {
            Log::warning() << "NamedMaskFactory: caching disabled, cannot resolve '" << path << "'" << std::endl;
        }
        else if (const auto lib = LibMir::instance().name(); eckit::StringTools::beginsWith(path, "~" + lib + "/")) {
            using eckit::geo::cache::Download;

            static Download download(eckit::PathName{LibMir::cacheDir()} / lib / "masks");
            return download.to_cached_path(LibMir::homeUrl() + path.substr(lib.length() + 1));
        }
    }

    return path;
}


void NamedLSM::print(std::ostream& out) const {
    out << "NamedLSM[name=" << name_ << "]";
}


Mask* NamedLSM::create(const param::MIRParametrisation& param, const repres::Representation& representation,
                       const std::string& which) const {
    return NamedMaskFactory::build(param, representation, which);
}


std::string NamedLSM::cacheKey(const param::MIRParametrisation& param, const repres::Representation& representation,
                               const std::string& which) const {
    return NamedMaskFactory::cacheKey(param, representation, which);
}


Mask* NamedMaskFactory::build(const param::MIRParametrisation& param, const repres::Representation& representation,
                              const std::string& which) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    std::string name;
    param.get("lsm-named-" + which, name) || param.get("lsm-named", name);
    name = sane(name);

    Log::debug() << "NamedMaskFactory: looking for '" << name << "'" << std::endl;
    if (auto j = m->find(name); j != m->end()) {
        return j->second->make(param, representation, which);
    }

    list(Log::error() << "NamedMaskFactory: unknown '" << name << "', choices are: ");
    throw exception::SeriousBug("NamedMaskFactory: unknown '" + name + "'");
}


std::string NamedMaskFactory::cacheKey(const param::MIRParametrisation& param,
                                       const repres::Representation& representation, const std::string& which) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    std::string name;
    param.get("lsm-named-" + which, name) || param.get("lsm-named", name);
    name = sane(name);

    Log::debug() << "NamedMaskFactory: looking for '" << name << "'" << std::endl;
    if (auto j = m->find(name); j != m->end()) {
        eckit::MD5 md5;
        j->second->hashCacheKey(md5, param, representation, which);
        return "named." + name + "." + md5.digest();
    }

    list(Log::error() << "NamedMaskFactory: unknown '" << name << "', choices are: ");
    throw exception::SeriousBug("NamedMaskFactory: unknown '" + name + "'");
}


void NamedMaskFactory::list(std::ostream& out) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


}  // namespace mir::lsm
