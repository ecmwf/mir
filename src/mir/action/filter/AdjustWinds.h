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


#ifndef AdjustWinds_H
#define AdjustWinds_H

#include "mir/action/plan/Action.h"
#include "mir/util/Rotation.h"
#include <vector>

namespace mir {

namespace repres { class Representation; }


namespace action {


class AdjustWinds : public Action {
  public:

// -- Exceptions
    // None

// -- Contructors

    AdjustWinds(const param::MIRParametrisation&);

// -- Destructor

    virtual ~AdjustWinds(); // Change to virtual if base class

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

    AdjustWinds(const AdjustWinds&);
    AdjustWinds& operator=(const AdjustWinds&);

// -- Members
    util::Rotation rotation_;

// -- Methods
    // None

    void windDirections(const repres::Representation* representation, std::vector<double> &result) const ;

// -- Overridden methods

    virtual void execute(data::MIRField & field, util::MIRStatistics& statistics) const;
    virtual bool sameAs(const Action& other) const;


// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const AdjustWinds& p)
    //	{ p.print(s); return s; }

};


}  // namespace action
}  // namespace mir
#endif

