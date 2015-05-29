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


#include "atlas/Grid.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/utils/MD5.h"

#include "mir/lsm/NoneLSM.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Compare.h"
#include "mir/data/MIRField.h"

namespace mir {
namespace lsm {
namespace {


static eckit::Mutex *local_mutex = 0;

static std::map<std::string, Mask *> cache;

static pthread_once_t once = PTHREAD_ONCE_INIT;

static void init() {
    local_mutex = new eckit::Mutex();
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


Mask &Mask::lookup(const param::MIRParametrisation  &parametrisation, const atlas::Grid &grid, const std::string &which) {

    bool lsm = false;
    parametrisation.get("lsm", lsm);

    if (!lsm) {
        return NoneLSM::instance();
    }


    std::string name;

    if (!parametrisation.get("lsm.selection" + which, name)) {
        if (!parametrisation.get("lsm.selection", name)) {
            throw eckit::SeriousBug("No lsm selection method provided");
        }
    }

    name = name +  which;
    const LSMChooser &chooser = LSMChooser::lookup(name);
    std::string key = chooser.cacheKey(name, parametrisation, grid, which);

    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    eckit::Log::info() << "Mask::lookup(" << key << ")" << std::endl;
    std::map<std::string, Mask *>::iterator j = cache.find(key);

    if (j != cache.end()) {
        return *(*j).second;
    }

    Mask *mask = chooser.create(name, parametrisation, grid, which);

    cache[key] = mask;

    return *cache[key];


}


Mask &Mask::lookupInput(const param::MIRParametrisation   &parametrisation, const atlas::Grid &grid) {
    return lookup(parametrisation, grid, ".input");
}


Mask &Mask::lookupOutput(const param::MIRParametrisation   &parametrisation, const atlas::Grid &grid) {
    return lookup(parametrisation, grid, ".output");
}


const data::MIRField &Mask::field() const {
    ASSERT(field_.get());
    return *field_;
}


bool Mask::cacheable() const {
    return true;
}


bool Mask::active() const {
    return true;
}


const std::vector<bool> &Mask::mask() const {
    if (mask_.size() == 0) {
        const util::compare::is_greater_equal_fn< double > check_lsm(0.5);

        ASSERT(!field().hasMissing());
        ASSERT(field().dimensions() == 1);

        const std::vector< double > &values = field().values(0);
        mask_.resize(values.size());
        std::transform(values.begin(), values.end(), mask_.begin(), check_lsm);
    }
    return mask_;
}


}  // namespace logic
}  // namespace mir

