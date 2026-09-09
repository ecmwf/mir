// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/action/interpolate/Gridded2UnrotatedGrid.h"

#include <memory>

#include "mir/param/GridSpecParametrisation.h"


namespace mir::action::interpolate {


class Gridded2GridSpec : public Gridded2UnrotatedGrid {
public:
    explicit Gridded2GridSpec(const param::MIRParametrisation&);

private:
    std::unique_ptr<param::GridSpecParametrisation> param_;

    bool sameAs(const Action&) const override;
    void print(std::ostream&) const override;
    const char* name() const override;
    const repres::Representation* outputRepresentation() const override;
};


}  // namespace mir::action::interpolate
