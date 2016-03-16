/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Simon Smart
/// @date March 2016


#ifndef mir_param_ConfigurationWrapper_H
#define mir_param_ConfigurationWrapper_H

#include <string>

#include "eckit/config/Configuration.h"

#include "mir/param/MIRParametrisation.h"



namespace eckit {
    class JSON;
}



namespace mir {
namespace param {


//----------------------------------------------------------------------------------------------------------------------


class ConfigurationWrapper : public MIRParametrisation {

public: // methods

    ConfigurationWrapper(eckit::Configuration&);
    virtual ~ConfigurationWrapper();


    virtual bool has(const std::string& name) const;

    virtual bool get(const std::string& name, std::string& value) const;
    virtual bool get(const std::string& name, bool& value) const;
    virtual bool get(const std::string& name, long& value) const;
    virtual bool get(const std::string& name, double& value) const;
    virtual bool get(const std::string& name, size_t& value) const;

    virtual bool get(const std::string& name, std::vector<long>& value) const;
    virtual bool get(const std::string& name, std::vector<double>& value) const;


protected: // methods

    virtual void print(std::ostream&) const;

private: // methods (not implemented)

    // Copying disallowed.
    ConfigurationWrapper(const ConfigurationWrapper&);
    ConfigurationWrapper& operator=(const ConfigurationWrapper&);

private: // members

    // Store a reference to the configuration, so that the wrapper can mimic a MIRParametrisation
    // on top.
    eckit::Configuration& configuration_;

};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace param
}  // namespace mir

#endif // mir_param_ConfigurationWrapper_H
