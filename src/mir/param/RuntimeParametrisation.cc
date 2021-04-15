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


#include "mir/param/RuntimeParametrisation.h"

#include <ostream>

#include "mir/util/Log.h"


namespace mir {
namespace param {


RuntimeParametrisation::RuntimeParametrisation(const MIRParametrisation& owner) : owner_(owner) {}


RuntimeParametrisation::~RuntimeParametrisation() = default;


void RuntimeParametrisation::print(std::ostream& out) const {
    out << "RuntimeParametrisation[";
    SimpleParametrisation::print(out);
    out << ",owner=[";
    out << owner_;
    out << "]]";
}

template <class T>
void RuntimeParametrisation::_set(const std::string& name, const T& value) {
    Log::debug() << "************* RuntimeParametrisation::set [" << name << "] = [" << value << "]" << std::endl;
    SimpleParametrisation::set(name, value);
}


template <class T>
void RuntimeParametrisation::_set(const std::string& name, const std::vector<T>& value) {
    Log::debug() << "************* RuntimeParametrisation::set [" << name << "] = #" << value.size() << std::endl;
    SimpleParametrisation::set(name, value);
}


MIRParametrisation& RuntimeParametrisation::set(const std::string& name, const std::string& value) {
    _set(name, value);
    return *this;
}


MIRParametrisation& RuntimeParametrisation::set(const std::string& name, const char* value) {
    _set(name, value);
    return *this;
}


MIRParametrisation& RuntimeParametrisation::set(const std::string& name, float value) {
    _set(name, value);
    return *this;
}


MIRParametrisation& RuntimeParametrisation::set(const std::string& name, double value) {
    _set(name, value);
    return *this;
}


MIRParametrisation& RuntimeParametrisation::set(const std::string& name, int value) {
    _set(name, value);
    return *this;
}


MIRParametrisation& RuntimeParametrisation::set(const std::string& name, long value) {
    _set(name, value);
    return *this;
}


MIRParametrisation& RuntimeParametrisation::set(const std::string& name, long long value) {
    _set(name, value);
    return *this;
}


MIRParametrisation& RuntimeParametrisation::set(const std::string& name, bool value) {
    _set(name, value);
    return *this;
}


MIRParametrisation& RuntimeParametrisation::set(const std::string& name, size_t value) {
    _set(name, value);
    return *this;
}


MIRParametrisation& RuntimeParametrisation::set(const std::string& name, const std::vector<int>& value) {
    _set(name, value);
    return *this;
}


MIRParametrisation& RuntimeParametrisation::set(const std::string& name, const std::vector<long>& value) {
    _set(name, value);
    return *this;
}


MIRParametrisation& RuntimeParametrisation::set(const std::string& name, const std::vector<long long>& value) {
    _set(name, value);
    return *this;
}


MIRParametrisation& RuntimeParametrisation::set(const std::string& name, const std::vector<size_t>& value) {
    _set(name, value);
    return *this;
}


MIRParametrisation& RuntimeParametrisation::set(const std::string& name, const std::vector<float>& value) {
    _set(name, value);
    return *this;
}


MIRParametrisation& RuntimeParametrisation::set(const std::string& name, const std::vector<double>& value) {
    _set(name, value);
    return *this;
}


MIRParametrisation& RuntimeParametrisation::set(const std::string& name, const std::vector<std::string>& value) {
    _set(name, value);
    return *this;
}


void RuntimeParametrisation::unset(const std::string& name) {
    hidden_.insert(name);
}


bool RuntimeParametrisation::has(const std::string& name) const {

    if (hidden_.find(name) != hidden_.end()) {
        return false;
    }

    return SimpleParametrisation::has(name) || owner_.has(name);
}


const MIRParametrisation& RuntimeParametrisation::userParametrisation() const {
    return *this;
}


const MIRParametrisation& RuntimeParametrisation::fieldParametrisation() const {
    return *this;
}


template <class T>
bool RuntimeParametrisation::_get(const std::string& name, T& value) const {

    if (hidden_.find(name) != hidden_.end()) {
        return false;
    }

    return SimpleParametrisation::get(name, value) || owner_.get(name, value);
}


bool RuntimeParametrisation::get(const std::string& name, std::string& value) const {
    return _get(name, value);
}


bool RuntimeParametrisation::get(const std::string& name, bool& value) const {
    return _get(name, value);
}


bool RuntimeParametrisation::get(const std::string& name, int& value) const {
    return _get(name, value);
}


bool RuntimeParametrisation::get(const std::string& name, long& value) const {
    return _get(name, value);
}


bool RuntimeParametrisation::get(const std::string& name, float& value) const {
    return _get(name, value);
}


bool RuntimeParametrisation::get(const std::string& name, double& value) const {
    return _get(name, value);
}


bool RuntimeParametrisation::get(const std::string& name, std::vector<int>& value) const {
    return _get(name, value);
}


bool RuntimeParametrisation::get(const std::string& name, std::vector<long>& value) const {
    return _get(name, value);
}


bool RuntimeParametrisation::get(const std::string& name, std::vector<float>& value) const {
    return _get(name, value);
}


bool RuntimeParametrisation::get(const std::string& name, std::vector<double>& value) const {
    return _get(name, value);
}


bool RuntimeParametrisation::get(const std::string& name, std::vector<std::string>& value) const {
    return _get(name, value);
}


}  // namespace param
}  // namespace mir
