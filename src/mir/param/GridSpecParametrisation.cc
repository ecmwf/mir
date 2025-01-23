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

#include <memory>

#include "eckit/geo/grid/ReducedGaussian.h"
#include "eckit/geo/grid/RegularGaussian.h"
#include "eckit/geo/grid/RegularLL.h"
#include "eckit/geo/spec/Custom.h"
#include "eckit/parser/YAMLParser.h"

#include "mir/util/Exceptions.h"


namespace mir::param {


GridSpecParametrisation::GridSpecParametrisation(const std::string& gridspec) :
    GridSpecParametrisation(
        *std::unique_ptr<const eckit::geo::Grid>(eckit::geo::GridFactory::make_from_string(gridspec))) {}


GridSpecParametrisation::GridSpecParametrisation(const eckit::geo::Grid& grid) {
    std::unique_ptr<eckit::geo::spec::Custom> custom(
        eckit::geo::spec::Custom::make_from_value(eckit::YAMLParser::decodeString(grid.spec_str())));
    ASSERT(custom);

    ASSERT(!custom->has_custom("area"));        // FIXME current limitation
    ASSERT(!custom->has_custom("projection"));  // FIXME current limitation(includes rotation)

    const auto type{grid.type()};

    if (type == "regular-ll") {
        const auto& ll = dynamic_cast<const eckit::geo::grid::RegularLL&>(grid);
        set("gridType", "regular_ll");

        if (!has("west_east_increment") && !has("south_north_increment")) {
            set("west_east_increment", ll.dlon());
            set("south_north_increment", ll.dlat());
        }

        if (!has("Ni") && !has("Nj")) {
            set("Ni", ll.nlon());
            set("Nj", ll.nlat());
        }
    }

    else if (type == "regular-gg") {
        const auto& gg = dynamic_cast<const eckit::geo::grid::RegularGaussian&>(grid);
        set("gridType", "regular_gg");

        if (!has("N")) {
            set("N", gg.N());
        }
    }

    else if (type == "reduced-gg") {
        const auto& gg = dynamic_cast<const eckit::geo::grid::ReducedGaussian&>(grid);
        set("gridType", "reduced_gg");

        if (!has("N")) {
            set("N", gg.N());
        }

        if (!has("pl")) {
            set("pl", gg.pl());
        }

        if (!has("Nj")) {
            set("Nj", gg.nj());
        }
    }

    else {
        throw exception::UserError("GridSpecParametrisation: unsupported type: '" + type + "'");
    }

    if (!has("north") && !has("west") && !has("south") && !has("east")) {
        const auto& bbox = grid.boundingBox();

        set("north", bbox.north);
        set("west", bbox.west);
        set("south", bbox.south);
        set("east", bbox.east);
    }

    ASSERT(has("gridType"));
}


}  // namespace mir::param
