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

#include <string>

#include "mir/param/MIRParametrisation.h"
#include "mir/param/SimpleParametrisation.h"


namespace mir::input {
class GribInput;
}  // namespace mir::input


namespace mir::param {


class CachedParametrisation : public MIRParametrisation {
public:
    // -- Exceptions
    // None

    // -- Constructors

    CachedParametrisation(MIRParametrisation& parametrisation);

    // -- Destructor

    ~CachedParametrisation() override;

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    void reset();

    void set(const std::string& name, bool value);
    void set(const std::string& name, int value);
    void set(const std::string& name, long value);
    void set(const std::string& name, double value);
    void set(const std::string& name, const std::string& value);
    void set(const std::string& name, const char* value);

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members
    // None

    // -- Methods

    void print(std::ostream&) const override;

    // -- Overridden methods

    // From MIRParametrisation
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

private:
    // -- Types
    // None

    // -- Members

    MIRParametrisation& parametrisation_;
    mutable SimpleParametrisation cache_;

    // -- Methods

    template <class T>
    bool _get(const std::string& name, T& value) const;

    template <class T>
    void _set(const std::string& name, const T& value) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None

    friend input::GribInput;
};


}  // namespace mir::param
