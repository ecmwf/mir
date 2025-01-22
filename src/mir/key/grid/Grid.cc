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
#include <sstream>

#include "eckit/filesystem/PathName.h"
#include "eckit/parser/YAMLParser.h"

#include "mir/config/LibMir.h"
#include "mir/key/grid/GridPattern.h"
#include "mir/key/grid/NamedFromFile.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"
#include "mir/util/ValueMap.h"


namespace mir::key::grid {


static util::once_flag once;
static std::map<std::string, Grid*>* m    = nullptr;
static util::recursive_mutex* local_mutex = nullptr;
static void init() {
    local_mutex = new util::recursive_mutex();
    m           = new std::map<std::string, Grid*>();
}


static void read_configuration_files() {
    static bool files_read = false;
    if (files_read) {
        return;
    }
    files_read = true;

    // Read config file, attaching new Grid's grids to parametrisations
    const auto path = LibMir::configFile(LibMir::config_file::GRIDS);
    if (path.exists()) {
        Log::debug() << "Grid: reading from '" << path << "'" << std::endl;

        util::ValueMap grids(eckit::YAMLParser::decodeFile(path));
        for (const auto& g : grids) {

            // This registers a new Grid (don't delete pointer)
            auto* ng = new NamedFromFile(g.first);
            ASSERT(ng);

            util::ValueMap map(g.second);
            map.set(*ng);

            Log::debug() << static_cast<const Grid&>(*ng) << std::endl;
        }
    }
}


Grid::Grid(const std::string& key, const std::string& type) : key_(key), type_(type) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(local_mutex);

    ASSERT(m->insert({key, this}).second);
}


Grid::~Grid() {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(local_mutex);

    ASSERT(m->find(key_) != m->end());
    m->erase(key_);
}


void Grid::list(std::ostream& out) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    const auto* sep = "";
    for (auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
    out << std::endl;
}


const repres::Representation* Grid::representation() const {
    std::ostringstream os;
    os << "Grid::representation() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


const repres::Representation* Grid::representation(const util::Rotation& /*unused*/) const {
    std::ostringstream os;
    os << "Grid::representation(Rotation&) not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


const repres::Representation* Grid::representation(const param::MIRParametrisation& /*unused*/) const {
    std::ostringstream os;
    os << "Grid::representation(MIRParametrisation&) not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Grid::parametrisation(const std::string& /*unused*/, param::SimpleParametrisation& /*unused*/) const {
    std::ostringstream os;
    os << "Grid::parametrisation() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


size_t Grid::gaussianNumber() const {
    std::ostringstream os;
    os << "Grid::gaussianNumber() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


bool Grid::get(const std::string& key, std::string& value, const param::MIRParametrisation& param) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    read_configuration_files();

    std::string grid;
    if (!param.userParametrisation().get(key, grid)) {
        return false;
    }

    // Look for specific key matches
    auto find = m->find(grid);
    if (find != m->end()) {
        value = find->first;
        return true;
    }

    // Look for pattern matchings
    value = GridPattern::match(grid, param);
    return !value.empty();
}


const Grid& Grid::lookup(const std::string& key, const param::MIRParametrisation& param) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    read_configuration_files();

    Log::debug() << "Grid: looking for '" << key << "'" << std::endl;

    // Look for specific key matches
    auto j = m->find(key);
    if (j != m->end()) {
        return *(j->second);
    }

    // Look for pattern matchings
    // This will automatically add the new Grid to the map
    auto match = GridPattern::match(key, param);
    if (!match.empty()) {
        const auto* gp = GridPattern::lookup(match);
        ASSERT(gp != nullptr);
        return *gp;
    }

    list(Log::error() << "Grid: unknown '" << key << "', choices are:\n");
    throw exception::SeriousBug("Grid: unknown '" + key + "'");
}


}  // namespace mir::key::grid
