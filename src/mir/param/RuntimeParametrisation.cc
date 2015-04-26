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


RuntimeParametrisation::RuntimeParametrisation() {
}


RuntimeParametrisation::~RuntimeParametrisation() {
}


void RuntimeParametrisation::print(std::ostream& out) const {
    out << "RuntimeParametrisation[";
    SimpleParametrisation::print(out);
    out << "]";
}


void RuntimeParametrisation::set(const std::string& name, long value) {
    eckit::Log::info() << "************* RuntimeParametrisation::set [" << name << "] = [" << value << "] (long)" << std::endl;
    SimpleParametrisation::set(name, value);
}

}  // namespace param
}  // namespace mir

