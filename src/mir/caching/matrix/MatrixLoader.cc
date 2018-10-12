/*
 * (C) Copyright 1996- ECMWF.
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


#include "mir/caching/matrix/MatrixLoader.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"

#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace caching {
namespace matrix {


MatrixLoader::MatrixLoader(const std::string&, const eckit::PathName& path) :
    path_(path.realName()) {
}


MatrixLoader::~MatrixLoader() = default;


eckit::linalg::SparseMatrix::Layout MatrixLoader::allocate(eckit::linalg::SparseMatrix::Shape& shape) {
    eckit::linalg::SparseMatrix::Layout layout;
    eckit::linalg::SparseMatrix::load(address(), size(), layout, shape);

    return layout;
}


void MatrixLoader::deallocate(eckit::linalg::SparseMatrix::Layout, eckit::linalg::SparseMatrix::Shape) {
    // We assume that the MatrixLoader is deleted at the same time as the matrix
    // and release the memory in its destructor
}


eckit::Channel& MatrixLoader::log() {
    static auto& channel = eckit::Log::info();
    return channel;
}


//----------------------------------------------------------------------------------------------------------------------


static pthread_once_t once = PTHREAD_ONCE_INIT;
static eckit::Mutex* local_mutex = nullptr;
static std::map< std::string, MatrixLoaderFactory* >* m = nullptr;
static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map< std::string, MatrixLoaderFactory* >();
}


MatrixLoaderFactory::MatrixLoaderFactory(const std::string& name) : name_(name) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    if (m->find(name) != m->end()) {
        throw eckit::SeriousBug("MatrixLoaderFactory: duplicate '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


MatrixLoaderFactory::~MatrixLoaderFactory() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    m->erase(name_);
}


MatrixLoader* MatrixLoaderFactory::build(const std::string& name, const eckit::PathName& path) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    eckit::Log::debug<LibMir>() << "MatrixLoaderFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(eckit::Log::error() << "MatrixLoaderFactory: unknown '" << name << "', choices are: ");
        throw eckit::SeriousBug("MatrixLoaderFactory: unknown '" + name + "'");
    }

    return (*j).second->make(name, path);
}

void MatrixLoaderFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


}  // namespace matrix
}  // namespace caching
}  // namespace mir
