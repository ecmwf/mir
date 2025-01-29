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

#include <ostream>

#include "mir/netcdf/Exceptions.h"
#include "mir/netcdf/Reshape.h"


namespace mir::netcdf {


template <class T>
class Mapper {
public:
    Mapper(std::vector<T>& v, std::vector<bool>& set, bool& overlap);
    Mapper(Mapper<T>& parent, const std::vector<Reshape*>& reshape);

    Mapper(const Mapper&) = delete;
    Mapper(Mapper&&)      = delete;

    ~Mapper();

    void operator=(const Mapper&) = delete;
    void operator=(Mapper&&)      = delete;

    void overlap(bool on) { overlap_ = on; }

    void set(size_t i, T value);

private:
private:
    std::vector<T>& v_;
    std::vector<bool>& set_;
    std::vector<Reshape*> reshapes_;
    bool& overlap_;
};

template <class T>
Mapper<T>::Mapper(std::vector<T>& v, std::vector<bool>& set, bool& overlap) : v_(v), set_(set), overlap_(overlap) {}

template <class T>
Mapper<T>::Mapper(Mapper<T>& parent, const std::vector<Reshape*>& reshapes) :
    v_(parent.v_), set_(parent.set_), overlap_(parent.overlap_) {
    Reshape* r = nullptr;
    for (auto* reshape : reshapes) {
        if (r != nullptr && r->merge(*reshape)) {
            // Pass
        }
        else {
            r = reshape;
            r->attach();
            reshapes_.push_back(r);
        }
    }

    for (auto* reshape : parent.reshapes_) {
        if (r != nullptr && r->merge(*reshape)) {
            // Pass
        }
        else {
            r = reshape;
            r->attach();
            reshapes_.push_back(r);
        }
    }
}


template <class T>
Mapper<T>::~Mapper() {
    for (auto* reshape : reshapes_) {
        reshape->detach();
    }
}


template <class T>
void Mapper<T>::set(size_t i, T v) {

    for (auto* reshape : reshapes_) {
        size_t k = (*reshape)(i);
        ASSERT(k >= i);
        i = k;
    }

    ASSERT(i < v_.size());

    if (!overlap_) {
        if (v_[i] != v) {
            ASSERT(!set_[i]);
        }
    }
    set_[i] = true;

    v_[i] = v;
}


}  // namespace mir::netcdf
