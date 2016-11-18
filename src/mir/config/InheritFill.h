/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Nov 2016


#ifndef mir_config_InheritFill_h
#define mir_config_InheritFill_h

#include <iosfwd>
#include <vector>
#include "mir/param/SimpleParametrisation.h"


namespace mir {
namespace config {


class InheritFill : protected param::SimpleParametrisation {
public:

    InheritFill(const std::string& fill="");

    /// Find best matching "filling" key
    bool pick(const InheritFill* who, const std::string& fill) const;

    /// Collect all inherited traits, prioritizing younger/children traits
    void inherit(param::SimpleParametrisation& who) const;

    void print(std::ostream&) const;

private:

    // No copy allowed
    InheritFill(const InheritFill&);
    InheritFill& operator=(const InheritFill&);

    // -- Members

    std::vector< const InheritFill* > children_;
    const std::string fill_;

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const InheritFill& p) {
        p.print(s);
        return s;
    }

};


}  // namespace config
}  // namespace mir


#endif
