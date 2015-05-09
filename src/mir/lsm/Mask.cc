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

#include "mir/lsm/Mask.h"

#include <memory>

#include "atlas/Grid.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "mir/lsm/NoneLSM.h"
#include "mir/param/MIRParametrisation.h"


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


Mask::Mask(const std::string &name, const std::string &key):
    name_(name), key_(key) {
}


Mask::~Mask() {
}

//-----------------------------------------------------------------------------

class EmptyLandSeaMask : public Mask {
    virtual bool active() const {
        return false;
    }
    virtual const data::MIRField &field() const {
        NOTIMP;
    }
    virtual std::string uniqueID() const {
        return "<no-lsm>";
    }
    virtual void print(std::ostream &out) const {
        out << name_;
    }
  public:
    EmptyLandSeaMask() : Mask("<no-lsm>", "<no-lsm>") {}
};

//-----------------------------------------------------------------------------

Mask &Mask::lookup(const param::MIRParametrisation  &parametrisation, const atlas::Grid &grid, const std::string& which) {


    std::string name;


    if (!parametrisation.get("lsm" + which, name)) {
        if (!parametrisation.get("lsm", name)) {
            return NoneLSM::instance();
        }
    }


    std::stringstream os;
    os << name << which << "-" << grid.unique_id();
    std::string key = os.str();

    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    eckit::Log::info() << "Mask::lookup(" << key << ")" << std::endl;
    std::map<std::string, Mask *>::iterator j = cache.find(key);

    if(j != cache.end()) {
        return *(*j).second;
    }

    name = name +  which;
    LSMChooser& chooser = LSMChooser::lookup(name);
    Mask* mask = chooser.create(name, key, parametrisation, grid);

    cache[key] = mask;

    return *cache[key];


}

Mask &Mask::lookupInput(const param::MIRParametrisation   &parametrisation, const atlas::Grid &grid) {
    return lookup(parametrisation, grid, ".input");
}


Mask &Mask::lookupOutput(const param::MIRParametrisation   &parametrisation, const atlas::Grid &grid) {
    return lookup(parametrisation, grid, ".output");
}


const data::MIRField& Mask::field() const {
    ASSERT(field_.get());
    return *field_;
}

bool Mask::cacheable() const {
    return true;
}

bool Mask::active() const {
    return true;
}

std::string Mask::uniqueID() const {
    return key_;
}
//-----------------------------------------------------------------------------


}  // namespace logic
}  // namespace mir

