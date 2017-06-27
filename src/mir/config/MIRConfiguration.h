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
#include "eckit/filesystem/PathName.h"
#include "eckit/memory/NonCopyable.h"
#include "eckit/memory/ScopedPtr.h"
#include "mir/config/ParameterConfiguration.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace config {


class MIRConfiguration : public param::MIRParametrisation, private eckit::NonCopyable {
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

    // Configure (or reconfigure) using a file
    void configure(const eckit::PathName& path="~mir/etc/mir/config.yaml");

    // -- Overridden methods

    // For MIRParametrisation
    bool has(const std::string& name) const;

    bool get(const std::string& name, std::string& value) const;
    bool get(const std::string& name, bool& value) const;
    bool get(const std::string& name, int& value) const;
    bool get(const std::string& name, long& value) const;
    bool get(const std::string& name, float& value) const;
    bool get(const std::string& name, double& value) const;

    bool get(const std::string& name, std::vector<int>& value) const;
    bool get(const std::string& name, std::vector<long>& value) const;
    bool get(const std::string& name, std::vector<float>& value) const;
    bool get(const std::string& name, std::vector<double>& value) const;
    bool get(const std::string& name, std::vector<std::string>& value) const;

private:

    // -- Contructors

    MIRConfiguration();

    // -- Destructor
    // None

    // -- Types
    // None

    // -- Members

    std::string configPath_;
    param::InheritParametrisation root_;
    eckit::ScopedPtr<ParameterConfiguration> parameterConfiguration_;

    // -- Methods

    template<class T>
    bool _get(const std::string& name, T& value) const;

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

