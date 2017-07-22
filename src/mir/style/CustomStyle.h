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
/// @date June 2017


#ifndef mir_style_CustomStyle_h
#define mir_style_CustomStyle_h

#include "mir/style/MIRStyle.h"


namespace mir {
namespace style {


class CustomStyle : public MIRStyle {
public:

    // -- Exceptions
    // None

    // -- Contructors

    CustomStyle(const param::MIRParametrisation&);

    // -- Destructor

    ~CustomStyle(); // Change to virtual if base class

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

    // -- Overridden methods
    virtual void print(std::ostream&) const;


    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // No copy allowed

    CustomStyle(const CustomStyle&);
    CustomStyle& operator=(const CustomStyle&);

    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods

    virtual void prepare(action::ActionPlan&) const;

    virtual bool forcedPrepare(const api::MIRJob& job,
                               const param::MIRParametrisation& input) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    // friend std::ostream& operator<<(std::ostream& s, const CustomStyle& p)
    // { p.print(s); return s; }

};


}  // namespace style
}  // namespace mir


#endif

