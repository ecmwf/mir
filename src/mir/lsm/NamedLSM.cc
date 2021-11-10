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
#include <ostream>

#include "eckit/utils/MD5.h"

#include "mir/lsm/GribFileMaskFromMIR.h"
#include "mir/lsm/MappedMask.h"
#include "mir/lsm/TenMinutesMask.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"


namespace mir {
namespace lsm {


static NamedLSM __lsm_selection("named");


static NamedMaskBuilder<MappedMask> __NamedMappedMask_1("1km", "~mir/share/mir/masks/lsm.1km.climate.v013.mask");
static NamedMaskBuilder<MappedMask> __NamedMappedMask_2("1km.climate.v020", "~mir/share/mir/masks/lsm.1km.climate.v020.mask");
static NamedMaskBuilder<MappedMask> __NamedMappedMask_3("1km.climate.v013", "~mir/share/mir/masks/lsm.1km.climate.v013.mask");

static NamedMaskBuilder<TenMinutesMask> __NamedTenMinutesMask("10min", "~mir/share/mir/masks/lsm.10min.mask");

static NamedMaskBuilder<GribFileMaskFromMIR> __NamedMask_1("N128", "~mir/share/mir/masks/lsm.N128.grib");
static NamedMaskBuilder<GribFileMaskFromMIR> __NamedMask_2("N256", "~mir/share/mir/masks/lsm.N256.grib");
static NamedMaskBuilder<GribFileMaskFromMIR> __NamedMask_3("N320", "~mir/share/mir/masks/lsm.N320.grib");
static NamedMaskBuilder<GribFileMaskFromMIR> __NamedMask_4("O320", "~mir/share/mir/masks/lsm.O320.grib");
static NamedMaskBuilder<GribFileMaskFromMIR> __NamedMask_5("O640", "~mir/share/mir/masks/lsm.O640.grib");
static NamedMaskBuilder<GribFileMaskFromMIR> __NamedMask_6("O1280", "~mir/share/mir/masks/lsm.O1280.grib");


static util::once_flag once;
static util::recursive_mutex* local_mutex          = nullptr;
static std::map<std::string, NamedMaskFactory*>* m = nullptr;


static void init() {
    local_mutex = new util::recursive_mutex();
    m           = new std::map<std::string, NamedMaskFactory*>();
}


static std::string sane(const std::string& insane) {
    std::string sane(insane);
    std::transform(insane.begin(), insane.end(), sane.begin(), tolower);
    return sane;
}


NamedLSM::NamedLSM(const std::string& name) : LSMSelection(name) {}


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


NamedMaskFactory::NamedMaskFactory(const std::string& name, const std::string& path) : name_(sane(name)), path_(path) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    if (m->find(name_) != m->end()) {
        throw exception::SeriousBug("NamedMaskFactory: duplicate '" + name + "'");
    }

    ASSERT(m->find(name_) == m->end());
    (*m)[name_] = this;
}


NamedMaskFactory::~NamedMaskFactory() {
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    m->erase(name_);
}


Mask* NamedMaskFactory::build(const param::MIRParametrisation& param, const repres::Representation& representation,
                              const std::string& which) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    std::string name;
    param.get("lsm-named-" + which, name) || param.get("lsm-named", name);
    name = sane(name);

    Log::debug() << "NamedMaskFactory: looking for '" << name << "'" << std::endl;
    auto j = m->find(name);
    if (j == m->end()) {
        list(Log::error() << "NamedMaskFactory: unknown '" << name << "', choices are: ");
        throw exception::SeriousBug("NamedMaskFactory: unknown '" + name + "'");
    }

    return j->second->make(param, representation, which);
}


std::string NamedMaskFactory::cacheKey(const param::MIRParametrisation& param,
                                       const repres::Representation& representation, const std::string& which) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    std::string name;
    param.get("lsm-named-" + which, name) || param.get("lsm-named", name);
    name = sane(name);

    Log::debug() << "NamedMaskFactory: looking for '" << name << "'" << std::endl;
    auto j = m->find(name);
    if (j == m->end()) {
        list(Log::error() << "NamedMaskFactory: unknown '" << name << "', choices are: ");
        throw exception::SeriousBug("NamedMaskFactory: unknown '" + name + "'");
    }

    eckit::MD5 md5;
    j->second->hashCacheKey(md5, param, representation, which);
    return "named." + name + "." + md5.digest();
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


}  // namespace lsm
}  // namespace mir
