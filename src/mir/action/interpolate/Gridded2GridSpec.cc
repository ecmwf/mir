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


#include "mir/action/interpolate/Gridded2UnrotatedGrid.h"

#include <memory>
#include <ostream>

#include "mir/key/grid/Grid.h"
#include "mir/util/EckitGeo.h"
#include "mir/util/Exceptions.h"


namespace mir::action::interpolate {


class Gridded2GridSpec : public Gridded2UnrotatedGrid {
public:
    explicit Gridded2GridSpec(const param::MIRParametrisation& param) : Gridded2UnrotatedGrid(param) {
        std::string gridspec;
        ASSERT(key::grid::Grid::get("grid", gridspec, param));

        grid_.reset(eckit::geo::GridFactory::make_from_string(gridspec));
        ASSERT(grid_);
    }

private:
    std::unique_ptr<const eckit::geo::Grid> grid_;

    bool sameAs(const Action& other) const override {
        const auto* o = dynamic_cast<const Gridded2GridSpec*>(&other);
        return (o != nullptr) && (grid_ == o->grid_) && Gridded2GriddedInterpolation::sameAs(other);
    }

    void print(std::ostream& out) const override {
        out << "Gridded2GridSpec[gridspec=" << grid_->spec_str() << ",";
        Gridded2UnrotatedGrid::print(out);
        out << "]";
    }

    const char* name() const override { return "Gridded2GridSpec"; }

    const repres::Representation* outputRepresentation() const override {
        NOTIMP;
        NOTIMP;
    }
};


static const ActionBuilder<Gridded2GridSpec> grid2grid("interpolate.grid2gridspec");


}  // namespace mir::action::interpolate
