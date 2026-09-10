// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>

#include "eckit/config/Parametrisation.h"


namespace mir::param {


class MIRParametrisation : public eckit::Parametrisation {
public:
    // -- Exceptions
    // None

    // -- Constructors

    MIRParametrisation();
    MIRParametrisation(const MIRParametrisation&) = delete;
    MIRParametrisation(MIRParametrisation&&)      = delete;

    // -- Destructor

    ~MIRParametrisation() override;

    // -- Convertors
    // None

    // -- Operators

    void operator=(const MIRParametrisation&) = delete;
    void operator=(MIRParametrisation&&)      = delete;

    // -- Methods

    virtual const MIRParametrisation& userParametrisation() const;
    virtual const MIRParametrisation& fieldParametrisation() const;

    // -- Overridden methods

    // From eckit::Parametrisation
    bool has(const std::string& name) const override = 0;

    bool get(const std::string& name, std::string& value) const override = 0;
    bool get(const std::string& name, bool& value) const override        = 0;
    bool get(const std::string& name, int& value) const override         = 0;
    bool get(const std::string& name, long& value) const override        = 0;
    bool get(const std::string& name, float& value) const override       = 0;
    bool get(const std::string& name, double& value) const override      = 0;

    bool get(const std::string& name, std::vector<int>& value) const override         = 0;
    bool get(const std::string& name, std::vector<long>& value) const override        = 0;
    bool get(const std::string& name, std::vector<float>& value) const override       = 0;
    bool get(const std::string& name, std::vector<double>& value) const override      = 0;
    bool get(const std::string& name, std::vector<std::string>& value) const override = 0;

    bool get(const std::string& name, long long& value) const override;
    bool get(const std::string& name, size_t& value) const override;

    bool get(const std::string& name, std::vector<bool>& value) const override;
    bool get(const std::string& name, std::vector<long long>& value) const override;
    bool get(const std::string& name, std::vector<size_t>& value) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members
    // None

    // -- Methods

    virtual void print(std::ostream&) const = 0;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const MIRParametrisation& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::param
