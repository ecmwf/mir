// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <map>
#include <vector>

#include "mir/param/MIRParametrisation.h"


namespace mir::key::style {


class CustomParametrisation : public param::MIRParametrisation {
public:
    // -- Exceptions
    // None

    // -- Constructors

    CustomParametrisation(const std::string& name, const std::map<std::string, std::vector<std::string> >& params,
                          const param::MIRParametrisation& parametrisation);

    // -- Destructor

    ~CustomParametrisation() override;

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members
    // None

    // -- Methods

    template <class T>
    bool _get(const std::string& name, T& value) const;

    // -- Overridden methods

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

    void print(std::ostream&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    std::string name_;
    std::map<std::string, std::vector<std::string> > params_;
    const param::MIRParametrisation& parametrisation_;

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::key::style
