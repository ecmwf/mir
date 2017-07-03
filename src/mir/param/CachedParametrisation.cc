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
/// @date Jul 2016


#include "mir/param/CachedParametrisation.h"

// #include "eckit/exception/Exceptions.h"
// #include "eckit/parser/JSON.h"
// #include "eckit/parser/Tokenizer.h"
// #include "eckit/types/Types.h"
// #include "eckit/utils/Translator.h"
// #include "eckit/value/Value.h"
// #include "mir/config/LibMir.h"
// #include "mir/param/DelayedParametrisation.h"


namespace mir {
namespace param {

CachedParametrisation::CachedParametrisation(MIRParametrisation& parametrisation):
    parametrisation_(parametrisation) {
}

CachedParametrisation::~CachedParametrisation() {
}

void CachedParametrisation::print(std::ostream &out) const {
    out << "CachedParametrisation[" << parametrisation_ << "]";
}

template<class T>
bool CachedParametrisation::_get(const std::string& name, T& value) const {
    MIRParametrisation& cache = cache_;

    if(cache.get(name, value)) {
        return true;
    }

    if(parametrisation_.get(name, value)) {
        cache_.set(name, value);
        return true;
    }

    return false;
}


bool CachedParametrisation::has(const std::string& name) const {
    MIRParametrisation& cache = cache_;
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


}  // namespace param
}  // namespace mir

