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


#include <iostream>

#include "eckit/log/Log.h"

#include "mir/param/RuntimeParametrisation.h"


namespace mir {
namespace param {


RuntimeParametrisation::RuntimeParametrisation(const MIRParametrisation& owner):
    owner_(owner) {
}


RuntimeParametrisation::~RuntimeParametrisation() {
}


void RuntimeParametrisation::print(std::ostream& out) const {
    out << "RuntimeParametrisation[";
    SimpleParametrisation::print(out);
    out << ",owner=[";
    out << owner_;
    out << "]]";
}

template<class T>
void RuntimeParametrisation::_set(const std::string& name, const T& value) {
    eckit::Log::info() << "************* RuntimeParametrisation::set [" << name << "] = [" << value << "]" << std::endl;
    SimpleParametrisation::set(name, value);
}

RuntimeParametrisation& RuntimeParametrisation::set(const std::string& name, const char* value) {
    _set(name, value);
    return *this;
}

RuntimeParametrisation& RuntimeParametrisation::set(const std::string& name, const std::string& value) {
    _set(name, value);
    return *this;
}

RuntimeParametrisation& RuntimeParametrisation::set(const std::string& name, bool value) {
    _set(name, value);
    return *this;
}

RuntimeParametrisation& RuntimeParametrisation::set(const std::string& name, long value) {
    _set(name, value);
    return *this;
}

RuntimeParametrisation& RuntimeParametrisation::set(const std::string& name, double value) {
    _set(name, value);
    return *this;
}



bool RuntimeParametrisation::has(const std::string& name) const {
    if(SimpleParametrisation::has(name)) {
        return true;
    }
    return owner_.has(name);
}



template<class T>
bool RuntimeParametrisation::_get(const std::string& name,  T& value) const {

    if (name.find("user.") == 0) {
        return _get(name.substr(5), value);
    }

    if(SimpleParametrisation::get(name, value)) {
        return true;
    }
    return owner_.get(name, value);
}

bool RuntimeParametrisation::get(const std::string& name, std::string& value) const {
    return _get(name, value);
}

bool RuntimeParametrisation::get(const std::string& name, bool& value) const {
    return _get(name, value);
}

bool RuntimeParametrisation::get(const std::string& name, long& value) const {
    return _get(name, value);
}

bool RuntimeParametrisation::get(const std::string& name, double& value) const {
    return _get(name, value);
}

bool RuntimeParametrisation::get(const std::string& name, std::vector<long>& value) const {
    return _get(name, value);
}

bool RuntimeParametrisation::get(const std::string& name, std::vector<double>& value) const {
    return _get(name, value);
}



}  // namespace param
}  // namespace mir

