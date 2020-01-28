/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_style_ECMWFStyle_h
#define mir_style_ECMWFStyle_h

#include "mir/style/MIRStyle.h"


namespace mir {
namespace style {


class ECMWFStyle : public MIRStyle {
public:

    // -- Exceptions
    // None

    // -- Contructors

    ECMWFStyle(const param::MIRParametrisation&);

    // -- Destructor

    virtual ~ECMWFStyle();

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

    virtual void prologue(action::ActionPlan&) const;
    virtual void sh2grid(action::ActionPlan&) const;
    virtual void sh2sh(action::ActionPlan&) const;
    virtual void grid2grid(action::ActionPlan&) const;
    virtual void epilogue(action::ActionPlan&) const;

    // -- Overridden methods

    virtual void print(std::ostream&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods

    void prepare(action::ActionPlan&, input::MIRInput&, output::MIROutput&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    // friend std::ostream& operator<<(std::ostream& s, const ECMWFStyle& p)
    // { p.print(s); return s; }

};


}  // namespace style
}  // namespace mir


#endif

