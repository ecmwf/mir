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


#include "mir/key/grid/Grid.h"

#include <map>

#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"

#include "mir/config/LibMir.h"
#include "mir/key/grid/NamedFromFile.h"
#include "mir/key/grid/NamedGridPattern.h"


namespace mir {
namespace key {
namespace grid {


static std::map<std::string, Grid*>* m = nullptr;
static pthread_once_t once             = PTHREAD_ONCE_INIT;
static eckit::Mutex* local_mutex       = nullptr;
static void init() {
    local_mutex = new eckit::Mutex();
    m           = new std::map<std::string, Grid*>();
}


static void read_configuration_files() {
    static bool files_read = false;
    if (files_read) {
        return;
    }
    files_read = true;

    // Read config file, attaching new Grid's grids to parametrisations
    eckit::PathName path("~mir/etc/mir/grids.yaml");
    if (path.exists()) {
        eckit::Log::debug<LibMir>() << "Grid: reading from '" << path << "'" << std::endl;

        eckit::ValueMap grids = eckit::YAMLParser::decodeFile(path);
        for (const auto& g : grids) {

            // This registers a new Grid (don't delete pointer)
            auto ng = new NamedFromFile(g.first);
            ASSERT(ng);

            for (const auto& p : eckit::ValueMap(g.second)) {
                // value type checking prevents lossy conversions (eg. string > double > string > double)
                p.second.isDouble()
                    ? ng->set(p.first, p.second.as<double>())
                    : p.second.isNumber() ? ng->set(p.first, p.second.as<long long>())
                                          : p.second.isBool() ? ng->set(p.first, p.second.as<bool>())
                                                              : ng->set(p.first, p.second.as<std::string>());
            }

            eckit::Log::debug<LibMir>() << ng << std::endl;
        }
    }
}


Grid::Grid(const std::string& key) : key_(key) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    ASSERT(m->find(key) == m->end());
    (*m)[key] = this;
}


Grid::~Grid() {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    ASSERT(m->find(key_) != m->end());
    m->erase(key_);
}


void Grid::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    auto sep = "";
    for (auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
    out << std::endl;
}


const Grid& Grid::lookup(const std::string& key) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    read_configuration_files();

    eckit::Log::debug<LibMir>() << "Grid: looking for '" << key << "'" << std::endl;

    // Look for specific key matches
    auto j = m->find(key);
    if (j != m->end()) {
        return *(j->second);
    }

    // Look for pattern matchings
    // This will automatically add the new Grid to the map
    auto ng = NamedGridPattern::build(key);
    if (ng != nullptr) {
        return *ng;
    }

    list(eckit::Log::error() << "No Grid '" << key << "', choices are:\n");
    throw eckit::SeriousBug("No Grid '" + key + "'");
}


bool Grid::known(const std::string& key) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    read_configuration_files();

    // Look for specific key matches
    if (m->find(key) != m->end()) {
        return true;
    }

    // Look for pattern matchings
    auto ng           = NamedGridPattern::build(key);
    bool knownPattern = ng != nullptr;
    delete ng;
    return knownPattern;
}


}  // namespace grid
}  // namespace key
}  // namespace mir
