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


namespace mir {
namespace param {


FieldParametrisation::FieldParametrisation() {
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
bool FieldParametrisation::_get(const std::string& name, T& value) const {
    eckit::Log::info() << "FieldParametrisation::_get(" << name << ") " <<  *this << std::endl;
    return false;
}

bool FieldParametrisation::get(const std::string &name, std::string &value) const {
    return _get(name, value);
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

// bool GribInput::get(const std::string &name, std::string &value) const {

//     eckit::Log::info() << "GribInput::get " << name << std::endl;

//     ASSERT(grib_.get());

//     // Assumes LL grid, and scanning mode

//     if (name == "area") {
//         double latitudeOfFirstGridPointInDegrees;
//         double longitudeOfFirstGridPointInDegrees;
//         double latitudeOfLastGridPointInDegrees;
//         double longitudeOfLastGridPointInDegrees;
//         double jDirectionIncrementInDegrees;
//         double iDirectionIncrementInDegrees;

//         if (!GRIB_GET(grib_get_double(grib_.get(), "latitudeOfFirstGridPointInDegrees", &latitudeOfFirstGridPointInDegrees))) {
//             return false;
//         }

//         if (!GRIB_GET(grib_get_double(grib_.get(), "longitudeOfFirstGridPointInDegrees", &longitudeOfFirstGridPointInDegrees))) {
//             return false;
//         }

//         if (!GRIB_GET(grib_get_double(grib_.get(), "latitudeOfLastGridPointInDegrees", &latitudeOfLastGridPointInDegrees))) {
//             return false;
//         }

//         if (!GRIB_GET(grib_get_double(grib_.get(), "longitudeOfLastGridPointInDegrees", &longitudeOfLastGridPointInDegrees))) {
//             return false;
//         }

//         if (!GRIB_GET(grib_get_double(grib_.get(), "jDirectionIncrementInDegrees", &jDirectionIncrementInDegrees))) {
//             return false;
//         }

//         if (!GRIB_GET(grib_get_double(grib_.get(), "iDirectionIncrementInDegrees", &iDirectionIncrementInDegrees))) {
//             return false;
//         }


//         double v = latitudeOfFirstGridPointInDegrees - latitudeOfLastGridPointInDegrees;
//         double h = (longitudeOfLastGridPointInDegrees + iDirectionIncrementInDegrees) - longitudeOfFirstGridPointInDegrees;

//         if (v == 180 && h == 360) {
//             value = "global";
//         } else {
//             eckit::StrStream os;
//             os << latitudeOfFirstGridPointInDegrees
//                << "/"
//                << longitudeOfFirstGridPointInDegrees
//                << "/"
//                << latitudeOfLastGridPointInDegrees
//                << "/"
//                << longitudeOfLastGridPointInDegrees
//                << eckit::StrStream::ends;

//             value = std::string(os);
//         }

//         eckit::Log::info() << "GribInput::get " << name << " is " << value << std::endl;

//         cache_[name] = value;
//         return true;
//     }

//     if (name == "grid") {

//         double jDirectionIncrementInDegrees;
//         double iDirectionIncrementInDegrees;

//         if (!GRIB_GET(grib_get_double(grib_.get(), "jDirectionIncrementInDegrees", &jDirectionIncrementInDegrees))) {
//             return false;
//         }

//         if (!GRIB_GET(grib_get_double(grib_.get(), "iDirectionIncrementInDegrees", &iDirectionIncrementInDegrees))) {
//             return false;
//         }


//         eckit::StrStream os;
//         os << iDirectionIncrementInDegrees
//            << "/"
//            << jDirectionIncrementInDegrees
//            << eckit::StrStream::ends;

//         value = std::string(os);

//         eckit::Log::info() << "GribInput::get " << name << " is " << value << std::endl;

//         cache_[name] = value;
//         return true;
//     }

//     if (name == "regular") {
//         std::string type;
//         if (get("gridType", type)) {
//             if (type == "regular_gg") {

//                 long N;

//                 GRIB_CALL(grib_get_long(grib_.get(), "N", &N));
//                 // GRIB_CALL(grib_get_double(grib_.get(), "iDirectionIncrementInDegrees", &iDirectionIncrementInDegrees));

//                 eckit::StrStream os;
//                 os << N << eckit::StrStream::ends;

//                 value = std::string(os);

//                 eckit::Log::info() << "GribInput::get " << name << " is " << value << std::endl;

//                 cache_[name] = value;
//                 return true;
//             }
//         }
//     }

//     if (name == "reduced") {
//         std::string type;
//         if (get("gridType", type)) {
//             if (type == "reduced_gg") {

//                 long N;

//                 GRIB_CALL(grib_get_long(grib_.get(), "N", &N));
//                 // GRIB_CALL(grib_get_double(grib_.get(), "iDirectionIncrementInDegrees", &iDirectionIncrementInDegrees));

//                 eckit::StrStream os;
//                 os << N << eckit::StrStream::ends;

//                 value = std::string(os);

//                 eckit::Log::info() << "GribInput::get " << name << " is " << value << std::endl;

//                 cache_[name] = value;
//                 return true;
//             }
//         }
//     }


//     const char *key = name.c_str();
//     size_t i = 0;
//     while (mappings[i].name) {
//         if (name == mappings[i].name) {
//             key = mappings[i].key;
//             break;
//         }
//         i++;
//     }

//     char buffer[1024];
//     size_t size = sizeof(buffer);
//     int err = grib_get_string(grib_.get(), key, buffer, &size);

//     if (err == GRIB_SUCCESS) {
//         value = buffer;
//         eckit::Log::info() << "GribInput::get " << name << " is " << value << " (as " << key << ")" << std::endl;
//         cache_[name] = value;
//         return true;
//     }

//     if (err != GRIB_NOT_FOUND) {
//         GRIB_ERROR(err, name.c_str());
//     }

//     return FieldParametrisation::get(name, value);
// }


}  // namespace param
}  // namespace mir

