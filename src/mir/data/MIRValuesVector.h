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

#define VEC_IS_STL_VECTOR
//#define VEC_IS_SPECIAL_VECTOR
//#define VEC_IS_SPECIAL_VECTOR_POSIX_MEMALIGN


#ifdef VEC_IS_STL_VECTOR
#include <vector>
#endif


#ifdef VEC_IS_SPECIAL_VECTOR
#include <complex>
#include <cstring>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>
#ifdef VEC_IS_SPECIAL_VECTOR_POSIX_MEMALIGN
#include <stdlib.h>
#endif
#endif


namespace mir {
namespace data {


#ifdef VEC_IS_SPECIAL_VECTOR


template <class T>
struct is_complex_or_floating_point_t : std::is_floating_point<T> {};


template <class T>
struct is_complex_or_floating_point_t<std::complex<T>> : std::is_floating_point<T> {};


template <typename T, typename A = std::allocator<T>>
class MIRValuesVectorT : private A {

public:
    static_assert(std::is_standard_layout<T>::value && is_complex_or_floating_point_t<T>::value,
                  "MIRValuesVectorT: supports only trivial floating point types");

    using value_type     = T;
    using allocator_type = A;

    using reference       = value_type&;
    using const_reference = const value_type&;
    using pointer         = value_type*;
    using const_pointer   = const value_type*;
    using difference_type = std::ptrdiff_t;
    using size_t          = std::size_t;
    using size_type       = std::size_t;

    using iterator               = value_type*;
    using const_iterator         = const value_type*;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
    pointer begin_;
    pointer end_;
    pointer endStorage_;

public:
    /// Construct a vector with given amount of elements, without initializing
    explicit MIRValuesVectorT(size_t n = 0);

    /// Construct a vector with given amount of initialised elements
    MIRValuesVectorT(size_t, const value_type&);

    /// Construct a vector by copying element range
    template <class It>
    MIRValuesVectorT(It first, It last) {
        size_t n         = std::distance(first, last);
        begin_           = allocate(n);
        end_             = begin_ + n;
        endStorage_      = begin_ + n;
        pointer destIter = begin_;
        while (first != last) {
            *destIter = *first;
            ++destIter;
            ++first;
        }
    }

    /// Construct a vector by copying another
    MIRValuesVectorT(const MIRValuesVectorT<T, A>&);

    /// Construct a vector by moving another
    MIRValuesVectorT(MIRValuesVectorT<T, A>&&) noexcept;

    /// Destructor
    ~MIRValuesVectorT() noexcept { deallocate(); }

    /// Assign operator (copy)
    MIRValuesVectorT& operator=(const MIRValuesVectorT<T, A>&);

    /// Assign operator (move)
    MIRValuesVectorT& operator=(MIRValuesVectorT<T, A>&&) noexcept;

    // iterators
    iterator begin() noexcept { return begin_; }
    iterator end() noexcept { return end_; }
    const_iterator begin() const noexcept { return begin_; }
    const_iterator end() const noexcept { return end_; }
    const_iterator cbegin() const noexcept { return begin_; }
    const_iterator cend() const noexcept { return end_; }

    reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
    const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end()); }
    const_reverse_iterator crend() const noexcept { return const_reverse_iterator(begin()); }

    // element access
    value_type& operator[](size_t index) noexcept { return begin_[index]; }
    const value_type& operator[](size_t index) const noexcept { return begin_[index]; }

    value_type& at(size_t);
    const value_type& at(size_t) const;

    value_type* data() noexcept { return begin_; }
    const value_type* data() const noexcept { return begin_; }

    /// Container number of elements
    size_t size() const noexcept { return end_ - begin_; }

    /// Container capacity
    size_t capacity() const noexcept { return endStorage_ - begin_; }

    /// Container empty test
    bool empty() const noexcept { return begin_ == end_; }

    /// Empty container
    void clear() { end_ = begin_; }

    /// Resize container, reallocating (without initializing) if new size is above capacity
    /// @note invalidates iterators
    void resize(size_t);

    /// Resize capacity, reallocating (without initializing) if new size is above capacity
    /// @note invalidates iterators
    void reserve(size_t);

    /// Resize container and assign range of values
    /// @note invalidates iterators
    template <class It>
    void assign(It first, It last) {
        size_t n = std::distance(first, last);
        if (n > capacity()) {
            deallocate();
            begin_      = allocate(n);
            endStorage_ = begin_ + n;
        }
        end_             = begin_ + n;
        pointer destIter = begin_;
        while (first != last) {
            *destIter = *first;
            ++destIter;
            ++first;
        }
    }

    /// Resize container and assign the given value to all elements
    /// @note invalidates iterators
    void assign(size_t, const value_type&);

    /// Copy element to the end
    /// @note invalidates iterators
    void push_back(const value_type&);

    /// Move element to the end
    /// @note invalidates iterators
    void push_back(value_type&&);

    /// Move element to the end
    /// @note invalidates iterators
    reference emplace_back(value_type&&);

    /// Insert element at a given position
    /// @note invalidates iterators
    iterator insert(const_iterator, const value_type&);

    /// Insert n copies of element at a given position
    /// @note invalidates iterators
    iterator insert(const_iterator, size_t, const value_type&);

    /// Insert range of values at a given position
    /// @note invalidates iterators
    template <class It>
    iterator insert(const_iterator position, It first, It last) {
        size_t n = std::distance(first, last);
        if (capacity() < size() + n) {
            size_t index = position - begin_;
            enlarge_for_insert(enlarge_size(n), index, n);
            position = begin_ + index;
        }
        else {
            auto n = size_t(end_ - position);
            std::memmove(const_cast<iterator>(position) + n, position, n * sizeof(value_type));
            end_ += n;
        }
        auto destIter = const_cast<iterator>(position);
        while (first != last) {
            *destIter = *first;
            ++destIter;
            ++first;
        }
        return const_cast<iterator>(position);
    }

    /// Swap container, by consequence also swapping any associated iterators
    void swap(MIRValuesVectorT<value_type, allocator_type>&) noexcept;

private:
    pointer allocate(size_t n);

    void deallocate() noexcept;

    size_t enlarge_size(size_t extraSpace) const noexcept;

    void enlarge(size_t newSize);

    void enlarge_for_insert(size_t newSize, size_t position, size_t n);

    void print(std::ostream&) const;

    friend std::ostream& operator<<(std::ostream& out, const MIRValuesVectorT& p) {
        p.print(out);
        return out;
    }
};


// container element definition
using MIRValuesVector = MIRValuesVectorT<double>;


// explicit instantiation
extern template class MIRValuesVectorT<MIRValuesVector::value_type>;
extern template class MIRValuesVectorT<std::complex<MIRValuesVector::value_type>>;
#endif


#ifdef VEC_IS_STL_VECTOR
template <typename T>
using MIRValuesVectorT = std::vector<T>;

// container element definition
using MIRValuesVector = MIRValuesVectorT<double>;
#endif


}  // namespace data


using data::MIRValuesVector;
using data::MIRValuesVectorT;


}  // namespace mir
