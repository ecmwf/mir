// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/method/knn/pick/Pick.h"

#include <map>

#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"


namespace mir::method::knn::pick {


static util::recursive_mutex* local_mutex     = nullptr;
static std::map<std::string, PickFactory*>* m = nullptr;
static util::once_flag once;
static void init() {
    local_mutex = new util::recursive_mutex();
    m           = new std::map<std::string, PickFactory*>();
}


Pick::Pick() = default;


Pick::~Pick() = default;


void Pick::distance(const repres::Representation& /*unused*/) const {
    // by default do nothing
}


const std::string& Pick::type() const {
    static const std::string TYPE{"nearest-method"};
    return TYPE;
}


PickFactory::PickFactory(const std::string& name) : name_(name) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    if (m->find(name) == m->end()) {
        (*m)[name] = this;
        return;
    }
    throw exception::SeriousBug("PickFactory: duplicated Pick '" + name + "'");
}


PickFactory::~PickFactory() {
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    m->erase(name_);
}


const Pick* PickFactory::build(const std::string& name, const param::MIRParametrisation& param) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    Log::debug() << "PickFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(Log::error() << "PickFactory: unknown '" << name << "', choices are:\n");
        throw exception::SeriousBug("PickFactory: unknown '" + name + "'");
    }

    return j->second->make(param);
}


void PickFactory::list(std::ostream& out) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    const char* sep = "";
    for (auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


}  // namespace mir::method::knn::pick
