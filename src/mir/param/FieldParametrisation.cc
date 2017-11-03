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
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/types/Fraction.h"
#include "mir/config/LibMir.h"
#include "mir/param/RulesFromFile.h"


namespace mir {
namespace param {


namespace {


inline double shift(const double& a, const double& b, double increment) {
    const eckit::Fraction inc(increment);
    eckit::Fraction shift = a - (a / inc).integralPart() * inc;

    if (!((a - b) / inc).integer()) {
        std::ostringstream oss;
        oss << "Cannot compute shift with a=" << a << ", b=" << b << ", inc=" << double(inc)
            << " shift=" << double(shift) << " (a-b)/inc=" << double((a - b) / inc);
        throw eckit::SeriousBug(oss.str());
    }

    return shift;
}


static Rules fileRules;


static pthread_once_t once = PTHREAD_ONCE_INIT;
static eckit::Mutex *local_mutex = 0;
static void init() {
    local_mutex = new eckit::Mutex();
    fileRules.readConfigurationFiles();
}


}  // (anonymous namespace)


FieldParametrisation::FieldParametrisation():
    paramId_(-1)  {
}


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
    double inc;
    double a;
    double b;

    if (name == "west_east_shift") {
        if (get("west_east_increment", inc) && get("east", a) && get("west", b)) {
            value = shift(a, b, inc);
            return true;
        }
    }

    if (name == "south_north_shift") {
        if (get("south_north_increment", inc) && get("north", a) && get("south", b)) {
            value = shift(a, b, inc);
            return true;
        }
    }

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

    // Check if this is in the MIRConfiguration
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
        return true;
    }

    if (name == "longitudes") {
        longitudes(value);
        return true;
    }

    return false;
}


bool FieldParametrisation::get(const std::string& name, std::vector<std::string>& value) const {
    return _get(name, value);
}


template <class T>
bool FieldParametrisation::_get(const std::string& name, T& value) const {

    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    static std::string PARAM_ID("paramId");

    ASSERT(name != PARAM_ID);

    // return paramId-specific setting
    // This assumes that other input (NetCDF, etc) also return a paramId

    if (paramId_ <= 0) {
        get(PARAM_ID, paramId_);
    }

    if (paramId_ <= 0) {
        return false;
    }

    if (userRules_) {
        if (userRules_->lookup(PARAM_ID, paramId_).get(name, value)) {
            return true;
        }
    }

    return fileRules.lookup(PARAM_ID, paramId_).get(name, value);
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
