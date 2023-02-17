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


#pragma once

#include <iosfwd>

#include "eckit/filesystem/PathName.h"
#include "eckit/linalg/SparseMatrix.h"


namespace mir::caching::matrix {


class MatrixLoader : public eckit::linalg::SparseMatrix::Allocator {

public:
    MatrixLoader(const std::string&, const eckit::PathName&);
    ~MatrixLoader() override;

    MatrixLoader(const MatrixLoader&)            = delete;
    MatrixLoader& operator=(const MatrixLoader&) = delete;

    virtual const void* address() const = 0;
    virtual size_t size() const         = 0;

    eckit::linalg::SparseMatrix::Layout allocate(eckit::linalg::SparseMatrix::Shape&) override;

    void deallocate(eckit::linalg::SparseMatrix::Layout, eckit::linalg::SparseMatrix::Shape) override;

protected:
    eckit::PathName path_;
};


class MatrixLoaderFactory {
    std::string name_;
    virtual MatrixLoader* make(const std::string& name, const eckit::PathName&) = 0;

    MatrixLoaderFactory(const MatrixLoaderFactory&)            = delete;
    MatrixLoaderFactory& operator=(const MatrixLoaderFactory&) = delete;

protected:
    MatrixLoaderFactory(const std::string&);
    virtual ~MatrixLoaderFactory();

public:
    static MatrixLoader* build(const std::string&, const eckit::PathName&);
    static void list(std::ostream&);
};


template <class T>
class MatrixLoaderBuilder : public MatrixLoaderFactory {
    MatrixLoader* make(const std::string& name, const eckit::PathName& path) override { return new T(name, path); }

public:
    MatrixLoaderBuilder(const std::string& name) : MatrixLoaderFactory(name) {}
};


}  // namespace mir::caching::matrix
