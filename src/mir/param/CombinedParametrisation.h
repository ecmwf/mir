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

#include "mir/param/DefaultParametrisation.h"
#include "mir/param/MIRParametrisation.h"


namespace mir::param {


class CombinedParametrisation : public MIRParametrisation {
public:
    // -- Exceptions
    // None

    // -- Constructors

    CombinedParametrisation(const MIRParametrisation& userParametrisation,
                            const MIRParametrisation& metadata = DefaultParametrisation::instance(),
                            const MIRParametrisation& defaults = DefaultParametrisation::instance());

    // -- Destructor

    ~CombinedParametrisation() override;

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
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    const MIRParametrisation& user_;
    const MIRParametrisation& field_;
    const MIRParametrisation& defaults_;

    // -- Methods
    // None

    template <class T>
    bool _get(const std::string&, T&) const;

    // -- Overridden methods

    const MIRParametrisation& userParametrisation() const override;
    const MIRParametrisation& fieldParametrisation() const override;

    // From MIRParametrisation
    void print(std::ostream&) const override;

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

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::param
