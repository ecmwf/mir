/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Aug 2016


#ifndef mir_util_MapKeyValue_h
#define mir_util_MapKeyValue_h

#include "mir/param/SimpleParametrisation.h"


namespace mir {
namespace util {


struct MapKeyValue : private param::SimpleParametrisation {

    MapKeyValue() {}

    MapKeyValue(const MapKeyValue& other) {
        operator=(other);
    }

    MapKeyValue& operator=(const MapKeyValue& other) {
        other.copyValuesTo(*this);
        return *this;
    }

    bool operator==(const MapKeyValue& other) const {
        return size() == other.size() && matches(other);
    }

    using SimpleParametrisation::has;
    using SimpleParametrisation::set;
    using SimpleParametrisation::reset;

    template< typename T >
    T get(const std::string& name, const T& defaultValue=T()) const {
        T value(defaultValue);
        return SimpleParametrisation::get(name, value)? value : defaultValue;
    }

    template< typename T >
    bool setFrom(const MIRParametrisation& other, const std::string& name, const T& defaultValue=T()) {
        T v;
        if (other.get(name, v)) {
            set(name, v);
            return true;
        }
        set(name, defaultValue);
        return false;
    }

    friend std::ostream& operator<<(std::ostream& s, const MapKeyValue& p) {
        p.print(s);
        return s;
    }

};


template<>
size_t MapKeyValue::get(const std::string& name, const size_t& defaultValue) const;


}  // namespace util
}  // namespace mir


#endif

