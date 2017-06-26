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


#ifndef CustomParametrisation_H
#define CustomParametrisation_H

#include "mir/param/MIRParametrisation.h"
#include <vector>


namespace mir {

namespace param {
class MIRParametrisation;
}

namespace style {


class CustomParametrisation : public param::MIRParametrisation {
public:

// -- Exceptions
    // None

// -- Contructors

    CustomParametrisation( const std::string& name,
        const std::map<std::string, std::vector<std::string> >& params,
                           const param::MIRParametrisation &parametrisation);

// -- Destructor

    virtual ~CustomParametrisation(); // Change to virtual if base class

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



// -- Methods

    template<class T>
    bool _get(const std::string& name,  T& value) const;


// -- Overridden methods
    // virtual bool has(const std::string& name) const;

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

    virtual void print(std::ostream&) const; // Change to virtual if base class


// -- Class members
    // None

// -- Class methods
    // None

private:

// No copy allowed

    CustomParametrisation(const CustomParametrisation&);
    CustomParametrisation& operator=(const CustomParametrisation&);

// -- Members

    std::string name_;

    std::map<std::string, std::vector<std::string> > params_;
    const param::MIRParametrisation &parametrisation_;

// -- Methods
    // None

// -- Overridden methods
    // None


// -- Class members
    // None

// -- Class methods
    // None

// -- Friends


};


}  // namespace param
}  // namespace mir
#endif

