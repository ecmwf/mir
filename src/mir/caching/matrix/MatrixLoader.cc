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


#include "mir/caching/matrix/MatrixLoader.h"

#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"


namespace mir {
namespace caching {
namespace matrix {


MatrixLoader::MatrixLoader(const std::string& /*unused*/, const eckit::PathName& path) : path_(path.realName()) {}


MatrixLoader::~MatrixLoader() = default;


eckit::linalg::SparseMatrix::Layout MatrixLoader::allocate(eckit::linalg::SparseMatrix::Shape& shape) {
    eckit::linalg::SparseMatrix::Layout layout;
    eckit::linalg::SparseMatrix::load(address(), size(), layout, shape);

    return layout;
}


void MatrixLoader::deallocate(eckit::linalg::SparseMatrix::Layout /*unused*/,
                              eckit::linalg::SparseMatrix::Shape /*unused*/) {
    // We assume that the MatrixLoader is deleted at the same time as the matrix
    // and release the memory in its destructor
}


static util::once_flag once;
static util::recursive_mutex* local_mutex             = nullptr;
static std::map<std::string, MatrixLoaderFactory*>* m = nullptr;
static void init() {
    local_mutex = new util::recursive_mutex();
    m           = new std::map<std::string, MatrixLoaderFactory*>();
}


MatrixLoaderFactory::MatrixLoaderFactory(const std::string& name) : name_(name) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    if (m->find(name) != m->end()) {
        throw exception::SeriousBug("MatrixLoaderFactory: duplicate '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


MatrixLoaderFactory::~MatrixLoaderFactory() {
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);
    m->erase(name_);
}


MatrixLoader* MatrixLoaderFactory::build(const std::string& name, const eckit::PathName& path) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    Log::debug() << "MatrixLoaderFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(Log::error() << "MatrixLoaderFactory: unknown '" << name << "', choices are: ");
        throw exception::SeriousBug("MatrixLoaderFactory: unknown '" + name + "'");
    }

    return j->second->make(name, path);
}


void MatrixLoaderFactory::list(std::ostream& out) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


}  // namespace matrix
}  // namespace caching
}  // namespace mir
