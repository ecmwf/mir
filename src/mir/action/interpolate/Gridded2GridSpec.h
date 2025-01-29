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
