/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"
#include "mir/config/LibMir.h"
#include "mir/method/decompose/Decompose.h"


namespace mir {
namespace method {
namespace decompose {


namespace {


static eckit::Mutex* local_mutex = 0;
static std::map< std::string, Decompose* > *m = 0;
static pthread_once_t once = PTHREAD_ONCE_INIT;


static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map< std::string, Decompose* >();
}


}  // (anonymous namespace)


Decompose::Decompose() {
}


DecomposeChooser::DecomposeChooser(const std::string& name, Decompose* choice) :
    name_(name) {
    pthread_once(&once, init);

    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    if (m->find(name) != m->end()) {
        throw eckit::SeriousBug("DecomposeChooser: duplicated Decompose '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = choice;
}


DecomposeChooser::~DecomposeChooser() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    m->erase(name_);
}


const Decompose& DecomposeChooser::lookup(const std::string& name) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    eckit::Log::debug<LibMir>() << "DecomposeChooser: looking for '" << name << "'" << std::endl;

    std::map< std::string, Decompose* >::const_iterator j = m->find(name);
    if (j == m->end()) {
        list(eckit::Log::error() << "No DecomposeChooser '" << name << "', choices are:\n");
        throw eckit::SeriousBug("No DecomposeChooser '" + name + "'");
    }

    return *(j->second);
}


void DecomposeChooser::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    std::map< std::string, Decompose* >::const_iterator j;
    for (j = m->begin(); j != m->end(); ++j) {
        out << (*j).first << "\n";
    }
    out << std::endl;
}


}  // namespace decompose
}  // namespace method
}  // namespace mir

