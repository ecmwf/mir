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
/// @date Oct 2016


#ifndef mir_caching_matrix_MatrixLoader_h
#define mir_caching_matrix_MatrixLoader_h

#include <iosfwd>

#include "eckit/filesystem/PathName.h"
#include "eckit/linalg/SparseMatrix.h"


namespace eckit {
class Channel;
}


namespace mir {
namespace caching {
namespace matrix {


class MatrixLoader : public eckit::linalg::SparseMatrix::Allocator {

public:
    MatrixLoader(const std::string&, const eckit::PathName&);
    virtual ~MatrixLoader();

    virtual const void* address() const = 0;
    virtual size_t size() const = 0;

    virtual eckit::linalg::SparseMatrix::Layout allocate(eckit::linalg::SparseMatrix::Shape&);

    virtual void deallocate(eckit::linalg::SparseMatrix::Layout, eckit::linalg::SparseMatrix::Shape);

protected:

    static eckit::Channel& log();

    eckit::PathName path_;

};


class MatrixLoaderFactory {
    std::string name_;
    virtual MatrixLoader* make(const std::string& name, const eckit::PathName&) = 0;

protected:
    MatrixLoaderFactory(const std::string&);
    virtual ~MatrixLoaderFactory();

public:
    static MatrixLoader* build(const std::string&, const eckit::PathName&);
    static void list(std::ostream&);
};

template <class T>
class MatrixLoaderBuilder : public MatrixLoaderFactory {
    virtual MatrixLoader* make(const std::string& name, const eckit::PathName& path) {
        return new T(name, path);
    }

public:
    MatrixLoaderBuilder(const std::string& name) : MatrixLoaderFactory(name) {}
};


}  // namespace matrix
}  // namespace caching
}  // namespace mir


#endif

