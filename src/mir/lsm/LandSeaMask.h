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


#ifndef LandSeaMask_H
#define LandSeaMask_H

#include <iosfwd>


namespace atlas {
class Grid;
}

namespace mir {

namespace param {
class MIRParametrisation;
}


namespace data {
class MIRField;
}

namespace lsm {


class LandSeaMask {
  public:

// -- Exceptions
    // None

// -- Contructors

    LandSeaMask(const std::string& name);

// -- Destructor

    virtual ~LandSeaMask(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods

    virtual bool active() const = 0;
    virtual const data::MIRField& field(const atlas::Grid &) const = 0;
    virtual std::string unique_id(const atlas::Grid &) const = 0;

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  protected:

// -- Members

    std::string name_;

// -- Methods


    virtual void print(std::ostream&) const = 0; // Change to virtual if base class

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  private:

// No copy allowed

    LandSeaMask(const LandSeaMask&);
    LandSeaMask& operator=(const LandSeaMask&);

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

    friend std::ostream& operator<<(std::ostream& s,const LandSeaMask& p) {
        p.print(s);
        return s;
    }

};


}  // namespace logic
}  // namespace mir
#endif

