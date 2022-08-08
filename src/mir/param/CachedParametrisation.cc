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


#include "mir/param/CachedParametrisation.h"

#include <ostream>


namespace mir {
namespace param {


CachedParametrisation::CachedParametrisation(MIRParametrisation& parametrisation) : parametrisation_(parametrisation) {}


CachedParametrisation::~CachedParametrisation() = default;


void CachedParametrisation::print(std::ostream& out) const {
    out << "CachedParametrisation[" << parametrisation_ << "]";
}


template <class T>
bool CachedParametrisation::_get(const std::string& name, T& value) const {
    const MIRParametrisation& cache = cache_;

    if (cache.get(name, value)) {
        return true;
    }

    if (parametrisation_.get(name, value)) {
        _set(name, value);
        return true;
    }

    return false;
}


template <class T>
void CachedParametrisation::_set(const std::string& name, const T& value) const {
    cache_.set(name, value);
}


bool CachedParametrisation::has(const std::string& name) const {
    const MIRParametrisation& cache = cache_;
    return cache.has(name) || parametrisation_.has(name);
}


bool CachedParametrisation::get(const std::string& name, std::string& value) const {
    return _get(name, value);
}


bool CachedParametrisation::get(const std::string& name, bool& value) const {
    return _get(name, value);
}


bool CachedParametrisation::get(const std::string& name, int& value) const {
    return _get(name, value);
}


bool CachedParametrisation::get(const std::string& name, long& value) const {
    return _get(name, value);
}


bool CachedParametrisation::get(const std::string& name, float& value) const {
    return _get(name, value);
}


bool CachedParametrisation::get(const std::string& name, double& value) const {
    return _get(name, value);
}


bool CachedParametrisation::get(const std::string& name, std::vector<int>& value) const {
    return _get(name, value);
}


bool CachedParametrisation::get(const std::string& name, std::vector<long>& value) const {
    return _get(name, value);
}


bool CachedParametrisation::get(const std::string& name, std::vector<float>& value) const {
    return _get(name, value);
}


bool CachedParametrisation::get(const std::string& name, std::vector<double>& value) const {
    return _get(name, value);
}


bool CachedParametrisation::get(const std::string& name, std::vector<std::string>& value) const {
    return _get(name, value);
}


void CachedParametrisation::reset() {
    cache_.reset();
}


void CachedParametrisation::set(const std::string& name, bool value) {
    _set(name, value);
}


void CachedParametrisation::set(const std::string& name, int value) {
    _set(name, value);
}


void CachedParametrisation::set(const std::string& name, long value) {
    _set(name, value);
}


void CachedParametrisation::set(const std::string& name, double value) {
    _set(name, value);
}


void CachedParametrisation::set(const std::string& name, const std::string& value) {
    _set(name, value);
}


void CachedParametrisation::set(const std::string& name, const char* value) {
    _set(name, std::string(value));
}


}  // namespace param
}  // namespace mir
