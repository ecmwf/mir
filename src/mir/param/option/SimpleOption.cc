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


#include "mir/param/option/SimpleOption.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/utils/Translator.h"

#include <iostream>

namespace mir {
namespace param {
namespace option {

template<class T> struct Title {
    const char *operator()() const;
};


template<>
const char *Title<size_t>::operator()() const {
    return "ordinal";
}

template<>
const char *Title<double>::operator()() const {
    return "real";
}

template<>
const char *Title<bool>::operator()() const {
    return "0/1";
}

template<>
const char *Title<eckit::PathName>::operator()() const {
    return "path";
}

template<class T>
SimpleOption<T>::SimpleOption(const std::string &name, const std::string &description):
    Option(name, description) {
}

template<class T>
SimpleOption<T>::~SimpleOption() {
}

template<class T>
void SimpleOption<T>::set(const std::string &value, SimpleParametrisation &parametrisation) const {
    T v = eckit::Translator<std::string, T>()(value);
    parametrisation.set(name_, v);
}

template<class T>
void SimpleOption<T>::set(SimpleParametrisation &parametrisation) const {
    Option::set(parametrisation);
}

template<>
void SimpleOption<bool>::set(SimpleParametrisation &parametrisation) const {
    parametrisation.set(name_, true);
}

template<class T>
void SimpleOption<T>::print(std::ostream &out) const {
    out << "   --" << name_ << "=" << Title<T>()() << " (" << description_ << ")";
}

} // namespace option
}  // namespace param
}  // namespace mir

