/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Feb 2017


#ifndef mir_config_MIRCompareConfiguration_h
#define mir_config_MIRCompareConfiguration_h

#include <iosfwd>
#include "eckit/filesystem/PathName.h"
#include "mir/config/AConfiguration.h"


namespace mir {
namespace config {


class MIRCompareConfiguration : public AConfiguration {
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

    static MIRCompareConfiguration& instance();

    // Lookup parametrisation according to metadata
    const param::MIRParametrisation* lookup(const param::MIRParametrisation& metadata) const;

    // Lookup parametrisation according to paramId and metadata
    const param::MIRParametrisation* lookup(const long& paramId, const param::MIRParametrisation& metadata) const;

    // -- Overridden methods

    // From AConfiguration

    // Configure (or reconfigure) using a file
    void configure(const eckit::PathName& path="~mir/etc/mir/mir-compare.yaml");

    // Return configuration defaults
    const param::MIRParametrisation* defaults() const;

private:

    // No copy allowed

    MIRCompareConfiguration(const MIRCompareConfiguration&);
    MIRCompareConfiguration& operator=(const MIRCompareConfiguration&);

    // -- Contructors

    MIRCompareConfiguration();

    // -- Destructor
    // None

    // -- Types
    // None

    // -- Members
    // None

    // -- Methods

    void print(std::ostream&) const;

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const MIRCompareConfiguration& p) {
        p.print(s);
        return s;
    }

};


}  // namespace config
}  // namespace mir


#endif
