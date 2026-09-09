// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/data/space/SpaceLinear.h"


namespace mir::data::space {


class Space2DVectorZonalMeridional : public SpaceLinear {

    // -- Overridden methods

    size_t dimensions() const override;
};


}  // namespace mir::data::space
