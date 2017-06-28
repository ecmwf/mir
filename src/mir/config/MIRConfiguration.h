/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Nov 2016


#ifndef mir_config_MIRConfiguration_h
#define mir_config_MIRConfiguration_h

#include <iosfwd>
#include <string>
#include "mir/param/InheritParametrisation.h"


namespace eckit {
class PathName;
}


namespace mir {
namespace config {


class MIRConfiguration : public param::InheritParametrisation {
public:

    // -- Exceptions
    // None

    // -- Contructors
    // None

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    static MIRConfiguration& instance();

    // -- Overridden methods
    // None

    // -- Members
    // None

private:

    // -- Contructors

    MIRConfiguration();

    // -- Destructor
    // None

    // -- Types
    // None

    // -- Members
    // None

    // -- Methods

    // Configure (or reconfigure) using a file
    void configure(const eckit::PathName& path);

    void print(std::ostream&) const;

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const MIRConfiguration& p) {
        p.print(s);
        return s;
    }

};


}  // namespace config
}  // namespace mir


#endif

