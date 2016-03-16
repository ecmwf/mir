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

#include "eckit/exception/Exceptions.h"
#include "eckit/config/Configuration.h"

#include "mir/param/MIRCombinedParametrisation.h"
#include "mir/log/MIR.h"


namespace mir {
namespace param {


MIRCombinedParametrisation::MIRCombinedParametrisation(
        const MIRParametrisation& user,
        const MIRParametrisation& metadata,
        const MIRParametrisation& defaults):
    user_(user),
    metadata_(metadata),
    defaults_(defaults) {
}


MIRCombinedParametrisation::~MIRCombinedParametrisation() {
}


void MIRCombinedParametrisation::print(std::ostream& out) const {
    out << "MIRCombinedParametrisation["
        // << "user="
        // << user_ <<
        // ",metadata=" << metadata_ <<
        // ",configuration=" << configuration_ <<
        // ",defaults=" << defaults_
        << "]";
}

bool MIRCombinedParametrisation::has(const std::string& name) const {
    eckit::Log::trace<MIR>() << "MIRCombinedParametrisation::has(" << name << ")" << std::endl;

    if (name.find("user.") == 0) {
        return user_.has(name.substr(5));
    }

    if (name.find("field.") == 0) {
        return metadata_.has(name.substr(6));
    }

// This could be a loop
    if (user_.has(name)) return true;
    if (metadata_.has(name)) return true;
    if (defaults_.has(name)) return true;

    eckit::Log::trace<MIR>() << "**** MIRCombinedParametrisation::has(" << name << ") unanswered" << std::endl;
    return false;
}

template<class T>
bool MIRCombinedParametrisation::_get(const std::string& name, T& value) const {
    eckit::Log::trace<MIR>() << "MIRCombinedParametrisation::get(" << name << ")" << std::endl;

    if (name.find("user.") == 0) {
        return user_.get(name.substr(5), value);
    }

    if (name.find("field.") == 0) {
        return metadata_.get(name.substr(6), value);
    }

    // This could be a loop
    if (user_.get(name, value)) return true;
    if (metadata_.get(name, value)) return true;
    if (defaults_.get(name, value)) return true;

    eckit::Log::trace<MIR>() << "**** MIRCombinedParametrisation::get(" << name << ") unanswered" << std::endl;

    return false;
}

bool MIRCombinedParametrisation::get(const std::string& name, std::string& value) const {
    return _get(name, value);
}

bool MIRCombinedParametrisation::get(const std::string& name, bool& value) const {
    return _get(name, value);
}

bool MIRCombinedParametrisation::get(const std::string& name, long& value) const {
    return _get(name, value);
}

bool MIRCombinedParametrisation::get(const std::string& name, double& value) const {
    return _get(name, value);
}

bool MIRCombinedParametrisation::get(const std::string& name, std::vector<long>& value) const {
    return _get(name, value);
}

bool MIRCombinedParametrisation::get(const std::string& name, std::vector<double>& value) const {
    return _get(name, value);
}


}  // namespace param
}  // namespace mir

