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
