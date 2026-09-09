// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/util/Types.h"

namespace mir {


struct LatitudeIncrement {
    explicit LatitudeIncrement(const Latitude& increment = Latitude(0)) : increment_(increment) {}
    const LatitudeIncrement& operator=(const Latitude& l) {
        increment_ = l;
        return *this;
    }
    const Latitude& latitude() const { return increment_; }

private:
    Latitude increment_;
};


}  // namespace mir
