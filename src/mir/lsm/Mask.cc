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
/// @author Tiago Quintino
/// @date Apr 2015


#include "mir/lsm/Mask.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/utils/MD5.h"
#include "mir/config/LibMir.h"
#include "mir/lsm/NoneLSM.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"


namespace mir {
namespace lsm {
namespace {


static eckit::Mutex *local_mutex = 0;

static std::map<std::string, Mask *> *cache = 0;

static pthread_once_t once = PTHREAD_ONCE_INIT;

static void init() {
    local_mutex = new eckit::Mutex();
    cache = new std::map<std::string, Mask *>();
}


}  // (anonymous namespace)


Mask::Mask(const std::string &name):
    name_(name) {
}


Mask::~Mask() {
}


void Mask::hash(eckit::MD5 &md5) const {
    md5 << name_;
}


void Mask::hashCacheKey(eckit::MD5& md5,
                        const eckit::PathName& path,
                        const param::MIRParametrisation& parametrisation,
                        const repres::Representation& representation,
                        const std::string& which) {

    std::string interpolation;
    if (!parametrisation.get("lsm-interpolation-" + which, interpolation)) {
        if (!parametrisation.get("lsm-interpolation", interpolation)) {
            throw eckit::SeriousBug("Not interpolation method defined for land-sea mask");
        }
    }

    md5 << path.asString();
    md5 << interpolation;
    md5 << representation.uniqueName();
}


Mask &Mask::lookup(const param::MIRParametrisation& parametrisation, const repres::Representation& representation, const std::string& which) {

    // lsm = true is a requirement for lsm processing
    bool lsm = false;
    parametrisation.get("lsm", lsm);

    // lsm-parameter-list is optional, and filters lsm processing for specific paramIds
    if (lsm) {
        std::vector<long> list;
        parametrisation.get("lsm-parameter-list", list);

        long paramId = 0;
        parametrisation.get("paramId", paramId);

        if (paramId > 0 && list.size()) {
            lsm = std::find(list.begin(), list.end(), paramId) != list.end();
        }
    }

    if (!lsm) {
        return NoneLSM::noMask();
    }


    std::string name;
    if (!parametrisation.get("lsm-selection-" + which, name)) {
        if (!parametrisation.get("lsm-selection", name)) {
            throw eckit::SeriousBug("No lsm selection method provided");
        }
    }

//    name = name + "-" + which;
    const LSMSelection &chooser = LSMSelection::lookup(name);
    std::string key = chooser.cacheKey(name, parametrisation, representation, which);

    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    eckit::Log::debug<LibMir>() << "Mask::lookup(" << key << ")" << std::endl;
    std::map<std::string, Mask *>::iterator j = cache->find(key);

    if (j != cache->end()) {
        return *(*j).second;
    }

    Mask *mask = chooser.create(name, parametrisation, representation, which);

    (*cache)[key] = mask;

    return *(*cache)[key];
}


Mask &Mask::lookupInput(const param::MIRParametrisation& parametrisation, const repres::Representation& representation) {
    return lookup(parametrisation, representation, "input");
}


Mask &Mask::lookupOutput(const param::MIRParametrisation& parametrisation, const repres::Representation& representation) {
    return lookup(parametrisation, representation, "output");
}


}  // namespace lsm
}  // namespace mir

