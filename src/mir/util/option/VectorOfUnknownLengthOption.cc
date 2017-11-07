/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/util/option/VectorOfUnknownLengthOption.h"

#include <iostream>
#include "eckit/config/Configured.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/parser/Tokenizer.h"
#include "eckit/types/Types.h"
#include "eckit/utils/Translator.h"


using namespace eckit;


namespace mir {
namespace util {
namespace option {


template<class T>
VectorOfUnknownLengthOption<T>::VectorOfUnknownLengthOption(const std::string& name, const std::string& description, const char* separator):
    Option(name, description), size_(0), separator_(separator) {
}


template<class T>
VectorOfUnknownLengthOption<T>::~VectorOfUnknownLengthOption() {
}


template<class T>
void VectorOfUnknownLengthOption<T>::set(Configured& parametrisation) const {
    size_ = 0;
    parametrisation.set(name_, std::vector<T>());
}


template<class T>
void VectorOfUnknownLengthOption<T>::set(const std::string& value, Configured& parametrisation) const {
    eckit::Translator<std::string, T> t;

    eckit::Tokenizer parse(separator_);
    std::vector<std::string> v;
    parse(value, v);

    std::vector<T> values;
    for (size_t i = 0; i < v.size(); i++) {
        values.push_back(t(v[i]));
    }

    size_ = values.size();
    parametrisation.set(name_, values);
}


template<class T>
void VectorOfUnknownLengthOption<T>::print(std::ostream& out) const {
    out << "   --" << name_;

    const char *sep = "=";
    for (size_t i = 0; i < size_; i++) {
        out << sep  << Title<T>()();
        sep = separator_;
    }

    out << " (" << description_ << ")";
}


template<class T>
void VectorOfUnknownLengthOption<T>::copy(const Configuration& from, Configured& to) const {
    std::vector<T> v;
    if (from.get(name_, v)) {
        to.set(name_, v);
    }
}


}  // namespace option
}  // namespace util
}  // namespace mir

