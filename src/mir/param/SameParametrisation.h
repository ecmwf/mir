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


#ifndef mir_param_SameParametrisation_h
#define mir_param_SameParametrisation_h

#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace param {


class SameParametrisation : public MIRParametrisation {
public:

    // -- Exceptions
    // None

    // -- Contructors

    SameParametrisation(const MIRParametrisation&, const MIRParametrisation&, bool strict);

    // -- Destructor
    // None

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

    const MIRParametrisation& parametrisation1_;
    const MIRParametrisation& parametrisation2_;
    const bool strict_;

    // -- Methods

    template<class T>
    bool _get(const std::string&, T&) const;

    // -- Overridden methods

    // From MIRParametrisation
    void print(std::ostream&) const;

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

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None

};


}  // namespace param
}  // namespace mir


#endif

