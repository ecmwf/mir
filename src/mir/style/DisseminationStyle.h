/*
 * (C) Copyright 1996-2017 ECMWF.
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


#ifndef mir_style_DisseminationStyle_h
#define mir_style_DisseminationStyle_h

#include "mir/style/ECMWFStyle.h"


namespace mir {
namespace style {


class DisseminationStyle : public ECMWFStyle {
public:

    // -- Exceptions
    // None

    // -- Contructors

    DisseminationStyle(const param::MIRParametrisation&);

    // -- Destructor

    ~DisseminationStyle(); // Change to virtual if base class

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
    DisseminationStyle(const DisseminationStyle&);
    DisseminationStyle& operator=(const DisseminationStyle&);

    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods

    void sh2grid(action::ActionPlan&) const;
    void shTruncate(action::ActionPlan&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    // friend std::ostream& operator<<(std::ostream& s, const DisseminationStyle& p)
    // { p.print(s); return s; }

};


}  // namespace style
}  // namespace mir


#endif

