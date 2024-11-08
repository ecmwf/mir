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


#include "mir/param/MIRParametrisation.h"

#include <sstream>

#include "mir/util/Exceptions.h"


namespace mir::param {


MIRParametrisation::MIRParametrisation() = default;


MIRParametrisation::~MIRParametrisation() = default;


const MIRParametrisation& MIRParametrisation::userParametrisation() const {
    std::ostringstream os;
    os << "MIRParametrisation::userParametrisation() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


const MIRParametrisation& MIRParametrisation::fieldParametrisation() const {
    std::ostringstream os;
    os << "MIRParametrisation::fieldParametrisation() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


bool MIRParametrisation::get(const std::string& name, size_t& value) const {
    long v;
    if (get(name, v)) {
        ASSERT(v >= 0);
        value = size_t(v);
        return true;
    }
    return false;
}


bool MIRParametrisation::get(const std::string& name, std::vector<size_t>& value) const {
    std::vector<long> v;
    if (get(name, v)) {
        value.clear();
        value.reserve(v.size());
        for (const long& l : v) {
            ASSERT(l >= 0);
            value.push_back(size_t(l));
        }
        return true;
    }
    return false;
}

bool MIRParametrisation::get(const std::string& name, long long& value) const {
    long v;
    if (get(name, v)) {
        ASSERT(v >= 0);
        value = v;
        return true;
    }
    return false;
}


bool MIRParametrisation::get(const std::string& name, std::vector<long long>& value) const {
    std::vector<long> v;
    if (get(name, v)) {
        value.clear();
        value.reserve(v.size());
        for (const long& l : v) {
            value.push_back(l);
        }
        return true;
    }
    return false;
}

bool MIRParametrisation::get(const std::string& name, std::vector<bool>& value) const {
	 std::ostringstream os;
	 os << "MIRParametrisation::get(const std::string& name, std::vector<bool>& value) not implemented for " << *this;
	 throw exception::SeriousBug(os.str());

 }
}  // namespace mir::param
