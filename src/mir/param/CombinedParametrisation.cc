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


#include "mir/param/CombinedParametrisation.h"

#include <ostream>


namespace mir::param {


CombinedParametrisation::CombinedParametrisation(const MIRParametrisation& user, const MIRParametrisation& metadata,
                                                 const MIRParametrisation& defaults) :
    user_(user), field_(metadata), defaults_(defaults) {}


CombinedParametrisation::~CombinedParametrisation() = default;


const MIRParametrisation& CombinedParametrisation::userParametrisation() const {
    return user_;
}


const MIRParametrisation& CombinedParametrisation::fieldParametrisation() const {
    return field_;
}


template <class T>
bool CombinedParametrisation::_get(const std::string& name, T& value) const {

    // This could be a loop
    return user_.get(name, value) || field_.get(name, value) || defaults_.get(name, value);
}


void CombinedParametrisation::print(std::ostream& out) const {
    out << "CombinedParametrisation["
        //        <<  "user=" << user_
        //        << ",field=" << field_
        //        << ",defaults=" << defaults_
        << "]";
}


bool CombinedParametrisation::has(const std::string& name) const {

    // This could be a loop
    return user_.has(name) || field_.has(name) || defaults_.has(name);
}


bool CombinedParametrisation::get(const std::string& name, std::string& value) const {
    return _get(name, value);
}


bool CombinedParametrisation::get(const std::string& name, bool& value) const {
    return _get(name, value);
}

bool CombinedParametrisation::get(const std::string& name, int& value) const {
    return _get(name, value);
}


bool CombinedParametrisation::get(const std::string& name, long& value) const {
    return _get(name, value);
}


bool CombinedParametrisation::get(const std::string& name, float& value) const {
    return _get(name, value);
}


bool CombinedParametrisation::get(const std::string& name, double& value) const {
    return _get(name, value);
}


bool CombinedParametrisation::get(const std::string& name, std::vector<int>& value) const {
    return _get(name, value);
}


bool CombinedParametrisation::get(const std::string& name, std::vector<float>& value) const {
    return _get(name, value);
}


bool CombinedParametrisation::get(const std::string& name, std::vector<long>& value) const {
    return _get(name, value);
}


bool CombinedParametrisation::get(const std::string& name, std::vector<double>& value) const {
    return _get(name, value);
}

bool CombinedParametrisation::get(const std::string& name, std::vector<std::string>& value) const {
    return _get(name, value);
}


}  // namespace mir::param
