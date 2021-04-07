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

#include "eckit/memory/Counted.h"

#include "mir/netcdf/HyperCube.h"


namespace mir {
namespace netcdf {


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


}  // namespace netcdf
}  // namespace mir
