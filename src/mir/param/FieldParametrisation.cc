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
#include "eckit/memory/Owned.h"
#include "eckit/memory/SharedPtr.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/types/Fraction.h"
#include "mir/config/LibMir.h"
#include "mir/param/SimpleParametrisation.h"


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


// handle memory correctly due to copying of SimpleParametrisations
struct CountedParametrisation : SimpleParametrisation, eckit::OwnedLock {};


static pthread_once_t once = PTHREAD_ONCE_INIT;
static std::map< long, eckit::SharedPtr<CountedParametrisation> > parameters_;


}  // (anonymous namespace)


static void init() {
    std::map< std::string, eckit::SharedPtr<CountedParametrisation> > allClasses;


    eckit::ValueMap classes = eckit::YAMLParser::decodeFile("~mir/etc/mir/classes.yaml");
    for (auto i : classes) {
        const std::string& klass = i.first;
        eckit::ValueMap values = i.second;

        if (values.find("dimension") != values.end()) {
            throw eckit::UserError("Class cannot use reserved key 'dimension' ('" + klass + "')");
        }

        CountedParametrisation* s = new CountedParametrisation();

        for (auto j : values) {
            std::string name = j.first;
            eckit::Value value = j.second;
            s->set(name, std::string(value));
        }

        allClasses[klass].reset(s);
    }


    eckit::ValueMap parameterClass = eckit::YAMLParser::decodeFile("~mir/etc/mir/parameter-class.yaml");
    for (auto i : parameterClass) {
        const std::string& klass = i.first;

        auto j = allClasses.find(klass);
        if (j == allClasses.end()) {
            throw eckit::UserError("Class unknown '" + klass + "'");
        }

        eckit::ValueList list = i.second;
        // std::cout << list << std::endl;

        for (long paramId : list) {
            if (parameters_.find(paramId) != parameters_.end()) {
                throw eckit::UserError("Parameter class is set more than once, for paramId=" + std::to_string(paramId));
            }
            parameters_[paramId].reset(j->second);
        }
    }


    eckit::ValueMap parameterDimension = eckit::YAMLParser::decodeFile("~mir/etc/mir/parameter-dimension.yaml");
    for (auto i : parameterDimension) {
        const std::string& dimension = i.first;
        eckit::ValueList list = i.second;

        for (long paramId : list) {

            // paramId-specific 'dimension' value
            CountedParametrisation* s = new CountedParametrisation();
            s->set("dimension", dimension);

            auto p = parameters_.find(paramId);
            if (p != parameters_.end()) {

                // known parameter: copy class settings to new entry
                std::string d;
                if (p->second->get("dimension", d)) {
                    throw eckit::UserError("Parameter dimension is set more than once, for paramId=" + std::to_string(paramId));
                }
                p->second->copyValuesTo(*s);
                p->second.reset(s);

            } else {

                // unknown parameter: set dimension only
                parameters_[paramId].reset(s);

            }
        }
    }
}


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

    ASSERT(name != "paramId");

    // This assumes that other input (NetCDF, etc) also return a paramId

    long paramId;
    if (!get("paramId", paramId)) {
        return false;
    }

    // return paramId specific parametrisation

    pthread_once(&once, init);

    const auto j = parameters_.find(paramId);

    if (j == parameters_.end()) {
        eckit::Log::warning() << "No information for paramId=" << paramId << ", using defaults" << std::endl;
        parameters_[paramId].reset(new CountedParametrisation());
        return false;
    }

    return (*j).second->get(name, value);
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
