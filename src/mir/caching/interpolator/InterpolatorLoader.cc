/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @author Tiago Quintino
///
/// @date Oct 2016


#include "mir/caching/interpolator/InterpolatorLoader.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"

#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace caching {
namespace interpolator {


InterpolatorLoader::InterpolatorLoader(const std::string&, const eckit::PathName& path) :
    path_(path.realName())
{
}


InterpolatorLoader::~InterpolatorLoader() {
}


eckit::linalg::SparseMatrix::Layout InterpolatorLoader::allocate(eckit::linalg::SparseMatrix::Shape& shape) {
    eckit::linalg::SparseMatrix::Layout layout;
    eckit::linalg::SparseMatrix::load(address(), size(), layout, shape);

    return layout;
}

void InterpolatorLoader::deallocate(eckit::linalg::SparseMatrix::Layout, eckit::linalg::SparseMatrix::Shape) {
    // We assume that the InterpolatorLoader is deleted at the same time as the matrix
    // and release the memory in its destructor
}


//=========================================================================


namespace {
static pthread_once_t once = PTHREAD_ONCE_INIT;
static eckit::Mutex* local_mutex = 0;
static std::map< std::string, InterpolatorLoaderFactory* >* m = 0;
static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map< std::string, InterpolatorLoaderFactory* >();
}
}  // (anonymous namespace)


InterpolatorLoaderFactory::InterpolatorLoaderFactory(const std::string& name) : name_(name) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    if (m->find(name) != m->end()) {
        throw eckit::SeriousBug("InterpolatorLoaderFactory: duplicate '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


InterpolatorLoaderFactory::~InterpolatorLoaderFactory() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    m->erase(name_);
}


InterpolatorLoader* InterpolatorLoaderFactory::build(const std::string& name, const eckit::PathName& path) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    eckit::Log::debug<LibMir>() << "InterpolatorLoaderFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(eckit::Log::error() << "InterpolatorLoaderFactory: unknown '" << name << "', choices are: ");
        throw eckit::SeriousBug("InterpolatorLoaderFactory: unknown '" + name + "'");
    }

    return (*j).second->make(name, path);
}

void InterpolatorLoaderFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


}  // namespace interpolator
}  // namespace caching
}  // namespace mir
