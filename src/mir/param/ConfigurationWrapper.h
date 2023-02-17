/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#pragma once

#include "mir/param/MIRParametrisation.h"


namespace eckit {
class Configuration;
}


namespace mir::param {


class ConfigurationWrapper : public MIRParametrisation {

public:  // methods
    ConfigurationWrapper(const eckit::Configuration&);

    // From MIRParametrisation
    const MIRParametrisation& userParametrisation() const override;
    const MIRParametrisation& fieldParametrisation() const override;

    bool has(const std::string& name) const override;

    bool get(const std::string& name, std::string& value) const override;
    bool get(const std::string& name, bool& value) const override;
    bool get(const std::string& name, int& value) const override;
    bool get(const std::string& name, long& value) const override;
    bool get(const std::string& name, float& value) const override;
    bool get(const std::string& name, double& value) const override;

    bool get(const std::string& name, std::vector<int>& value) const override;
    bool get(const std::string& name, std::vector<long>& value) const override;
    bool get(const std::string& name, std::vector<float>& value) const override;
    bool get(const std::string& name, std::vector<double>& value) const override;
    bool get(const std::string& name, std::vector<std::string>& value) const override;

protected:  // methods
    void print(std::ostream&) const override;

private:  // members
    // Store a reference to the configuration, so that the wrapper can mimic a MIRParametrisation
    const eckit::Configuration& configuration_;
};


}  // namespace mir::param
