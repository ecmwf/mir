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


#ifndef Copy_H
#define Copy_H

#include "mir/action/Action.h"

namespace mir {
namespace input {
class MIRInput;
}
namespace output {
class MIROutput;
}
namespace action {


class Copy : public Action {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    Copy(const param::MIRParametrisation &, input::MIRInput &input, output::MIROutput &output);

    // -- Destructor

    virtual ~Copy(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    virtual void execute(data::MIRField &) const;

    // -- Class members
    // None

    // -- Class methods
    // None

  protected:

    // -- Members
    // None

    // -- Methods

    void print(std::ostream &) const; // Change to virtual if base class

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  private:

    // No copy allowed

    Copy(const Copy &);
    Copy &operator=(const Copy &);

    // -- Members

    input::MIRInput &input_;
    output::MIROutput &output_;

    // -- Methods
    // None

    // -- Overridden methods

    virtual bool needField() const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const Copy& p)
    //  { p.print(s); return s; }

};


}  // namespace action
}  // namespace mir
#endif

