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


#ifndef mir_action_transform_Sh2Gridded_h
#define mir_action_transform_Sh2Gridded_h

#include "atlas/internals/atlas_config.h"
#include "mir/action/plan/Action.h"

#ifdef ATLAS_HAVE_TRANS
#include "transi/trans.h"
#else
struct Trans_t {};
#endif


namespace atlas {
namespace grid {
class Grid;
}
}
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


class Sh2Gridded : public Action {
public:

    // -- Exceptions
    // None

    // -- Contructors
    Sh2Gridded(const param::MIRParametrisation&);

    // -- Destructor
    virtual ~Sh2Gridded();

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
    virtual void sh2grid(struct Trans_t& trans, data::MIRField& field) const = 0;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // No copy allowed
    Sh2Gridded(const Sh2Gridded&);
    Sh2Gridded& operator=(const Sh2Gridded&);

    // -- Members
    // None

    // -- Methods
    virtual const repres::Representation* outputRepresentation() const = 0;

    void transform(data::MIRField& field, const atlas::grid::Grid& grid, context::Context& ctx, const std::string& key, size_t truncation) const;
    void transform(data::MIRField& field, const atlas::grid::Grid& grid, context::Context& ctx) const;

    // -- Overridden methods
    virtual void execute(context::Context&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const Sh2GriddedTransform& p)
    //	{ p.print(s); return s; }

};


}  // namespace transform
}  // namespace action
}  // namespace mir


#endif
