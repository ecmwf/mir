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

#include "mir/lsm/LSMChooser.h"

#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/exception/Exceptions.h"
#include "mir/log/MIR.h"

#include <set>

namespace mir {
namespace lsm {
namespace {


static eckit::Mutex *local_mutex = 0;
static std::map<std::string, LSMChooser *> *m = 0;


static pthread_once_t once = PTHREAD_ONCE_INIT;

static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map<std::string, LSMChooser *>();
}


}  // (anonymous namespace)


LSMChooser::LSMChooser(const std::string &name):
    name_(name) {
    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


LSMChooser::~LSMChooser() {
    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    ASSERT(m->find(name_) != m->end());
    m->erase(name_);
}


void LSMChooser::list(std::ostream &out) {
    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);


    std::set<std::string> seen;
    const char *sep = "";
    for (std::map<std::string, LSMChooser *>::const_iterator j = m->begin() ; j != m->end() ; ++j) {
        std::string name = (*j).first.substr(0, (*j).first.find("."));
        if (seen.find(name) == seen.end()) {
            out << sep << name;
            sep = ", ";
            seen.insert(name);
        }
    }
}

const LSMChooser &LSMChooser::lookup(const std::string &name) {

    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    std::map<std::string, LSMChooser *>::const_iterator j = m->find(name);

    eckit::Log::trace<MIR>() << "Looking for LSMChooser [" << name << "]" << std::endl;
    if (j == m->end()) {
        eckit::Log::error() << "No LSMChooser for [" << name << "]" << std::endl;
        eckit::Log::error() << "LSMChoosers are:" << std::endl;
        for (j = m->begin() ; j != m->end() ; ++j)
            eckit::Log::error() << "   " << (*j).first << std::endl;
        throw eckit::SeriousBug(std::string("No LSMChooser called ") + name);
    }


    return *(*j).second;
}


}  // namespace lsm
}  // namespace mir

