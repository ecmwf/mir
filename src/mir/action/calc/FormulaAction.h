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


#ifndef mir_action_calcl_FormulaAction_h
#define mir_action_calcl_FormulaAction_h

#include <map>
#include <memory>

#include "mir/action/plan/Action.h"


namespace mir {
namespace util {
class Formula;
}
}  // namespace mir


namespace mir {
namespace action {


class FormulaAction : public Action {
public:
    // -- Exceptions
    // None

    // -- Constructors

    FormulaAction(const param::MIRParametrisation&);

    // -- Destructor

    virtual ~FormulaAction() override;  // Change to virtual if base class

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

    void print(std::ostream&) const override;  // Change to virtual if base class

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // No copy allowed

    FormulaAction(const FormulaAction&);
    FormulaAction& operator=(const FormulaAction&);

    // -- Members

    std::unique_ptr<util::Formula> formula_;
    std::map<std::string, long> metadata_;

    // -- Methods
    // None

    // -- Overridden methods

    virtual void execute(context::Context& ctx) const override;
    virtual bool sameAs(const Action& other) const override;
    virtual const char* name() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    // friend ostream& operator<<(ostream& s,const FormulaAction& p)
    //	{ p.print(s); return s; }
};


}  // namespace action
}  // namespace mir
#endif
