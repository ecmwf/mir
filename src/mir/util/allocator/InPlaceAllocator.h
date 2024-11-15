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


#include <iosfwd>

#include "eckit/linalg/SparseMatrix.h"


namespace mir::util::allocator {


using eckit::linalg::SparseMatrix;
using Index  = eckit::linalg::Index;
using Scalar = eckit::linalg::Scalar;
using Size   = eckit::linalg::Size;


class InPlaceAllocator : public SparseMatrix::Allocator {
public:
    using Layout = SparseMatrix::Layout;
    using Shape  = SparseMatrix::Shape;

    InPlaceAllocator(Size Nr, Size Nc, Size nnz, Index* ia, Index* ja, Scalar* a);

    Layout allocate(Shape&) override;
    void deallocate(Layout, Shape) override;
    void print(std::ostream&) const override;
    bool inSharedMemory() const override;

private:
    const Size Nr_;
    const Size Nc_;
    const Size nnz_;
    Index* ia_;  // NOTE: not owned
    Index* ja_;
    Scalar* a_;
};


}  // namespace mir::util::allocator
