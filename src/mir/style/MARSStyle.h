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


#ifndef mir_style_MARSStyle_h
#define mir_style_MARSStyle_h

#include "mir/style/ECMWFStyle.h"


namespace mir {
namespace style {


class MARSStyle : public ECMWFStyle {
public:

    // -- Exceptions
    // None

    // -- Contructors

    MARSStyle(const param::MIRParametrisation&);

    // -- Destructor

    ~MARSStyle(); // Change to virtual if base class

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
    MARSStyle(const MARSStyle&);
    MARSStyle& operator=(const MARSStyle&);

    // -- Members
    // None

    // -- Methods

    long getTargetGaussianNumber() const;
    long getIntendedTruncation() const;

    // -- Overridden methods

    void sh2grid(action::ActionPlan&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    // friend std::ostream& operator<<(std::ostream& s, const MARSStyle& p)
    // { p.print(s); return s; }

};


}  // namespace style
}  // namespace mir


#endif

