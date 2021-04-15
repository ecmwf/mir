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


#include "mir/netcdf/GridSpec.h"

#include <mutex>
#include <ostream>
#include <sstream>

#include "mir/netcdf/Dataset.h"
#include "mir/netcdf/Variable.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir {
namespace netcdf {


static std::mutex* local_mutex               = nullptr;
static std::map<size_t, GridSpecGuesser*>* m = nullptr;
static std::once_flag once;
static void init() {
    local_mutex = new std::mutex();
    m           = new std::map<size_t, GridSpecGuesser*>();
}


GridSpec::GridSpec(const Variable& variable) : variable_(variable) {}


GridSpec::~GridSpec() = default;


GridSpec* GridSpec::create(const Variable& variable) {

    auto spec = GridSpecGuesser::guess(variable);
    if (spec == nullptr) {
        std::ostringstream oss;
        oss << "Cannot guess GridSpec for " << variable;
        throw exception::SeriousBug(oss.str());
    }

    return spec;
}


GridSpecGuesser::GridSpecGuesser(size_t priority) : priority_(priority) {
    std::call_once(once, init);
    std::lock_guard<std::mutex> lock(*local_mutex);

    ASSERT(m->find(priority) == m->end());
    (*m)[priority] = this;
}


GridSpecGuesser::~GridSpecGuesser() {
    std::lock_guard<std::mutex> lock(*local_mutex);

    m->erase(priority_);
}

GridSpec* GridSpecGuesser::guess(const Variable& variable) {
    std::call_once(once, init);
    std::lock_guard<std::mutex> lock(*local_mutex);

    // We assume lat/lon are the innermost coordinates
    const Variable& latitudes  = variable.lookupInDataset("latitude", "degrees_north", 2);
    const Variable& longitudes = variable.lookupInDataset("longitude", "degrees_east", 1);

    for (auto& j : *m) {
        auto spec = j.second->guess(variable, latitudes, longitudes);
        if (spec != nullptr) {
            Log::info() << "GRIDSPEC is " << *spec << std::endl;
            return spec;
        }
    }

    return nullptr;
}


}  // namespace netcdf
}  // namespace mir
