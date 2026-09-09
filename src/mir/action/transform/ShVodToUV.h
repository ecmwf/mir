// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/action/plan/Action.h"
#include "mir/util/Atlas.h"
#include "mir/util/Types.h"


namespace mir::action::transform {


class ShVodToUV : public Action {
public:
    // -- Types

    using atlas_config_t = atlas::util::Config;

    // -- Exceptions
    // None

    // -- Constructors

    explicit ShVodToUV(const param::MIRParametrisation&);

    // -- Destructor
    // None

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

private:
    // -- Members

    atlas_config_t options_;

    // -- Methods
    // None

    // -- Overridden methods

    void print(std::ostream&) const override;

    bool sameAs(const Action&) const override;
    void execute(context::Context&) const override;
    const char* name() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::action::transform
