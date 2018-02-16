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
/// @date May 2015


#include "mir/compat/GribCompatibility.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"

#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"

namespace mir {
namespace compat {


static pthread_once_t once = PTHREAD_ONCE_INIT;
static eckit::Mutex *local_mutex = 0;
static std::map< std::string, GribCompatibility* >* m = 0;
static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map< std::string, GribCompatibility* >();
}


GribCompatibility::GribCompatibility(const std::string &name):
    name_(name)  {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;

}


GribCompatibility::~GribCompatibility() {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    ASSERT(m->find(name_)!= m->end());
    m->erase(name_);
}


//=========================================================================



void GribCompatibility::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


const GribCompatibility& GribCompatibility::lookup(const std::string& name) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    eckit::Log::debug<LibMir>() << "GribCompatibility: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(eckit::Log::error() << "GribCompatibility: unknown '" << name << "', choices are: ");
        throw eckit::SeriousBug("GribCompatibility: unknown '" + name + "'");
    }

    return *(*j).second;
}



}  // namespace method
}  // namespace mir

