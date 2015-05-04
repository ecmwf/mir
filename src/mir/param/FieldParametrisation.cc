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


#include "eckit/exception/Exceptions.h"

#include "mir/param/FieldParametrisation.h"
#include "mir/param/MIRConfiguration.h"

namespace mir {
namespace param {


FieldParametrisation::FieldParametrisation(): check_(false), logic_(0) {
}


FieldParametrisation::~FieldParametrisation() {
}


bool FieldParametrisation::has(const std::string &name) const {

    // FIXME: not very elegant
    if (name == "spherical") {
        long dummy;
        return get("truncation", dummy);
    }

    eckit::Log::info() << "FieldParametrisation::has(" << name << ") " << *this << std::endl;
    return false;
}


template<class T>
bool FieldParametrisation::_get(const std::string &name, T &value) const {

    const param::MIRConfiguration &configuration = param::MIRConfiguration::instance();

    if (!logic_ && !check_) {
        check_ = true;

        long paramId = 0;

        // This assumes that other input (NetCDF, etc) also return a paramId
        if (get("paramId", paramId)) {
            logic_ = configuration.lookup(paramId);
            if (logic_) {
                eckit::Log::info() << "paramId=" << paramId << " " << *logic_ << std::endl;
            }
        } else {
            eckit::Log::info() << "ERROR: " << *this << " has no paramId" << std::endl;
        }
    }

    eckit::Log::info() << "FieldParametrisation::_get(" << name << ") " <<  *this << std::endl;

    if (logic_) {
        return logic_->get(name, value);
    }

    return false;
}

bool FieldParametrisation::get(const std::string &name, std::string &value) const {
    return _get(name, value);
}

bool FieldParametrisation::get(const std::string &name, bool &value) const {
    return _get(name, value);
}

bool FieldParametrisation::get(const std::string &name, long &value) const {
    return _get(name, value);
}

bool FieldParametrisation::get(const std::string &name, double &value) const {
    return _get(name, value);
}

bool FieldParametrisation::get(const std::string &name, std::vector<long> &value) const {
    return _get(name, value);
}

bool FieldParametrisation::get(const std::string &name, std::vector<double> &value) const {

    if (_get(name, value)) { // This will check if this in the logic paramaretirsaion
        return true;
    }

    // Special case

    if (name == "grid") {
        double west_east_increment, north_south_increment;

        if (get("west_east_increment", west_east_increment) && get("north_south_increment", north_south_increment)) {
            value.resize(2);
            value[0] = west_east_increment;
            value[1] = north_south_increment;
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

    return _get(name, value);
}

}  // namespace param
}  // namespace mir

