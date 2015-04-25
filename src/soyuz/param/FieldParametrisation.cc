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

#include "soyuz/param/FieldParametrisation.h"


namespace mir {
namespace param {


FieldParametrisation::FieldParametrisation() {
}


FieldParametrisation::~FieldParametrisation() {
}


bool FieldParametrisation::get(const std::string &name, std::string &value) const {

    // This are the keywork that must be replied to
    // to make sure they can be compared to the user request

    if (name == "area") {

        std::string north;
        std::string west;
        std::string south;
        std::string east;

        if (get("north", north) && get("west", west) && get("south", south) && get("east", east)) {
            eckit::StrStream os;
            os << north
               << "/"
               << west
               << "/"
               << south
               << "/"
               << east
               << eckit::StrStream::ends;

            value = std::string(os);
        } else {
            value = "unknown";
        }
        return true;
    }

    if (name == "grid") {

        std::string west_east_increment;
        std::string north_south_increment;

        if (get("west_east_increment", west_east_increment) && get("north_south_increment", north_south_increment) ) {
            eckit::StrStream os;
            os << west_east_increment
               << "/"
               << north_south_increment
               << eckit::StrStream::ends;

            value = std::string(os);
        } else {
            value = "unknown";
        }
        return true;
    }

    if (name == "regular") {
        value = "unknown";
        std::string type;
        if (get("gridType", type)) {
            if (type == "regular_gg") {
                if (get("N", value)) {
                    return true;
                }
            }
        }
        return true;
    }

    if (name == "reduced") {
        value = "unknown";
        std::string type;
        if (get("gridType", type)) {
            if (type == "reduced_gg") {
                if (get("N", value)) {
                    return true;
                }
            }
        }
        return true;
    }

    // These two return true of false only
    if(name == "gridded") {
        // TODO: something better, this is just a hack
        if(!has("truncation")) {
            return true;
        }
    }

    if(name == "spherical") {
        if(has("truncation")) {
            return true;
        }
    }

    return lowLevelGet(name, value);
}

template<class T>
bool FieldParametrisation::_get(const std::string& name, T& value) const {
    NOTIMP;
}

bool FieldParametrisation::get(const std::string& name, bool& value) const {
    return _get(name, value);
}

bool FieldParametrisation::get(const std::string& name, long& value) const {
    return _get(name, value);
}

bool FieldParametrisation::get(const std::string& name, double& value) const {
    return _get(name, value);
}

bool FieldParametrisation::get(const std::string& name, std::vector<long>& value) const {
    return _get(name, value);
}

bool FieldParametrisation::get(const std::string& name, std::vector<double>& value) const {
    return _get(name, value);
}

bool FieldParametrisation::lowLevelGet(const std::string& name, bool& value) const {
    NOTIMP;
}

bool FieldParametrisation::lowLevelGet(const std::string& name, long& value) const {
    NOTIMP;
}

bool FieldParametrisation::lowLevelGet(const std::string& name, double& value) const {
    NOTIMP;
}

bool FieldParametrisation::lowLevelGet(const std::string& name, std::vector<long>& value) const {
    NOTIMP;
}

bool FieldParametrisation::lowLevelGet(const std::string& name, std::vector<double>& value) const {
    NOTIMP;
}



}  // namespace param
}  // namespace mir

