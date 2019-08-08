/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/knn/pick/PickWithLSM.h"

#include <map>

#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"
#include "eckit/utils/MD5.h"

#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/lsm/LandSeaMasks.h"


namespace mir {
namespace method {
namespace knn {
namespace pick {


static eckit::Mutex* local_mutex = nullptr;
static std::map< std::string, PickWithLSMFactory* > *m = nullptr;
static pthread_once_t once = PTHREAD_ONCE_INIT;


static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map< std::string, PickWithLSMFactory* >();
}


PickWithLSM::PickWithLSM(const param::MIRParametrisation& param, const lsm::LandSeaMasks& lsm) :
    imask_(lsm.inputMask()),
    omask_(lsm.outputMask()) {
    ASSERT(!imask_.empty());
    ASSERT(!omask_.empty());
}


bool PickWithLSM::sameAs(const Pick& other) const {
    // Note: LSM's themselves are not used for this distinction!

    auto o = dynamic_cast<const PickWithLSM*>(&other);
    return o;
}


void PickWithLSM::hash(eckit::MD5& h) const {
    // Note: LSM's themselves are not used for this distinction!

    std::ostringstream s;
    s << *this;
    h.add(s.str());    
}


PickWithLSMFactory::PickWithLSMFactory(const std::string& name) :
    name_(name) {
    pthread_once(&once, init);
    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    if (m->find(name) != m->end()) {
        throw eckit::SeriousBug("PickWithLSMFactory: duplicated PickWithLSM '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


PickWithLSMFactory::~PickWithLSMFactory() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    m->erase(name_);
}


const PickWithLSM* PickWithLSMFactory::build(const std::string& name, const param::MIRParametrisation& param, const lsm::LandSeaMasks& lsm) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    eckit::Log::debug<LibMir>() << "PickWithLSMFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(eckit::Log::error() << "No PickWithLSMFactory '" << name << "', choices are:\n");
        throw eckit::SeriousBug("No PickWithLSMFactory '" + name + "'");
    }

    return j->second->make(param, lsm);
}


void PickWithLSMFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    for (auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


bool PickWithLSMFactory::has(const std::string& name) {
    return m->find(name) != m->end();
}


}  // namespace pick
}  // namespace knn
}  // namespace method
}  // namespace mir

