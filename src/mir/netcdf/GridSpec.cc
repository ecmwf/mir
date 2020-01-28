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

#include <iostream>
#include <sstream>

#include "eckit/exception/Exceptions.h"
#include "eckit/types/Types.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"

#include "mir/netcdf/Variable.h"
#include "mir/netcdf/Dataset.h"


namespace mir {
namespace netcdf {

static eckit::Mutex* local_mutex             = nullptr;
static std::map<size_t, GridSpecGuesser*>* m = nullptr;
static pthread_once_t once                   = PTHREAD_ONCE_INIT;

static void init() {
    local_mutex = new eckit::Mutex();
    m           = new std::map<size_t, GridSpecGuesser*>();
}


GridSpec::GridSpec(const Variable &variable):
    variable_(variable) {
}

GridSpec::~GridSpec() = default;

GridSpec* GridSpec::create(const Variable &variable) {

    GridSpec* spec = GridSpecGuesser::guess(variable);

    if (!spec) {
        std::ostringstream oss;
        oss <<  "Cannot guess GridSpec for " << variable;
        throw eckit::SeriousBug(oss.str());
    }

    return spec;

}


GridSpecGuesser::GridSpecGuesser(size_t priority) :
    priority_(priority) {

    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    ASSERT(m->find(priority) == m->end());
    (*m)[priority] = this;
}

GridSpecGuesser::~GridSpecGuesser() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    m->erase(priority_);
}

static const Variable& find_variable(const Variable& variable,
                               const std::string& standardName,
                               const std::string& units,
                               size_t n) {

    const Dataset& dataset = variable.dataset();

    for (const auto& k : dataset.variables()) {
        Variable& v = *(k.second);
        if (v.sharesDimensions(variable) && v.getAttributeValue<std::string>("standard_name") == standardName) {
            std::cout << "XXXXX find_variable" << v << " has standard_name " << standardName << std::endl;
            return v;
        }
    }

    for (const auto& k : dataset.variables()) {
        Variable& v = *(k.second);
        if (v.sharesDimensions(variable) && v.getAttributeValue<std::string>("units") == units) {
            std::cout  << "XXXXX find_variable"  << v << " has units " << units << std::endl;
            return v;
        }
    }

    std::vector<std::string> coordinates = variable.coordinates();
    ASSERT(coordinates.size() >= n);

    const Variable& v = dataset.variable(coordinates[coordinates.size() - n]);
    std::cout  << "XXXXX find_variable"  << v << "is number " << coordinates.size() - n << std::endl;
    return v;

}


GridSpec* GridSpecGuesser::guess(const Variable& variable) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    // We assume lat/lon are the innermost coordinates
    const Variable& latitudes = find_variable(variable, "latitude", "degrees_north", 2);
    const Variable& longitudes = find_variable(variable, "longitude", "degrees_east", 1);

    for (auto& j : *m) {
        GridSpec* spec = j.second->guess(variable, latitudes, longitudes);
        if (spec) {
            eckit::Log::info() << "GRIDSPEC is " << *spec << std::endl;
            return spec;
        }
    }

    return nullptr;
}


}  // namespace netcdf
}  // namespace mir
