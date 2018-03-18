/*
 * (C) Copyright 1996- ECMWF.
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


#ifndef mir_action_transform_ShToGridded_h
#define mir_action_transform_ShToGridded_h

#include "eckit/memory/ScopedPtr.h"
#include "eckit/utils/Hash.h"
#include "atlas/option/TransOptions.h"
#include "mir/action/plan/Action.h"
#include "mir/api/Atlas.h"
#include "mir/method/Cropping.h"
#include "mir/util/function/Function.h"


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


class ShToGridded : public Action {
public:

    // -- Types

    typedef atlas::trans::Trans atlas_trans_t;

    struct atlas_config_t : public atlas::util::Config  {
        using Config::Config;
        eckit::Hash::digest_t digest() const;
    };

    // -- Exceptions
    // None

    // -- Contructors

    ShToGridded(const param::MIRParametrisation&);

    // -- Destructor

    virtual ~ShToGridded();

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

    void local(bool l);
    bool local() const;

    // -- Overridden methods

    virtual void print(std::ostream&) const = 0;
    virtual bool sameAs(const Action& other) const = 0;

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // -- Members

    method::Cropping cropping_;
    atlas_config_t options_;
    eckit::ScopedPtr<util::function::Function> compressIf_;

    // -- Methods

    virtual void sh2grid(data::MIRField&, const atlas_trans_t&, const atlas::Grid&) const = 0;

    virtual const repres::Representation* outputRepresentation() const = 0;

    void transform(data::MIRField&, const repres::Representation&, context::Context&) const;

    // -- Overridden methods

    virtual void execute(context::Context&) const;

    virtual bool mergeWithNext(const Action&);

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const ShToGriddedTransform& p)
    //  { p.print(s); return s; }

};


}  // namespace transform
}  // namespace action
}  // namespace mir


#endif
