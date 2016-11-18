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


#ifndef mir_config_MIRConfiguration_h
#define mir_config_MIRConfiguration_h

#include <algorithm>
#include <iosfwd>
#include <vector>
#include "eckit/memory/ScopedPtr.h"
#include "eckit/value/Value.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/config/InheritFill.h"
#include "mir/config/InheritParam.h"


namespace mir {
namespace param {
class MIRParametrisation;
}
}


namespace mir {
namespace config {


class MIRConfiguration {
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

    static const MIRConfiguration& instance();

    const param::MIRParametrisation* lookup(const long& paramId, const param::MIRParametrisation& metadata) const;

private:

    // No copy allowed

    MIRConfiguration(const MIRConfiguration&);
    MIRConfiguration& operator=(const MIRConfiguration&);

    // -- Contructors

    MIRConfiguration();

    // -- Destructor
    // None

    // -- Types
    // None

    // -- Members

    std::string configFill_;
    std::string configFile_;
    std::string configDir_;

    eckit::ScopedPtr<InheritParam> root_;
    eckit::ScopedPtr<InheritFill> fill_;

    // -- Methods

    bool parseInheritMap(InheritParam*, const eckit::ValueMap&) const;

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
