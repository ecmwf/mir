/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// Baudouin Raoult - ECMWF Jan 2015

#include "eckit/exception/Exceptions.h"
#include "eckit/types/Types.h"

#include "mir/netcdf/GridSpec.h"

// #include "mir/netcdf/Attribute.h"
// #include "mir/netcdf/Dimension.h"
// #include "mir/netcdf/Exceptions.h"
#include "mir/netcdf/Variable.h"
// #include "mir/netcdf/HyperCube.h"
#include "mir/netcdf/Dataset.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"

#include <iostream>
#include <sstream>


namespace mir {
namespace netcdf {

static eckit::Mutex* local_mutex = 0;
static std::map<size_t, GridSpecGuesser*>* m = 0;
static pthread_once_t once = PTHREAD_ONCE_INIT;

static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map<size_t, GridSpecGuesser*>();
}


GridSpec::GridSpec(const Variable &variable):
    variable_(variable) {
}

GridSpec::~GridSpec()
{
}

GridSpec* GridSpec::create(const Variable &variable) {

    GridSpec* spec = GridSpecGuesser::guess(variable);

    if (!spec) {
        std::ostringstream oss;
        oss <<  "Cannot guess GridSpec for " << variable;
        throw eckit::SeriousBug(oss.str());
    }

    return spec;

}


//================================================================

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

    for (auto k : dataset.variables()) {
        Variable& v = *(k.second);
        if (v.sharesDimensions(variable) && v.getAttributeValue<std::string>("standard_name") == standardName) {
            std::cout << "XXXXX find_variable" << v << " has standard_name " << standardName << std::endl;
            return v;
        }
    }

    for (auto k : dataset.variables()) {
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



GridSpec* GridSpecGuesser::guess(const Variable &variable) {

    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    std::map<size_t, GridSpecGuesser*>::const_iterator j;

    // We assume lat/lon are the innermost coordinates


    const Variable &latitudes = find_variable(variable,
                                "latitude",
                                "degrees_north",
                                2);

    const Variable &longitudes = find_variable(variable,
                                 "longitude",
                                 "degrees_east",
                                 1);

    for (auto j = m->begin(); j != m->end(); ++j) {
        GridSpec* spec = (*j).second->guess(variable, latitudes, longitudes);
        if (spec) {
            return spec;
        }
    }

    return 0;

}

}
}
