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


#include "mir/param/FieldParametrisation.h"

#include <sstream>

#include "mir/param/Rules.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::param {


static const MIRParametrisation* find_param_rules(const param::MIRParametrisation& param) {
    static const Rules rules;
    return rules.find(param);
}


FieldParametrisation::FieldParametrisation() : param_(nullptr) {}


FieldParametrisation::~FieldParametrisation() = default;


bool FieldParametrisation::has(const std::string& /*name*/) const {
    // Log::debug() << "FieldParametrisation::has(" << name << ") " << *this << std::endl;
    return false;
}


bool FieldParametrisation::get(const std::string& name, std::string& value) const {
    return _get(name, value);
}


bool FieldParametrisation::get(const std::string& name, bool& value) const {
    return _get(name, value);
}


bool FieldParametrisation::get(const std::string& name, int& value) const {
    return _get(name, value);
}


bool FieldParametrisation::get(const std::string& name, long& value) const {
    return _get(name, value);
}


bool FieldParametrisation::get(const std::string& name, float& value) const {
    return _get(name, value);
}


bool FieldParametrisation::get(const std::string& name, double& value) const {
    return _get(name, value);
}


bool FieldParametrisation::get(const std::string& name, std::vector<int>& value) const {
    return _get(name, value);
}


bool FieldParametrisation::get(const std::string& name, std::vector<long>& value) const {
    return _get(name, value);
}


bool FieldParametrisation::get(const std::string& name, std::vector<float>& value) const {
    return _get(name, value);
}


bool FieldParametrisation::get(const std::string& name, std::vector<double>& value) const {

    // Check if this is in the Rules
    if (_get(name, value)) {
        return true;
    }

    // Special cases
    if (name == "grid") {
        std::vector<double> grid(2, 0.);
        if (get("west_east_increment", grid[0]) && get("south_north_increment", grid[1])) {
            value.swap(grid);
            return true;
        }
    }

    if (name == "area") {
        std::vector<double> area(4, 0.);
        if (get("north", area[0]) && get("west", area[1]) && get("south", area[2]) && get("east", area[3])) {
            value.swap(area);
            return true;
        }
    }

    if (name == "latitudes") {
        latitudes(value);
        return !value.empty();
    }

    if (name == "longitudes") {
        longitudes(value);
        return !value.empty();
    }

    return false;
}


bool FieldParametrisation::get(const std::string& name, std::vector<std::string>& value) const {
    return _get(name, value);
}


void FieldParametrisation::reset() {
    // Reset cached values
    param_ = nullptr;
}


template <class T>
bool FieldParametrisation::_get(const std::string& name, T& value) const {
    static const SimpleParametrisation empty;
    static const std::string PARAM_ID("paramId");

    ASSERT(name != PARAM_ID);

    // return paramId-specific setting (classification)
    if (param_ == nullptr && (param_ = find_param_rules(*this)) == nullptr) {
        param_ = &empty;
        ASSERT(param_ != nullptr);
    }

    return param_->get(name, value);
}


void FieldParametrisation::latitudes(std::vector<double>& /*unused*/) const {
    std::ostringstream os;
    os << "FieldParametrisation::latitudes() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void FieldParametrisation::longitudes(std::vector<double>& /*unused*/) const {
    std::ostringstream os;
    os << "FieldParametrisation::longitudes() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


}  // namespace mir::param
