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


#include "mir/param/LibraryConfiguration.h"

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

LibraryConfiguration::LibraryConfiguration():
    library_(LibMir::instance().configuration()) {
}

LibraryConfiguration::~LibraryConfiguration() {
}

void LibraryConfiguration::print(std::ostream &out) const {
    out << "LibraryConfiguration[]";
}

template<class T>
bool LibraryConfiguration::_get(const std::string& name, T& value) const {
    return library_.get(name, value);
}

bool LibraryConfiguration::has(const std::string& name) const {
    MIRParametrisation& cache = cache_;
    return cache.has(name) || parametrisation_.has(name);
}

bool LibraryConfiguration::get(const std::string& name, std::string& value) const {
    return _get(name, value);

}
bool LibraryConfiguration::get(const std::string& name, bool& value) const {
    return _get(name, value);
}

bool LibraryConfiguration::get(const std::string& name, int& value) const {
    return _get(name, value);
}

bool LibraryConfiguration::get(const std::string& name, long& value) const {
    return _get(name, value);
}

bool LibraryConfiguration::get(const std::string& name, float& value) const {
    return _get(name, value);
}

bool LibraryConfiguration::get(const std::string& name, double& value) const {
    return _get(name, value);
}

bool LibraryConfiguration::get(const std::string& name, std::vector<int>& value) const {
    return _get(name, value);
}

bool LibraryConfiguration::get(const std::string& name, std::vector<long>& value) const {
    return _get(name, value);
}

bool LibraryConfiguration::get(const std::string& name, std::vector<float>& value) const {
    return _get(name, value);
}

bool LibraryConfiguration::get(const std::string& name, std::vector<double>& value) const {
    return _get(name, value);
}

bool LibraryConfiguration::get(const std::string& name, std::vector<std::string>& value) const {
    return _get(name, value);
}


}  // namespace param
}  // namespace mir

