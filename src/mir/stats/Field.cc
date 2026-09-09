// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/stats/Field.h"

#include <cmath>
#include <map>
#include <ostream>

#include "eckit/log/JSON.h"

#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"


namespace mir::stats {


static util::recursive_mutex* local_mutex      = nullptr;
static std::map<std::string, FieldFactory*>* m = nullptr;
static util::once_flag once;
static void init() {
    local_mutex = new util::recursive_mutex();
    m           = new std::map<std::string, FieldFactory*>();
}


Field::Field(const param::MIRParametrisation& /*unused*/) {}


void Field::json_tv(eckit::JSON& j, const std::string& type, double value) {
    j.startObject();
    j << "type" << type;
    if (!std::isnan(value)) {
        j << "value" << value;
    }
    j.endObject();
}


Field::~Field() = default;


FieldFactory::FieldFactory(const std::string& name) : name_(name) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    if (m->find(name) != m->end()) {
        throw exception::SeriousBug("FieldFactory: duplicate '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


FieldFactory::~FieldFactory() {
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    m->erase(name_);
}


void FieldFactory::list(std::ostream& out) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    const char* sep = "";
    for (auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
    out << std::endl;
}


Field* FieldFactory::build(const std::string& name, const param::MIRParametrisation& param) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    Log::debug() << "FieldFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(Log::error() << "FieldFactory: unknown '" << name << "', choices are: ");
        throw exception::SeriousBug("FieldFactory: unknown '" + name + "'");
    }

    return j->second->make(param);
}


}  // namespace mir::stats
