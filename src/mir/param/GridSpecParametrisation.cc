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


#include "mir/param/GridSpecParametrisation.h"

#include "eckit/geo/grid/ReducedGaussian.h"
#include "eckit/geo/grid/RegularGaussian.h"
#include "eckit/geo/grid/RegularLL.h"
#include "eckit/parser/YAMLParser.h"

#include "mir/util/Exceptions.h"


namespace mir::param {


GridSpecParametrisation::GridSpecParametrisation(const eckit::geo::Grid& grid) :
    custom_(eckit::geo::spec::Custom::make_from_value(eckit::YAMLParser::decodeString(grid.spec_str()))) {
    ASSERT(custom_);

    ASSERT(!custom_->has_custom("area"));        // FIXME current limitation
    ASSERT(!custom_->has_custom("projection"));  // FIXME current limitation(includes rotation)

    const auto type{grid.type()};

    if (type == "regular-ll") {
        const auto& ll = dynamic_cast<const eckit::geo::grid::RegularLL&>(grid);
        custom_->set("gridType", "regular_ll");

        if (!has("west_east_increment") && !has("south_north_increment")) {
            custom_->set("west_east_increment", ll.dlon());
            custom_->set("south_north_increment", ll.dlat());
        }

        if (!has("Ni") && !has("Nj")) {
            custom_->set("Ni", ll.nlon());
            custom_->set("Nj", ll.nlat());
        }
    }

    else if (type == "regular-gg") {
        const auto& gg = dynamic_cast<const eckit::geo::grid::RegularGaussian&>(grid);
        custom_->set("gridType", "regular_gg");

        if (!has("N")) {
            custom_->set("N", gg.N());
        }
    }

    else if (type == "reduced-gg") {
        const auto& gg = dynamic_cast<const eckit::geo::grid::ReducedGaussian&>(grid);
        custom_->set("gridType", "reduced_gg");

        if (!has("N")) {
            custom_->set("N", gg.N());
        }

        if (!has("pl")) {
            custom_->set("pl", gg.pl());
        }

        if (!has("Nj")) {
            custom_->set("Nj", gg.nj());
        }
    }
    else {
        throw exception::UserError("GridSpecParametrisation: unsupported type: '" + type + "'");
    }

    if (!has("north") && !has("west") && !has("south") && !has("east")) {
        const auto& bbox = grid.boundingBox();

        custom_->set("north", bbox.north);
        custom_->set("west", bbox.west);
        custom_->set("south", bbox.south);
        custom_->set("east", bbox.east);
    }

    ASSERT(has("gridType"));
}


void GridSpecParametrisation::print(std::ostream& out) const {
    out << *custom_;
}


}  // namespace mir::param
