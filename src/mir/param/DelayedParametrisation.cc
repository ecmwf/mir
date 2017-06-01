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


#include "mir/param/DelayedParametrisation.h"

#include "eckit/exception/Exceptions.h"


namespace mir {
namespace param {


DelayedParametrisation::DelayedParametrisation() {
}


DelayedParametrisation::~DelayedParametrisation() {
}


void DelayedParametrisation::get(const std::string& name, std::string&) const {
    std::ostringstream os;
    os << "DelayedParametrisation::get(" << name << ") [string] not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


void DelayedParametrisation::get(const std::string& name, bool&) const {
    std::ostringstream os;
    os << "DelayedParametrisation::get(" << name << ") [bool] not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


void DelayedParametrisation::get(const std::string& name, int&) const {
    std::ostringstream os;
    os << "DelayedParametrisation::get(" << name << ") [int] not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


void DelayedParametrisation::get(const std::string& name, long&) const {
    std::ostringstream os;
    os << "DelayedParametrisation::get(" << name << ") [long] not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


void DelayedParametrisation::get(const std::string& name, size_t&) const {
    std::ostringstream os;
    os << "DelayedParametrisation::get(" << name << ") [size_t] not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


void DelayedParametrisation::get(const std::string& name, float&) const {
    std::ostringstream os;
    os << "DelayedParametrisation::get(" << name << ") [float] not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


void DelayedParametrisation::get(const std::string& name, double&) const {
    std::ostringstream os;
    os << "DelayedParametrisation::get(" << name << ") [double] not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


void DelayedParametrisation::get(const std::string& name, std::vector<int>&) const {
    std::ostringstream os;
    os << "DelayedParametrisation::get(" << name << ") [vector<int>] not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


void DelayedParametrisation::get(const std::string& name, std::vector<size_t>&) const {
    std::ostringstream os;
    os << "DelayedParametrisation::get(" << name << ") [vector<size_t>] not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


void DelayedParametrisation::get(const std::string& name, std::vector<float>&) const {
    std::ostringstream os;
    os << "DelayedParametrisation::get(" << name << ") [vector<float>] not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


void DelayedParametrisation::get(const std::string& name, std::vector<std::string>&) const {
    std::ostringstream os;
    os << "DelayedParametrisation::get(" << name << ") [vector<string>] not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


void DelayedParametrisation::get(const std::string& name, std::vector<long>&) const {
    std::ostringstream os;
    os << "DelayedParametrisation::get(" << name << ") [vector<long>] not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


void DelayedParametrisation::get(const std::string& name, std::vector<double>&) const {
    std::ostringstream os;
    os << "DelayedParametrisation::get(" << name << ") [vector<double] not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


}  // namespace param
}  // namespace mir

