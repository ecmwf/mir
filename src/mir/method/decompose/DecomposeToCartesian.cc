/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Oct 2016


#include "mir/method/decompose/DecomposeToCartesian.h"

#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"
#include "mir/config/LibMir.h"


namespace mir {
namespace method {
namespace decompose {


namespace {


static eckit::Mutex* local_mutex = 0;
static std::map< std::string, DecomposeToCartesian* > *m = 0;
static pthread_once_t once = PTHREAD_ONCE_INIT;


static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map< std::string, DecomposeToCartesian* >();
}


}  // (anonymous namespace)


DecomposeToCartesianChooser::DecomposeToCartesianChooser(const std::string& name, DecomposeToCartesian* choice) :
    name_(name) {
    pthread_once(&once, init);

    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    if (m->find(name) != m->end()) {
        throw eckit::SeriousBug("DecomposeToCartesianChooser: duplicated DecomposeToCartesian '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = choice;
}


DecomposeToCartesianChooser::~DecomposeToCartesianChooser() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    m->erase(name_);
}


const DecomposeToCartesian& DecomposeToCartesianChooser::lookup(const std::string& name) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    eckit::Log::debug<LibMir>() << "DecomposeToCartesianChooser: looking for '" << name << "'" << std::endl;

    std::map< std::string, DecomposeToCartesian* >::const_iterator j = m->find(name);
    if (j == m->end()) {
        eckit::Log::error() << "No DecomposeToCartesianChooser '" << name << "'.";
        throw eckit::SeriousBug("No DecomposeToCartesianChooser '" + name + "'");
    }

    return *(j->second);
}


}  // namespace decompose
}  // namespace method
}  // namespace mir

