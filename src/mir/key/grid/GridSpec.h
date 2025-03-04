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

#include "mir/key/grid/Grid.h"


namespace mir::key::grid {


class GridSpec : public Grid {
public:
    GridSpec(const std::string& key);

    const repres::Representation* representation() const override;
    const repres::Representation* representation(const util::Rotation&) const override;
    const repres::Representation* representation(const param::MIRParametrisation&) const override;
    size_t gaussianNumber() const override;

protected:
    void print(std::ostream& out) const override;
};


}  // namespace mir::key::grid
