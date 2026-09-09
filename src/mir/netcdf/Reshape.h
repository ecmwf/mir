// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "eckit/memory/Counted.h"

#include "mir/netcdf/HyperCube.h"


namespace mir::netcdf {


class Reshape : public eckit::Counted {
public:
    Reshape(const HyperCube& cube, size_t which, size_t where, size_t count, char tag);

    size_t operator()(size_t) const;

    bool merge(const Reshape&);

protected:
    ~Reshape() override;

private:
    // -- Members
    HyperCube::Dimensions cube_;
    HyperCube::Dimensions mul_;

    size_t which_;
    size_t where_;
    size_t count_;
    int size_;
    char tag_;

    // -- Methods

    void print(std::ostream&) const;

    // - Friend
    friend std::ostream& operator<<(std::ostream& s, const Reshape& v) {
        v.print(s);
        return s;
    }
};


}  // namespace mir::netcdf
