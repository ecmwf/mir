// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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
