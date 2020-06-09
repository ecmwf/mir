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


#include "mir/namedgrids/NamedGrid.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"

#include "mir/config/LibMir.h"
#include "mir/namedgrids/NamedGridPattern.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/Representation.h"


namespace mir {
namespace namedgrids {


static std::map<std::string, NamedGrid*>* m = nullptr;
static pthread_once_t once                  = PTHREAD_ONCE_INIT;
static eckit::Mutex* local_mutex            = nullptr;
static void init() {
    local_mutex = new eckit::Mutex();
    m           = new std::map<std::string, NamedGrid*>();
}


namespace {


void read_configuration_files() {
    static bool files_read = false;
    if (files_read) {
        return;
    }
    files_read = true;

    // Read config file, attaching new NamedGrid's grids to parametrisations
    struct NamedGridFromFile : NamedGrid, param::SimpleParametrisation {
        NamedGridFromFile(const std::string& name) : NamedGrid(name) {}
        const repres::Representation* representation() const { return repres::RepresentationFactory::build(*this); }
        const repres::Representation* representation(const util::Rotation&) const { NOTIMP; }
        size_t gaussianNumber() const {
            long N = 64;
            if (!get("gaussianNumber", N)) {
                eckit::Log::warning()
                    << "NamedGridFromFile::gaussianNumber: didn't find key 'gaussianNumber', setting N=" << N
                    << " (hardcoded!)" << std::endl;
            }
            return size_t(N);
        }
        void print(std::ostream& out) const {
            out << "NamedGridFromFile[name=" << name_ << ",parametrisation=";
            SimpleParametrisation::print(out);
            out << "]";
        }
    };

    eckit::PathName path("~mir/etc/mir/grids.yaml");
    if (path.exists()) {
        eckit::Log::debug<LibMir>() << "NamedGrid: reading from '" << path << "'" << std::endl;

        eckit::ValueMap grids = eckit::YAMLParser::decodeFile(path);
        for (const auto& g : grids) {

            // This registers a new NamedGrid (don't delete pointer)
            auto ng = new NamedGridFromFile(g.first);
            ASSERT(ng);

            for (const auto& p : eckit::ValueMap(g.second)) {
                // value type checking prevents lossy conversions (eg. string > double > string > double)
                p.second.isDouble()
                    ? ng->set(p.first, p.second.as<double>())
                    : p.second.isNumber() ? ng->set(p.first, p.second.as<long long>())
                                          : p.second.isBool() ? ng->set(p.first, p.second.as<bool>())
                                                              : ng->set(p.first, p.second.as<std::string>());
            }

            ng->print(eckit::Log::debug<LibMir>());
            eckit::Log::debug<LibMir>() << std::endl;
        }
    }
}

}  // namespace


NamedGrid::NamedGrid(const std::string& name) : name_(name) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


NamedGrid::~NamedGrid() {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    ASSERT(m->find(name_) != m->end());
    m->erase(name_);
}


void NamedGrid::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    auto sep = "";
    for (auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
    out << std::endl;
}


const NamedGrid& NamedGrid::lookup(const std::string& name) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    read_configuration_files();

    eckit::Log::debug<LibMir>() << "NamedGrid: looking for '" << name << "'" << std::endl;

    // Look for specific name matches
    auto j = m->find(name);
    if (j != m->end()) {
        return *(j->second);
    }

    // Look for pattern matchings
    // This will automatically add the new NamedGrid to the map
    auto ng = NamedGridPattern::build(name);
    if (ng != nullptr) {
        return *ng;
    }

    list(eckit::Log::error() << "No NamedGrid '" << name << "', choices are:\n");
    throw eckit::SeriousBug("No NamedGrid '" + name + "'");
}


bool NamedGrid::known(const std::string& name) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    read_configuration_files();

    // Look for specific name matches
    if (m->find(name) != m->end()) {
        return true;
    }

    // Look for pattern matchings
    auto ng           = NamedGridPattern::build(name);
    bool knownPattern = ng != nullptr;
    delete ng;
    return knownPattern;
}


}  // namespace namedgrids
}  // namespace mir
