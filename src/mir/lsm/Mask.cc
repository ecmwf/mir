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


#include "mir/lsm/Mask.h"

#include "eckit/filesystem/PathName.h"
#include "eckit/utils/MD5.h"

#include "mir/lsm/NoneLSM.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Mutex.h"
//#include "mir/param/RuntimeParametrisation.h"
//#include "mir/repres/latlon/RegularLL.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir {
namespace lsm {


static util::recursive_mutex* local_mutex  = nullptr;
static std::map<std::string, Mask*>* cache = nullptr;
static util::once_flag once;
static void init() {
    local_mutex = new util::recursive_mutex();
    cache       = new std::map<std::string, Mask*>();
}


Mask::Mask() = default;


Mask::~Mask() = default;


void Mask::hash(eckit::MD5&) const {}


void Mask::hashCacheKey(eckit::MD5& md5, const eckit::PathName& path, const param::MIRParametrisation& parametrisation,
                        const repres::Representation& representation, const std::string& which) {

    std::string interpolation;
    if (!parametrisation.get("lsm-interpolation-" + which, interpolation)) {
        if (!parametrisation.get("lsm-interpolation", interpolation)) {
            throw exception::SeriousBug("No interpolation method defined for land-sea mask");
        }
    }

    md5 << path.asString();
    md5 << interpolation;
    md5 << representation.uniqueName();
}


Mask& Mask::lookup(const param::MIRParametrisation& parametrisation, const repres::Representation& representation,
                   const std::string& which) {

    // lsm = true is a requirement for lsm processing
    bool lsm = false;
    parametrisation.get("lsm", lsm);

    if (!lsm) {
        return NoneLSM::noMask();
    }


    std::string name;
    if (!parametrisation.get("lsm-selection-" + which, name)) {
        if (!parametrisation.get("lsm-selection", name)) {
            throw exception::SeriousBug("No lsm selection method provided");
        }
    }

    auto& chooser   = LSMSelection::lookup(name);
    std::string key = chooser.cacheKey(parametrisation, representation, which);

    {
        // To protect cache
        util::call_once(once, init);
        util::lock_guard<util::recursive_mutex> lock(*local_mutex);

        Log::debug() << "Mask::lookup(" << key << ")" << std::endl;
        auto j = cache->find(key);
        if (j != cache->end()) {
            return *(j->second);
        }

        return *((*cache)[key] = chooser.create(parametrisation, representation, which));
    }
}


Mask& Mask::lookupInput(const param::MIRParametrisation& parametrisation,
                        const repres::Representation& representation) {
    return lookup(parametrisation, representation, "input");
}


Mask& Mask::lookupOutput(const param::MIRParametrisation& parametrisation,
                         const repres::Representation& representation) {
    return lookup(parametrisation, representation, "output");
}


static bool same(const param::MIRParametrisation& parametrisation1, const param::MIRParametrisation& parametrisation2,
                 const std::string& /*which*/) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    // Check 'master' lsm key
    bool lsm1 = false;
    parametrisation1.get("lsm", lsm1);

    bool lsm2 = false;
    parametrisation2.get("lsm", lsm2);

    return lsm1 == lsm2;
#if 0
    if (lsm1 != lsm2) {
        return false;
    }

    // Check LSM selection method
    std::string name1;
    parametrisation1.get("lsm-selection-" + which, name1) || parametrisation1.get("lsm-selection", name1);

    std::string name2;
    parametrisation2.get("lsm-selection-" + which, name2) || parametrisation2.get("lsm-selection", name2);

    if (name1 != name2) {
        return false;
    }

    // Check LSM cache key
    // TODO: this creates a dummy representation, could be better...
    param::RuntimeParametrisation runtime1(parametrisation1);
    setParametrisation(runtime1);
    repres::latlon::RegularLL representation1(runtime1);

    param::RuntimeParametrisation runtime2(parametrisation2);
    setParametrisation(runtime2);
    repres::latlon::RegularLL representation2(runtime2);

    ASSERT(name1 == name2);
    const LSMSelection& chooser = LSMSelection::lookup(name1);
    const std::string key1 = chooser.cacheKey(parametrisation1, representation1, which);
    const std::string key2 = chooser.cacheKey(parametrisation2, representation2, which);

    if (key1 != key2) {
        return false;
    }
    return true;
#endif
}


bool Mask::sameInput(const param::MIRParametrisation& parametrisation1,
                     const param::MIRParametrisation& parametrisation2) {
    return same(parametrisation1, parametrisation2, "input");
}


bool Mask::sameOutput(const param::MIRParametrisation& parametrisation1,
                      const param::MIRParametrisation& parametrisation2) {
    return same(parametrisation1, parametrisation2, "output");
}


}  // namespace lsm
}  // namespace mir
