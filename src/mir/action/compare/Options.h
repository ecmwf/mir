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


#ifndef mir_action_compare_Options_h
#define mir_action_compare_Options_h

#include "mir/param/SimpleParametrisation.h"


namespace mir {
namespace action {
namespace compare {


struct Options : private param::SimpleParametrisation {
    Options() {}
    Options(const Options& other) { operator=(other); }
    Options& operator=(const Options& other) {
        other.copyValuesTo(*this);
        return *this;
    }
    bool operator==(const Options& other) const {
        return size() == other.size() && matches(other);
    }
    using SimpleParametrisation::has;
    using SimpleParametrisation::set;
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
    friend std::ostream& operator<<(std::ostream& s, const Options& p) {
        p.print(s);
        return s;
    }
};


template<>
size_t Options::get(const std::string& name, const size_t& defaultValue) const;


}  // namespace compare
}  // namespace action
}  // namespace mir


#endif

