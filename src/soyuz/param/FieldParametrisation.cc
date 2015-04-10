// File FieldParametrisation.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "soyuz/param/FieldParametrisation.h"
#include "eckit/exception/Exceptions.h"


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
        std::string ignore;
        if(!get("truncation", ignore)) {
            return true;
        }
    }

    if(name == "spherical") {
        std::string ignore;
        if(get("truncation", ignore)) {
            return true;
        }
    }

    return lowLevelGet(name, value);
}
