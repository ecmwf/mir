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
#include "mir/config/LibMir.h"
#include "mir/util/Grib.h"
#include "mir/input/GribFileInput.h"

#include "mir/input/MIRInput.h"
#include "eckit/filesystem/PathName.h"

namespace mir {
namespace input {


static eckit::Mutex *local_mutex = 0;
static std::map<std::string, MIRInputFactory *> *m = 0;
static pthread_once_t once = PTHREAD_ONCE_INIT;


static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map<std::string, MIRInputFactory *>();
}


MIRInput::MIRInput() {
}


MIRInput::~MIRInput() {
}


grib_handle *MIRInput::gribHandle(size_t which) const {
     ASSERT(which == 0);
    static grib_handle *handle = 0;
    if (!handle) {
        handle = grib_handle_new_from_samples(0, "GRIB1");
        ASSERT(handle);
    }
    return handle;
}


bool MIRInput::next() {
    std::ostringstream os;
    MIRInput &self = *this;
    os << "MIRInput::next() not implemented for " << self;
    throw eckit::SeriousBug(os.str());
}

size_t MIRInput::copy(double *values, size_t size) const {
    std::ostringstream os;
    os << "MIRInput::copy() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


size_t MIRInput::dimensions() const {
    std::ostringstream os;
    os << "MIRInput::dimensions() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}

//======================================

MIRInputFactory::MIRInputFactory(const std::string &name):
    name_(name) {
    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    if (m->find(name) != m->end()) {
        throw eckit::SeriousBug("MIRInputFactory: duplication factory: " + name);
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


MIRInputFactory::~MIRInputFactory() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    m->erase(name_);

}


MIRInput *MIRInputFactory::build(const std::string& path) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    std::string name = eckit::PathName(path).extension();

    eckit::Log::debug<LibMir>() << "Looking for MIRInputFactory [" << name << "]" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        if (j == m->end()) {
            eckit::Log::error() << "No MIRInputFactory for [" << name << "]" << std::endl;
            eckit::Log::error() << "MIRInputFactory are:" << std::endl;
            for (j = m->begin() ; j != m->end() ; ++j)
                eckit::Log::error() << "   " << (*j).first << std::endl;
            eckit::Log::error() << "MIRInputFactory assuming grib" << std::endl;
            return new GribFileInput(path);
        }
    }

    return (*j).second->make(path);
}


void MIRInputFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    for (std::map<std::string, MIRInputFactory *>::const_iterator j = m->begin() ; j != m->end() ; ++j) {
        out << sep << (*j).first;
        sep = ", ";
    }
}

}  // namespace input
}  // namespace mir

