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
#include "mir/param/MIRParametrisation.h"


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

    // Lookup parametrisation according to metadata
    const param::MIRParametrisation& lookup(const param::MIRParametrisation& metadata) const;

    // Lookup parametrisation according to paramId and metadata
    const param::MIRParametrisation& lookup(const long& paramId, const param::MIRParametrisation& metadata) const;

    // -- Overridden methods
    // None

    // -- Members

    static std::string path;
    static const std::string defaultPath;


private:

    // -- Contructors

    MIRConfiguration();

    // -- Destructor
    // None

    // -- Types
    // None

    // -- Members

    std::string configPath_;

    // -- Methods

    template<class T>
    bool _get(const std::string& name, T& value) const;

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

