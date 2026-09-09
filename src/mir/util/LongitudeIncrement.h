// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/util/Types.h"

namespace mir {


struct LongitudeIncrement {
    explicit LongitudeIncrement(const Longitude& increment = Longitude(0)) : increment_(increment) {}
    const LongitudeIncrement& operator=(const Longitude& l) {
        increment_ = l;
        return *this;
    }
    const Longitude& longitude() const { return increment_; }

private:
    Longitude increment_;
};


}  // namespace mir
