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


#ifndef BitmapFilter_H
#define BitmapFilter_H

#include "mir/action/plan/Action.h"


namespace mir {
namespace util {
class Bitmap;
}
namespace action {


class BitmapFilter : public Action {
  public:

// -- Exceptions
    // None

// -- Contructors

    BitmapFilter(const param::MIRParametrisation&);

// -- Destructor

    virtual ~BitmapFilter(); // Change to virtual if base class

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

    void print(std::ostream&) const; // Change to virtual if base class

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  private:

// No copy allowed

    BitmapFilter(const BitmapFilter&);
    BitmapFilter& operator=(const BitmapFilter&);

// -- Members

    std::string path_;

// -- Methods

    util::Bitmap& bitmap() const;

// -- Overridden methods

    virtual void execute(context::Context & ctx) const;
    virtual bool sameAs(const Action& other) const;
    virtual const char* name() const;


// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const BitmapFilter& p)
    //	{ p.print(s); return s; }

};


}  // namespace action
}  // namespace mir
#endif

