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


#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Once.h"
#include "eckit/thread/Mutex.h"
#include "eckit/exception/Exceptions.h"

#include "mir/lsm/LandSeaMask.h"
#include "mir/lsm/LSMChooser.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/data/MIRField.h"

#include "atlas/Grid.h"

namespace mir {
namespace lsm {
namespace {


static eckit::Mutex *local_mutex = 0;

static std::map<std::string, LandSeaMask *> cache;

static pthread_once_t once = PTHREAD_ONCE_INIT;

static void init() {
    local_mutex = new eckit::Mutex();
}


}  // (anonymous namespace)


LandSeaMask::LandSeaMask(const std::string &name, const std::string &key):
    name_(name), key_(key) {
}


LandSeaMask::~LandSeaMask() {
}

//-----------------------------------------------------------------------------

class EmptyLandSeaMask : public LandSeaMask {
    virtual bool active() const {
        return false;
    }
    virtual const data::MIRField &field() const {
        NOTIMP;
    }
    virtual std::string unique_id() const {
        NOTIMP;
    }
    virtual void print(std::ostream &out) const {
        out << name_;
    }
  public:
    EmptyLandSeaMask() : LandSeaMask("<no-lsm>", "<no-lsm>") {}
};

//-----------------------------------------------------------------------------

LandSeaMask &LandSeaMask::lookup(const param::MIRParametrisation  &parametrisation, const atlas::Grid &grid, const std::string& which) {

    static EmptyLandSeaMask empty;

    std::string name;


    if (!parametrisation.get("lsm" + which, name)) {
        if (!parametrisation.get("lsm", name)) {
            return empty;
        }
    }


    std::stringstream os;
    os << name << which << "-" << grid.unique_id();
    std::string key = os.str();

    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    eckit::Log::info() << "LandSeaMask::lookup(" << key << ")" << std::endl;
    std::map<std::string, LandSeaMask *>::iterator j = cache.find(key);

    if(j != cache.end()) {
        return *(*j).second;
    }

    name = name +  which;
    LSMChooser& chooser = LSMChooser::lookup(name);
    LandSeaMask* mask = chooser.create(name, key, parametrisation, grid);

    cache[key] = mask;

    return *cache[key];


}

LandSeaMask &LandSeaMask::lookupInput(const param::MIRParametrisation   &parametrisation, const atlas::Grid &grid) {
    return lookup(parametrisation, grid, ".input");
}


LandSeaMask &LandSeaMask::lookupOutput(const param::MIRParametrisation   &parametrisation, const atlas::Grid &grid) {
    return lookup(parametrisation, grid, ".output");
}


const data::MIRField& LandSeaMask::field() const {
    ASSERT(field_.get());
    return *field_;
}

bool LandSeaMask::cacheable() const {
    return true;
}

bool LandSeaMask::active() const {
    return true;
}

std::string LandSeaMask::unique_id() const {
    return key_;
}
//-----------------------------------------------------------------------------


}  // namespace logic
}  // namespace mir

