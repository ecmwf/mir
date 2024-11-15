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


#include "mir/util/allocator/InPlaceAllocator.h"

#include "mir/util/Exceptions.h"


namespace mir::util::allocator {


InPlaceAllocator::InPlaceAllocator(Size Nr, Size Nc, Size nnz, Index* ia, Index* ja, Scalar* a) :
    Nr_(Nr), Nc_(Nc), nnz_(nnz), ia_(ia), ja_(ja), a_(a) {
    ASSERT(ia_ != nullptr);
    ASSERT(ja_ != nullptr);
    ASSERT(a_ != nullptr);
}


InPlaceAllocator::Layout InPlaceAllocator::allocate(Shape& shape) {
    shape.size_ = nnz_;
    shape.rows_ = Nr_;
    shape.cols_ = Nc_;

    Layout layout;
    layout.outer_ = reinterpret_cast<decltype(Layout::outer_)>(ia_);
    layout.inner_ = ja_;
    layout.data_  = a_;

    return layout;
}


void InPlaceAllocator::deallocate(Layout, Shape) {}


void InPlaceAllocator::print(std::ostream&) const {
    NOTIMP;
}


bool InPlaceAllocator::inSharedMemory() const {
    return false;
}


}  // namespace mir::util::allocator
