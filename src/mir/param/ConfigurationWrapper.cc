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


#include "mir/param/ConfigurationWrapper.h"

#include <ostream>

#include "eckit/config/Configuration.h"


namespace mir {
namespace param {


ConfigurationWrapper::ConfigurationWrapper(const eckit::Configuration& config) : configuration_(config) {}

const MIRParametrisation& ConfigurationWrapper::userParametrisation() const {
    return *this;
}

const MIRParametrisation& ConfigurationWrapper::fieldParametrisation() const {
    return *this;
}

bool ConfigurationWrapper::has(const std::string& name) const {
    return configuration_.has(name);
}


bool ConfigurationWrapper::get(const std::string& name, std::string& value) const {
    return configuration_.get(name, value);
}


bool ConfigurationWrapper::get(const std::string& name, bool& value) const {
    return configuration_.get(name, value);
}


bool ConfigurationWrapper::get(const std::string& name, int& value) const {
    return configuration_.get(name, value);
}


bool ConfigurationWrapper::get(const std::string& name, long& value) const {
    return configuration_.get(name, value);
}


bool ConfigurationWrapper::get(const std::string& name, float& value) const {
    return configuration_.get(name, value);
}


bool ConfigurationWrapper::get(const std::string& name, double& value) const {
    return configuration_.get(name, value);
}


bool ConfigurationWrapper::get(const std::string& name, std::vector<int>& value) const {
    return configuration_.get(name, value);
}


bool ConfigurationWrapper::get(const std::string& name, std::vector<long>& value) const {
    return configuration_.get(name, value);
}


bool ConfigurationWrapper::get(const std::string& name, std::vector<float>& value) const {
    return configuration_.get(name, value);
}


bool ConfigurationWrapper::get(const std::string& name, std::vector<double>& value) const {
    return configuration_.get(name, value);
}


bool ConfigurationWrapper::get(const std::string& name, std::vector<std::string>& value) const {
    return configuration_.get(name, value);
}


void ConfigurationWrapper::print(std::ostream& s) const {
    s << "ConfigurationWrapper[" << configuration_ << "]";
}


}  // namespace param
}  // namespace mir
