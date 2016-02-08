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

#include "mir/packing/Packer.h"

#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/exception/Exceptions.h"
#include "mir/log/MIR.h"


namespace mir {
namespace packing {
namespace {


static eckit::Mutex *local_mutex = 0;
static std::map<std::string, Packer *> *m = 0;


static pthread_once_t once = PTHREAD_ONCE_INIT;

static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map<std::string, Packer *>();
}


}  // (anonymous namespace)


Packer::Packer(const std::string &name):
    name_(name) {
    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


Packer::~Packer() {
    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    ASSERT(m->find(name_) != m->end());
    m->erase(name_);
}


void Packer::list(std::ostream& out) {
    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    for (std::map<std::string, Packer *>::const_iterator j = m->begin() ; j != m->end() ; ++j) {
        out << sep << (*j).first;
        sep = ", ";
    }
}


const Packer& Packer::lookup(const std::string &name) {

    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    std::map<std::string, Packer *>::const_iterator j = m->find(name);

    eckit::Log::trace<MIR>() << "Looking for Packer [" << name << "]" << std::endl;

    if (j == m->end()) {
        eckit::Log::error() << "No Packer for [" << name << "]" << std::endl;
        eckit::Log::error() << "Packers are:" << std::endl;
        for (j = m->begin() ; j != m->end() ; ++j)
            eckit::Log::error() << "   " << (*j).first << std::endl;
        throw eckit::SeriousBug(std::string("No Packer called ") + name);
    }

    return *(*j).second;
}


}  // namespace logic
}  // namespace mir

