/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include "mir/param/FieldParametrisation.h"

#include "eckit/exception/Exceptions.h"
#include "mir/config/LibMir.h"
#include "mir/config/MIRConfiguration.h"
#include "mir/param/InheritParametrisation.h"


namespace mir {
namespace param {


FieldParametrisation::FieldParametrisation() {}


FieldParametrisation::~FieldParametrisation() {}


bool FieldParametrisation::has(const std::string& name) const {

    // FIXME: not very elegant
    // if (name == "spectral") {
    //     long dummy;
    //     return get("truncation", dummy);
    // }

    eckit::Log::debug<LibMir>() << "FieldParametrisation::has(" << name << ") "
                                << *this << std::endl;
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

    if (_get(name, value)) { // This will check if this in the style paramaretirsaion
        return true;
    }

    // Special case

    if (name == "grid") {
        double west_east_increment, south_north_increment;

        if (get("west_east_increment", west_east_increment) && get("south_north_increment", south_north_increment)) {
            value.resize(2);
            value[0] = west_east_increment;
            value[1] = south_north_increment;
            return true;
        }
    }

    if (name == "area") {
        double north, west, south, east;

        if (get("north", north) && get("west", west) && get("south", south) && get("east", east)) {
            value.resize(4);
            value[0] = north;
            value[1] = west;
            value[2] = south;
            value[3] = east;
            return true;
        }
    }

    if (name == "latitudes") {
        latitudes(value);
        return true;
    }

    if (name == "longitudes") {
        longitudes(value);
        return true;
    }

    return _get(name, value);
}


bool FieldParametrisation::get(const std::string& name, std::vector<std::string>& value) const {
    return _get(name, value);
}


template <class T>
bool FieldParametrisation::_get(const std::string& name, T& value) const {

    ASSERT(name != "paramId");

    // This assumes that other input (NetCDF, etc) also return a paramId
    long paramId = 0;
    if (get("paramId", paramId)) {
        // return paramId specific parametrisation
        const config::MIRConfiguration& configuration = config::MIRConfiguration::instance();
        const param::MIRParametrisation& param = configuration.lookup(paramId, *this);
        return param.get(name, value);
    }

    return false;
}


void FieldParametrisation::latitudes(std::vector<double>&) const {
    std::ostringstream os;
    os << "FieldParametrisation::latitudes() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


void FieldParametrisation::longitudes(std::vector<double>&) const {
    std::ostringstream os;
    os << "FieldParametrisation::longitudes() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


}  // namespace param
}  // namespace mir
