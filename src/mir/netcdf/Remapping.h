// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "eckit/memory/Counted.h"

#include "mir/netcdf/HyperCube.h"


namespace mir::netcdf {


class Remapping : public HyperCube::Remapping, public eckit::Counted {

public:
    Remapping(size_t = 0);
    ~Remapping() override;

    void validate() const;

public:
    // -- Members
    // -- Methods

    virtual void print(std::ostream&) const;

    // - Friend
    friend std::ostream& operator<<(std::ostream& s, const Remapping& v) {
        v.print(s);
        return s;
    }
};


}  // namespace mir::netcdf
