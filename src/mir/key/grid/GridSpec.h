// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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
