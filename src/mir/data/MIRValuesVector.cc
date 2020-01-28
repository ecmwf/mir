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


#include "mir/data/MIRValuesVector.h"

#ifdef VEC_IS_SPECIAL_VECTOR
#include <algorithm>
#include <stdexcept>
#endif


namespace mir {
namespace data {


#ifdef VEC_IS_SPECIAL_VECTOR


template<typename T, typename A>
MIRValuesVectorT<T, A>::MIRValuesVectorT(size_t n) :
    begin_(n ? allocate(n) : nullptr),
    end_(n ? begin_ + n : nullptr),
    endStorage_(end_) {
}


template<typename T, typename A>
MIRValuesVectorT<T, A>::MIRValuesVectorT(size_t n, const value_type& val) :
    begin_(allocate(n)),
    end_(begin_ + n),
    endStorage_(end_) {
    std::uninitialized_fill_n<pointer, size_t>(begin_, n, val);
}


template<typename T, typename A>
MIRValuesVectorT<T, A>::MIRValuesVectorT(const MIRValuesVectorT<T, A>& other) :
    begin_(allocate(other.size())),
    end_(begin_ + other.size()),
    endStorage_(end_) {
    std::memcpy(begin_, other.begin_, other.size() * sizeof(value_type));
}


template<typename T, typename A>
MIRValuesVectorT<T, A>::MIRValuesVectorT(MIRValuesVectorT<T, A>&& other) noexcept :
    begin_(other.begin_),
    end_(other.end_),
    endStorage_(other.endStorage_) {
    other.begin_ = nullptr;
    other.end_ = nullptr;
    other.endStorage_ = nullptr;
}


template<typename T, typename A>
MIRValuesVectorT<T, A>& MIRValuesVectorT<T, A>::operator=(const MIRValuesVectorT<T, A>& other) {
    const size_t n = other.size();
    if (n > capacity()) {
        iterator newStorage = allocate(n);
        deallocate();
        begin_ = newStorage;
        end_ = begin_ + n;
        endStorage_ = end_;
    }
    std::memcpy(begin_, other.begin_, n * sizeof(value_type));
    return *this;
}


template<typename T, typename A>
typename MIRValuesVectorT<T, A>::value_type& MIRValuesVectorT<T, A>::at(size_t index) {
    const size_t s = size();
    if (index < s) {
        return begin_[index];
    }
    throw std::out_of_range("MIRValuesVectorT: access " + std::to_string(index) + " exceeds size " + std::to_string(s));
}


template<typename T, typename A>
const typename MIRValuesVectorT<T, A>::value_type& MIRValuesVectorT<T, A>::at(size_t index) const {
    const size_t s = size();
    if (index < s) {
        return begin_[index];
    }
    throw std::out_of_range("MIRValuesVectorT: access " + std::to_string(index) + " exceeds size " + std::to_string(s));
}


template<typename T, typename A>
void MIRValuesVectorT<T, A>::resize(size_t n) {
    if (capacity() < n) {
        size_t newSize = enlarge_size(n);
        pointer newBegin = allocate(newSize);
        std::memcpy(newBegin, begin_, size() * sizeof(value_type));
        deallocate();
        begin_ = newBegin;
        endStorage_ = begin_ + newSize;
    }
    end_ = begin_ + n;
}


template<typename T, typename A>
void MIRValuesVectorT<T, A>::reserve(size_t n) {
    if (capacity() < n) {
        const size_t curSize = size();
        pointer newBegin = allocate(n);
        std::memcpy(newBegin, begin_, curSize * sizeof(value_type));
        deallocate();
        begin_ = newBegin;
        end_ = newBegin + curSize;
        endStorage_ = begin_ + n;
    }
}


template<typename T, typename A>
void MIRValuesVectorT<T, A>::assign(size_t n, const value_type& val) {
    if (n > capacity()) {
        deallocate();
        begin_ = allocate(n);
        endStorage_ = begin_ + n;
    }
    end_ = begin_ + n;
    std::uninitialized_fill_n<value_type*, size_t>(begin_, n, val);
}


template<typename T, typename A>
void MIRValuesVectorT<T, A>::push_back(const value_type& val) {
    if (end_ == endStorage_) {
        enlarge(enlarge_size(1));
    }
    *end_ = val;
    ++end_;
}


template<typename T, typename A>
void MIRValuesVectorT<T, A>::push_back(value_type&& val) {
    if (end_ == endStorage_) {
        enlarge(enlarge_size(1));
    }
    *end_ = std::move(val);
    ++end_;
}


template<typename T, typename A>
typename MIRValuesVectorT<T, A>::reference MIRValuesVectorT<T, A>::emplace_back(value_type&& val) {
    push_back(val);
    return *end_;
}


template<typename T, typename A>
typename MIRValuesVectorT<T, A>::iterator MIRValuesVectorT<T, A>::insert(const_iterator position, const value_type& val) {
    if (end_ == endStorage_) {
        auto index = size_t(position - begin_);
        enlarge_for_insert(enlarge_size(1), index, 1);
        position = begin_ + index;
    } else {
        auto n = size_t(end_ - position);
        std::memmove(const_cast<iterator>(position) + 1, position, n * sizeof(value_type));
        ++end_;
    }
    *const_cast<iterator>(position) = val;
    return const_cast<iterator>(position);
}


template<typename T, typename A>
typename MIRValuesVectorT<T, A>::iterator MIRValuesVectorT<T, A>::insert(const_iterator position, size_t n, const value_type& val) {
    if (capacity() < size() + n) {
        auto index = size_t(position - begin_);
        enlarge_for_insert(enlarge_size(n), index, n);
        position = begin_ + index;
    } else {
        auto n = size_t(end_ - position);
        std::memmove(const_cast<iterator>(position) + n, position, n * sizeof(value_type));
        end_ += n;
    }
    std::uninitialized_fill_n<pointer, size_t>(const_cast<iterator>(position), n, val);
    return const_cast<iterator>(position);
}


template<typename T, typename A>
void MIRValuesVectorT<T, A>::swap(MIRValuesVectorT<T, A>& other) noexcept {
    std::swap(begin_, other.begin_);
    std::swap(end_, other.end_);
    std::swap(endStorage_, other.endStorage_);
}


template<typename T, typename A>
typename MIRValuesVectorT<T, A>::size_t MIRValuesVectorT<T, A>::enlarge_size(size_t extraSpace) const noexcept {
    return size() + std::max(size(), extraSpace);
}


template<typename T, typename A>
void MIRValuesVectorT<T, A>::enlarge(size_t newSize) {
    pointer newBegin = allocate(newSize);
    std::memcpy(newBegin, begin_, size() * sizeof(value_type));
    deallocate();
    end_ = newBegin + size();
    begin_ = newBegin;
    endStorage_ = begin_ + newSize;
}


template<typename T, typename A>
void MIRValuesVectorT<T, A>::enlarge_for_insert(size_t newSize, size_t position, size_t n) {
    pointer newBegin = allocate(newSize);
    std::memcpy(newBegin, begin_, position * sizeof(value_type));
    std::memcpy(newBegin + position + n, begin_ + position, (size() - position) * sizeof(value_type));
    deallocate();
    end_ = newBegin + size() + n;
    begin_ = newBegin;
    endStorage_ = begin_ + newSize;
}


template<typename T, typename A>
void MIRValuesVectorT<T, A>::print(std::ostream& out) const {
    const char* sep = "";
    for (pointer p = begin_; p != end_; ++p) {
        out << sep << *p;
        sep = ", ";
    }
}


template<typename T, typename A>
MIRValuesVectorT<T, A>& MIRValuesVectorT<T, A>::operator=(MIRValuesVectorT<T, A>&& other) noexcept {
    deallocate();
    begin_ = other.begin_;
    end_ = other.end_;
    endStorage_ = other.endStorage_;
    other.begin_ = nullptr;
    other.end_ = nullptr;
    other.endStorage_ = nullptr;
    return *this;
}


template<typename T, typename A>
typename MIRValuesVectorT<T, A>::pointer MIRValuesVectorT<T, A>::allocate(size_t n) {
#ifdef VEC_IS_SPECIAL_VECTOR_POSIX_MEMALIGN
    pointer p;
    ::posix_memalign((void**)&p,
                     sizeof(value_type) * 64,
                     sizeof(value_type) * n);
    return p;
#else
    return allocator_type::allocate(n);
#endif
}


template<typename T, typename A>
void MIRValuesVectorT<T, A>::deallocate() noexcept {
    if (begin_ != nullptr) {
#ifdef VEC_IS_SPECIAL_VECTOR_POSIX_MEMALIGN
        ::free(begin_);
#else
        allocator_type::deallocate(begin_, capacity());
#endif
}
}


// explicit instantiation
template class MIRValuesVectorT<MIRValuesVector::value_type>;
template class MIRValuesVectorT<std::complex<MIRValuesVector::value_type>>;


#endif


}  // namespace data


}  // namespace mir

