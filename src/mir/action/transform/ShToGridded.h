// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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
