// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/param/CachedParametrisation.h"

#include <ostream>


namespace mir::param {


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


}  // namespace mir::param
