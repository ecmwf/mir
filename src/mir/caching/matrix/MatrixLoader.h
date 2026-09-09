// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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

protected:
    MatrixLoaderFactory(const std::string&);
    virtual ~MatrixLoaderFactory();

public:
    MatrixLoaderFactory(const MatrixLoaderFactory&)            = delete;
    MatrixLoaderFactory(MatrixLoaderFactory&&)                 = delete;
    MatrixLoaderFactory& operator=(const MatrixLoaderFactory&) = delete;
    MatrixLoaderFactory& operator=(MatrixLoaderFactory&&)      = delete;

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
