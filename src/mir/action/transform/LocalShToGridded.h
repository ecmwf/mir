/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_action_transform_LocalShToGridded_h
#define mir_action_transform_LocalShToGridded_h

#include "atlas/option/TransOptions.h"
#include "mir/action/plan/Action.h"
#include "mir/api/Atlas.h"


namespace mir {
namespace data {
class MIRField;
}
namespace repres {
class Representation;
}
}


namespace mir {
namespace action {
namespace transform {


class LocalShToGridded : public Action {
public:

    // -- Exceptions
    // None

    // -- Contructors

    LocalShToGridded(const param::MIRParametrisation&);

    // -- Destructor

    virtual ~LocalShToGridded();

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

    virtual void setTransOptions(atlas::util::Config&) const = 0;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // -- Members
    // None

    // -- Methods
    
    virtual void sh2grid(data::MIRField&, atlas::trans::Trans&, const atlas::Grid&) const = 0;

    virtual const repres::Representation* outputRepresentation() const = 0;

    void transform(data::MIRField& field,
                   const repres::Representation& representation,
                   context::Context& ctx) const;

    // -- Overridden methods
    
    virtual void execute(context::Context&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const LocalShToGriddedTransform& p)
    //  { p.print(s); return s; }

};


}  // namespace transform
}  // namespace action
}  // namespace mir


#endif
