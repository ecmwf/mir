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


#include "Parametrisation.h"

#include <sstream>
#include <string>
#include <vector>

#include "eckit/log/JSON.h"


namespace mir_bridge {


void Parametrisation::set_str(rust::Str name, rust::Str value) {
    set(std::string(name), std::string(value));
}


void Parametrisation::set_f64(rust::Str name, double value) {
    set(std::string(name), value);
}


void Parametrisation::set_i64(rust::Str name, int64_t value) {
    set(std::string(name), static_cast<long long>(value));
}


void Parametrisation::set_bool(rust::Str name, bool value) {
    set(std::string(name), value);
}


void Parametrisation::set_f64_list(rust::Str name, rust::Slice<const double> values) {
    set(std::string(name), std::vector<double>(values.begin(), values.end()));
}


void Parametrisation::set_i64_list(rust::Str name, rust::Slice<const int64_t> values) {
    set(std::string(name), std::vector<long long>(values.begin(), values.end()));
}


rust::String Parametrisation::to_json() const {
    std::ostringstream s;
    eckit::JSON j(s);
    json(j);
    return rust::String(s.str());
}


std::unique_ptr<Parametrisation> Parametrisation::make() {
    return std::make_unique<Parametrisation>();
}


}  // namespace mir_bridge
