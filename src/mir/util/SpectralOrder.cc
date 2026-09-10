// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/util/SpectralOrder.h"

#include <map>
#include <sstream>

#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"


namespace mir::util {


long SpectralOrder::getTruncationFromGaussianNumber(long /*unused*/) const {
    std::ostringstream os;
    os << "SpectralOrder::getTruncationFromGaussianNumber() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


long SpectralOrder::getGaussianNumberFromTruncation(long /*unused*/) const {
    std::ostringstream os;
    os << "SpectralOrder::getGaussianNumberFromTruncation() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


static once_flag once;
static recursive_mutex* local_mutex                    = nullptr;
static std::map<std::string, SpectralOrderFactory*>* m = nullptr;
static void init() {
    local_mutex = new recursive_mutex();
    m           = new std::map<std::string, SpectralOrderFactory*>();
}


SpectralOrderFactory::SpectralOrderFactory(const std::string& name) : name_(name) {
    call_once(once, init);
    lock_guard<recursive_mutex> lock(*local_mutex);

    if (m->find(name) != m->end()) {
        throw exception::SeriousBug("SpectralOrderFactory: duplicate '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


SpectralOrderFactory::~SpectralOrderFactory() {
    lock_guard<recursive_mutex> lock(*local_mutex);

    m->erase(name_);
}


SpectralOrder* SpectralOrderFactory::build(const std::string& name) {
    call_once(once, init);
    lock_guard<recursive_mutex> lock(*local_mutex);

    Log::debug() << "SpectralOrderFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(Log::error() << "SpectralOrderFactory: unknown '" << name << "', choices are: ");
        throw exception::SeriousBug("SpectralOrderFactory: unknown '" + name + "'");
    }

    return j->second->make();
}


void SpectralOrderFactory::list(std::ostream& out) {
    call_once(once, init);
    lock_guard<recursive_mutex> lock(*local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


}  // namespace mir::util
