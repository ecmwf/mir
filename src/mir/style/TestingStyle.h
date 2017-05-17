/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_style_TestingStyle_h
#define mir_style_TestingStyle_h

#include "mir/style/ECMWFStyle.h"


namespace mir {
namespace style {


class TestingStyle : public ECMWFStyle {
public:

    // -- Exceptions
    // None

    // -- Constructors
    TestingStyle(const param::MIRParametrisation& parametrisation);

    // -- Destructor
    ~TestingStyle();

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
    void print(std::ostream&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // No copy allowed
    TestingStyle(const TestingStyle&);
    TestingStyle& operator=(const TestingStyle&);

    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods
    void sh2grid(action::ActionPlan&) const {}
    bool forcedPrepare(const param::MIRParametrisation&) const { return true; }
    void prepare(action::ActionPlan&) const;
    void shTruncate(action::ActionPlan&) const {}

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None

};


}  // namespace style
}  // namespace mir


#endif

