/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include "mir/param/option/VectorOption.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/utils/Translator.h"
#include "eckit/parser/Tokenizer.h"

#include <iostream>

namespace mir {
namespace param {
namespace option {


template<class T>
VectorOption<T>::VectorOption(const std::string &name, const std::string &description, size_t size):
    Option(name, description), size_(size) {
}

template<class T>
VectorOption<T>::~VectorOption() {
}

template<class T>
void VectorOption<T>::set(const std::string &value, SimpleParametrisation &parametrisation) const {
    eckit::Translator<std::string, T> t;

    eckit::Tokenizer parse("/");
    std::vector<std::string> v;
    parse(value, v);

    std::vector<T> values;
    for(size_t i = 0; i < v.size(); i++) {
        values.push_back(t(v[i]));
    }

    ASSERT(values.size() == size_);
    parametrisation.set(name_, values);
}

template<class T>
void VectorOption<T>::print(std::ostream &out) const {
    out << "   --" << name_;

    const char *sep = "=";
    for(size_t i = 0; i < size_; i++) {
        out << sep  << Title<T>()();
        sep = "/";
    }

    out << " (" << description_ << ")";
}

} // namespace option
}  // namespace param
}  // namespace mir

