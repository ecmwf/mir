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


#include "mir/param/SameParametrisation.h"

#include <ostream>

#include "mir/util/Exceptions.h"


namespace mir::param {


SameParametrisation::SameParametrisation(const MIRParametrisation& parametrisation1,
                                         const MIRParametrisation& parametrisation2, bool strict) :
    parametrisation1_(parametrisation1), parametrisation2_(parametrisation2), strict_(strict) {}


template <class T>
bool SameParametrisation::_get(const std::string& name, T& value) const {

    // This could be a loop
    T value1;
    T value2;
    bool has1 = parametrisation1_.get(name, value1);
    bool has2 = parametrisation2_.get(name, value2);

    if ((!strict_ && has1 != has2) || (has1 && has2 && value1 == value2)) {
        value = has1 ? value1 : value2;
        ASSERT(value == value);
        return true;
    }

    return false;
}


void SameParametrisation::print(std::ostream& out) const {
    out << "SameParametrisation["
           "parametrisation1="
        << parametrisation1_ << ",parametrisation2=" << parametrisation2_ << "]";
}


bool SameParametrisation::has(const std::string& name) const {
    return parametrisation1_.has(name) || parametrisation2_.has(name);
}


bool SameParametrisation::get(const std::string& name, std::string& value) const {
    return _get(name, value);
}


bool SameParametrisation::get(const std::string& name, bool& value) const {
    return _get(name, value);
}

bool SameParametrisation::get(const std::string& name, int& value) const {
    return _get(name, value);
}


bool SameParametrisation::get(const std::string& name, long& value) const {
    return _get(name, value);
}


bool SameParametrisation::get(const std::string& name, float& value) const {
    return _get(name, value);
}


bool SameParametrisation::get(const std::string& name, double& value) const {
    return _get(name, value);
}


bool SameParametrisation::get(const std::string& name, std::vector<int>& value) const {
    return _get(name, value);
}


bool SameParametrisation::get(const std::string& name, std::vector<float>& value) const {
    return _get(name, value);
}


bool SameParametrisation::get(const std::string& name, std::vector<long>& value) const {
    return _get(name, value);
}


bool SameParametrisation::get(const std::string& name, std::vector<double>& value) const {
    return _get(name, value);
}


bool SameParametrisation::get(const std::string& name, std::vector<std::string>& value) const {
    return _get(name, value);
}


}  // namespace mir::param
