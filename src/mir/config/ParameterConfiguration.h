/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date June 2017


#ifndef mir_config_ParameterConfiguration_h
#define mir_config_ParameterConfiguration_h

#include <iosfwd>
#include "eckit/memory/NonCopyable.h"
#include "mir/param/InheritParametrisation.h"


namespace eckit {
class PathName;
}
namespace mir {
namespace param {
class MIRParametrisation;
}
}


namespace mir {
namespace config {


class ParameterConfiguration : private eckit::NonCopyable {
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

    static ParameterConfiguration& instance();

    // Lookup parametrisation according to metadata
    const param::MIRParametrisation& lookup(const param::MIRParametrisation& metadata) const;

    // Lookup parametrisation according to paramId and metadata
    const param::MIRParametrisation& lookup(const long& paramId, const param::MIRParametrisation& metadata) const;

    // Configure (or reconfigure) using a file
    void configure(const eckit::PathName& path="~mir/etc/mir/parameter.yaml");

    // -- Overridden methods
    // None

private:

    // -- Contructors

    ParameterConfiguration();

    // -- Destructor
    // None

    // -- Types
    // None

    // -- Members

    std::string configPath_;
    param::InheritParametrisation root_;

    // -- Methods

    void print(std::ostream&) const;

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const ParameterConfiguration& p) {
        p.print(s);
        return s;
    }

};


}  // namespace config
}  // namespace mir


#endif

