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
#include <mutex>

#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir {
namespace packing {


static std::once_flag once;
static std::mutex* local_mutex                  = nullptr;
static std::map<std::string, PackerFactory*>* m = nullptr;
static void init() {
    local_mutex = new std::mutex();
    m           = new std::map<std::string, PackerFactory*>();
}


Packer::Packer(const param::MIRParametrisation&, const param::MIRParametrisation&) {}


Packer::~Packer() = default;


PackerFactory::PackerFactory(const std::string& name) : name_(name) {
    std::call_once(once, init);
    std::lock_guard<std::mutex> lock(*local_mutex);

    if (m->find(name) != m->end()) {
        throw exception::SeriousBug("PackerFactory: duplicate '" + name + "'");
    }

    (*m)[name] = this;
}


PackerFactory::~PackerFactory() {
    std::lock_guard<std::mutex> lock(*local_mutex);

    m->erase(name_);
}


Packer* PackerFactory::build(const std::string& name, const param::MIRParametrisation& user,
                             const param::MIRParametrisation& field) {
    std::call_once(once, init);
    std::lock_guard<std::mutex> lock(*local_mutex);

    Log::debug() << "PackerFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(Log::error() << "PackerFactory: unknown '" << name << "', choices are: ");
        throw exception::SeriousBug("PackerFactory: unknown '" + name + "'");
    }

    return j->second->make(user, field);
}


void PackerFactory::list(std::ostream& out) {
    std::call_once(once, init);
    std::lock_guard<std::mutex> lock(*local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


}  // namespace packing
}  // namespace mir
