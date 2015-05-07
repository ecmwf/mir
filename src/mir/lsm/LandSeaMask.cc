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
#include "mir/param/MIRParametrisation.h"
#include "mir/data/MIRField.h"

#include "atlas/Grid.h"

namespace mir {
namespace lsm {
namespace {


static eckit::Mutex *local_mutex = 0;
static std::map<std::string, LandSeaMaskFactory *> *m = 0;

static std::map<std::string, LandSeaMask *> cache;

static pthread_once_t once = PTHREAD_ONCE_INIT;

static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map<std::string, LandSeaMaskFactory *>();
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

LandSeaMask &LandSeaMask::lookup(const param::MIRParametrisation  &parametrisation, const atlas::Grid &grid, bool input) {

    static EmptyLandSeaMask empty;

    std::string name;

    if (input) {
        if (!parametrisation.get("lsm.input", name)) {
            if (!parametrisation.get("lsm", name)) {
                return empty;
            }
        }
    } else {
        if (!parametrisation.get("lsm.output", name)) {
            if (!parametrisation.get("lsm", name)) {
                return empty;
            }
        }
    }

    std::stringstream os;
    os << name << "-" << (input ? "in-" : "out-") << grid.unique_id();
    std::string key = os.str();

    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    eckit::Log::info() << "LandSeaMask::lookup(" << key << ")" << std::endl;
    std::map<std::string, LandSeaMask *>::iterator j = cache.find(key);

    if(j != cache.end()) {
        return *(*j).second;
    }

    name = name + (input ? ".input" : ".output");
    LandSeaMask* mask = LandSeaMaskFactory::build(name, key, parametrisation, grid);

    cache[key] = mask;

    return *cache[key];


}

LandSeaMask &LandSeaMask::lookupInput(const param::MIRParametrisation   &parametrisation, const atlas::Grid &grid) {
    return lookup(parametrisation, grid, true);
}


LandSeaMask &LandSeaMask::lookupOutput(const param::MIRParametrisation   &parametrisation, const atlas::Grid &grid) {
    return lookup(parametrisation, grid, false);
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


LandSeaMaskFactory::LandSeaMaskFactory(const std::string &name):
    name_(name) {

    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


LandSeaMaskFactory::~LandSeaMaskFactory() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    m->erase(name_);

}


LandSeaMask *LandSeaMaskFactory::build(const std::string &name, const std::string &key, const param::MIRParametrisation &param, const atlas::Grid &grid) {

    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    std::map<std::string, LandSeaMaskFactory *>::const_iterator j = m->find(name);

    eckit::Log::info() << "Looking for LandSeaMaskFactory [" << name << "]" << std::endl;

    if (j == m->end()) {
        eckit::Log::error() << "No LandSeaMaskFactory for [" << name << "]" << std::endl;
        eckit::Log::error() << "LandSeaMaskFactories are:" << std::endl;
        for (j = m->begin() ; j != m->end() ; ++j)
            eckit::Log::error() << "   " << (*j).first << std::endl;
        throw eckit::SeriousBug(std::string("No LandSeaMaskFactory called ") + name);
    }

    return (*j).second->make(name, key, param, grid);
}


}  // namespace logic
}  // namespace mir

