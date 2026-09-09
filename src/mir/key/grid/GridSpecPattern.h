// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/key/grid/GridPattern.h"


namespace mir::key::grid {


class GridSpecPattern : public GridPattern {
public:
    explicit GridSpecPattern(const std::string& pattern);

private:
    void print(std::ostream&) const override;
    const Grid* make(const std::string& name) const override;
    std::string canonical(const std::string& name) const override;
};


}  // namespace mir::key::grid
