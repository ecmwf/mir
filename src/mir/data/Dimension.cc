/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/data/Dimension.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"
#include "mir/config/LibMir.h"


namespace mir {
namespace data {


Dimension::Dimension() {
}


Dimension::~Dimension() {
}


//=========================================================================


namespace {
static pthread_once_t once = PTHREAD_ONCE_INIT;
static eckit::Mutex* local_mutex = 0;
static std::map< std::string, DimensionChooser* >* m = 0;
static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map< std::string, DimensionChooser* >();
}
}  // (anonymous namespace)


DimensionChooser::DimensionChooser(const std::string& name, Dimension* choice) :
    name_(name),
    choice_(choice) {
    pthread_once(&once, init);
    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    if (m->find(name) != m->end()) {
        throw eckit::SeriousBug("DimensionChooser: duplicate '" + name + "'");
    }

    (*m)[name] = this;
}


DimensionChooser::~DimensionChooser() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    delete choice_;
    m->erase(name_);
}


const Dimension& DimensionChooser::lookup(const std::string& name) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    eckit::Log::debug<LibMir>() << "DimensionChooser: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(eckit::Log::error() << "DimensionChooser: unknown '" << name << "', choices are: ");
        throw eckit::SeriousBug("DimensionChooser: unknown '" + name + "'");
    }

    return *((j->second)->choice_);
}


void DimensionChooser::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


}  // namespace data
}  // namespace mir

