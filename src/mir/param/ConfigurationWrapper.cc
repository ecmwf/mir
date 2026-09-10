// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/param/ConfigurationWrapper.h"

#include <ostream>

#include "eckit/config/Configuration.h"


namespace mir::param {


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


}  // namespace mir::param
