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


#pragma once

#include "mir/action/plan/Action.h"
#include "mir/method/Cropping.h"
#include "mir/util/Atlas.h"
#include "mir/util/Types.h"


namespace mir {
namespace data {
class MIRField;
}
namespace repres {
class Representation;
}
}  // namespace mir


namespace mir::action::transform {


class ShToGridded : public Action {
public:
    // -- Types

    using atlas_trans_t  = atlas::trans::Trans;
    using atlas_config_t = atlas::util::Config;

    // -- Exceptions
    // None

    // -- Constructors

    explicit ShToGridded(const param::MIRParametrisation&);

    // -- Destructor

    ~ShToGridded() override;

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
    // None

    // -- Overridden methods

    void print(std::ostream&) const override  = 0;
    bool sameAs(const Action&) const override = 0;
    void estimate(context::Context&, api::MIREstimation&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    method::Cropping cropping_;
    atlas_config_t options_;

    // -- Methods

    virtual void sh2grid(data::MIRField&, const atlas_trans_t&, const param::MIRParametrisation&) const = 0;

    virtual const repres::Representation* outputRepresentation() const = 0;

    void transform(data::MIRField&, const repres::Representation&, context::Context&) const;

    // -- Overridden methods

    void execute(context::Context&) const override;

    bool mergeWithNext(const Action&) override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::action::transform
