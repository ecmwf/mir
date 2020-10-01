/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/packing/Packer.h"

#include <map>

#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"

#include "mir/config/LibMir.h"


namespace mir {
namespace packing {


static pthread_once_t once                      = PTHREAD_ONCE_INIT;
static eckit::Mutex* local_mutex                = nullptr;
static std::map<std::string, PackerFactory*>* m = nullptr;
static void init() {
    local_mutex = new eckit::Mutex();
    m           = new std::map<std::string, PackerFactory*>();
}


Packer::Packer(const param::MIRParametrisation&, const param::MIRParametrisation&) {}


Packer::~Packer() = default;


PackerFactory::PackerFactory(const std::string& name) : name_(name) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    if (m->find(name) != m->end()) {
        throw eckit::SeriousBug("PackerFactory: duplicate '" + name + "'");
    }

    (*m)[name] = this;
}


PackerFactory::~PackerFactory() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    m->erase(name_);
}


Packer* PackerFactory::build(const std::string& name, const param::MIRParametrisation& user,
                             const param::MIRParametrisation& field) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    eckit::Log::debug<LibMir>() << "PackerFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(eckit::Log::error() << "PackerFactory: unknown '" << name << "', choices are: ");
        throw eckit::SeriousBug("PackerFactory: unknown '" + name + "'");
    }

    return j->second->make(user, field);
}


void PackerFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


}  // namespace packing
}  // namespace mir
