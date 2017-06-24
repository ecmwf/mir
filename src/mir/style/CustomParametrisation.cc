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


#include "mir/style/CustomParametrisation.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/config/LibMir.h"


namespace mir {
namespace style {


//==========================================================
CustomParametrisation::CustomParametrisation(const param::MIRParametrisation &parametrisation):
    parametrisation_(parametrisation) {
}


CustomParametrisation::~CustomParametrisation() {

}


bool CustomParametrisation::has(const std::string& name) const {
    std::cout << "CustomParametrisation::has: " << name << std::endl;
    return parametrisation_.has(name);
}

bool CustomParametrisation::get(const std::string& name, std::string& value) const {
    std::cout << "CustomParametrisation::get: " << name << std::endl;
    return parametrisation_.get(name, value);
}

bool CustomParametrisation::get(const std::string& name, bool& value) const {
    std::cout << "CustomParametrisation::get: " << name << std::endl;
    return parametrisation_.get(name, value);
}

bool CustomParametrisation::get(const std::string& name, int& value) const {
    std::cout << "CustomParametrisation::get: " << name << std::endl;
    return parametrisation_.get(name, value);
}

bool CustomParametrisation::get(const std::string& name, long& value) const {
    std::cout << "CustomParametrisation::get: " << name << std::endl;
    return parametrisation_.get(name, value);
}

bool CustomParametrisation::get(const std::string& name, float& value) const {
    std::cout << "CustomParametrisation::get: " << name << std::endl;
    return parametrisation_.get(name, value);
}

bool CustomParametrisation::get(const std::string& name, double& value) const {
    std::cout << "CustomParametrisation::get: " << name << std::endl;
    return parametrisation_.get(name, value);
}

bool CustomParametrisation::get(const std::string& name, std::vector<int>& value) const {
    std::cout << "CustomParametrisation::get: " << name << std::endl;
    return parametrisation_.get(name, value);
}

bool CustomParametrisation::get(const std::string& name, std::vector<long>& value) const {
    std::cout << "CustomParametrisation::get: " << name << std::endl;
    return parametrisation_.get(name, value);
}

bool CustomParametrisation::get(const std::string& name, std::vector<float>& value) const {
    std::cout << "CustomParametrisation::get: " << name << std::endl;
    return parametrisation_.get(name, value);
}

bool CustomParametrisation::get(const std::string& name, std::vector<double>& value) const {
    std::cout << "CustomParametrisation::get: " << name << std::endl;
    return parametrisation_.get(name, value);
}

bool CustomParametrisation::get(const std::string& name, std::vector<std::string>& value) const {
    std::cout << "CustomParametrisation::get: " << name << std::endl;
    return parametrisation_.get(name, value);
}


void CustomParametrisation::print(std::ostream &out) const {
    out << "<CustomParametrisation>";
}

}  // namespace param
}  // namespace mir

