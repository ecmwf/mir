/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Simon Smart
/// @author Pedro Maciel
/// @date March 2016


#ifndef mir_param_ConfigurationWrapper_h
#define mir_param_ConfigurationWrapper_h

#include "mir/param/MIRParametrisation.h"


namespace eckit {
class Configuration;
}


namespace mir {
namespace param {


class ConfigurationWrapper : public MIRParametrisation {

public: // methods

    ConfigurationWrapper(const eckit::Configuration&);

    // From MIRParametrisation
    const MIRParametrisation& userParametrisation() const;
    const MIRParametrisation& fieldParametrisation() const;

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

protected: // methods

    virtual void print(std::ostream&) const;

private: // members

    // Store a reference to the configuration, so that the wrapper can mimic a MIRParametrisation
    const eckit::Configuration& configuration_;

};


}  // namespace param
}  // namespace mir


#endif
