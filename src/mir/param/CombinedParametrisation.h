/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#ifndef mir_param_CombinedParametrisation_h
#define mir_param_CombinedParametrisation_h

#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace param {


class CombinedParametrisation : public MIRParametrisation {
public:

    // -- Exceptions
    // None

    // -- Contructors

    CombinedParametrisation(const MIRParametrisation& userParametrisation,
                            const MIRParametrisation& metadata,
                            const MIRParametrisation& defaults);

    // -- Destructor

    virtual ~CombinedParametrisation(); // Change to virtual if base class

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

    template<class T>
    bool _get(const std::string&, T&) const;

    // -- Overridden methods

    virtual const MIRParametrisation& userParametrisation() const;
    virtual const MIRParametrisation& fieldParametrisation() const;

    // From MIRParametrisation
    virtual void print(std::ostream&) const;

    virtual bool has(const std::string& name) const;

    virtual bool get(const std::string& name, std::string& value) const;
    virtual bool get(const std::string& name, bool& value) const;
    virtual bool get(const std::string& name, int& value) const;
    virtual bool get(const std::string& name, long& value) const;
    virtual bool get(const std::string& name, float& value) const;
    virtual bool get(const std::string& name, double& value) const;

    virtual bool get(const std::string& name, std::vector<int>& value) const;
    virtual bool get(const std::string& name, std::vector<long>& value) const;
    virtual bool get(const std::string& name, std::vector<float>& value) const;
    virtual bool get(const std::string& name, std::vector<double>& value) const;
    virtual bool get(const std::string& name, std::vector<std::string>& value) const;

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

