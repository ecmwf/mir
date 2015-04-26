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


#ifndef MIRCombinedParametrisation_H
#define MIRCombinedParametrisation_H

#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace param {


class MIRCombinedParametrisation : public MIRParametrisation {
  public:

// -- Exceptions
    // None

// -- Contructors

    MIRCombinedParametrisation(const MIRParametrisation& user,
                               const MIRParametrisation& runtime,
                               const MIRParametrisation& metadata,
                               const MIRParametrisation& configuration,
                               const MIRParametrisation& defaults);

// -- Destructor

    ~MIRCombinedParametrisation(); // Change to virtual if base class

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

    // void print(ostream&) const; // Change to virtual if base class

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  private:

// No copy allowed

    MIRCombinedParametrisation(const MIRCombinedParametrisation&);
    MIRCombinedParametrisation& operator=(const MIRCombinedParametrisation&);

// -- Members
    const MIRParametrisation& user_;
    const MIRParametrisation& runtime_;
    const MIRParametrisation& metadata_;
    const MIRParametrisation& configuration_;
    const MIRParametrisation& defaults_;

// -- Methods
    // None

    template<class T>
    bool _get(const std::string&, T&) const;

// -- Overridden methods

    // From MIRParametrisation
    virtual void print(std::ostream&) const;
    virtual bool get(const std::string&, std::string&) const;
    virtual bool get(const std::string& name, bool& value) const;
    virtual bool get(const std::string& name, long& value) const;
    virtual bool get(const std::string& name, double& value) const;
    virtual bool get(const std::string& name, std::vector<long>& value) const;
    virtual bool get(const std::string& name, std::vector<double>& value) const;

    virtual bool has(const std::string& name) const;

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const MIRCombinedParametrisation& p)
    //	{ p.print(s); return s; }

};


}  // namespace param
}  // namespace mir
#endif

