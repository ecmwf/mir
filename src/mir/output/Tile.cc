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


#include "mir/output/Tile.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "eckit/filesystem/PathName.h"
#include "mir/action/area/AreaCropper.h"
#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/output/PNGOutput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::output {


namespace {


struct Tiling {
    explicit Tiling(const param::MIRParametrisation& param) {
        std::vector<long> tile;
        ASSERT(param.get("tile", tile));

        if (tile.size() == 4) {
            single = true;

            ASSERT(1 <= tile[0]);
            ASSERT(1 <= tile[1]);
            Ti = static_cast<size_t>(tile[0]);
            Tj = static_cast<size_t>(tile[1]);

            ASSERT(0 <= tile[2] && tile[2] < Ti);
            ASSERT(0 <= tile[3] && tile[3] < Tj);
            i = static_cast<size_t>(tile[2]);
            j = static_cast<size_t>(tile[3]);

            return;
        }

        if (tile.size() == 2) {
            single = false;

            ASSERT(1 <= tile[0]);
            ASSERT(1 <= tile[1]);
            Ti = static_cast<size_t>(tile[0]);
            Tj = static_cast<size_t>(tile[1]);
            i  = 0;
            j  = 0;

            return;
        }

        throw exception::BadValue("Tile: invalid tiling", Here());
    }

    friend bool operator==(const Tiling& a, const Tiling& b) {
        return a.Ti == b.Ti && a.Tj == b.Tj && a.i == b.i && a.j == b.j && a.single == b.single;
    }

    friend std::ostream& operator<<(std::ostream& out, const Tiling& tiling) {
        if (tiling.single) {
            out << "Tile[Ni=" << tiling.Ti << ",Nj=" << tiling.Tj << ",i=" << tiling.i << ",j=" << tiling.j << "]";
        }
        else {
            out << "Tile[Ni=" << tiling.Ti << ",Nj=" << tiling.Tj << "]";
        }
        return out;
    }

    struct tile_type {
        tile_type(size_t _i, size_t _j, const util::BoundingBox& _bbox) : i(_i), j(_j), bbox(_bbox) {}

        std::vector<double> make_area() const {
            return {bbox.north().value(), bbox.west().value(), bbox.south().value(), bbox.east().value()};
        }

        size_t i;
        size_t j;
        util::BoundingBox bbox;
    };

    std::vector<tile_type> make_tiles(const util::BoundingBox& bbox) const {
        std::vector<tile_type> tiles;

        for (size_t j = 0; j < Tj; ++j) {
            for (size_t i = 0; i < Ti; ++i) {
                const auto north = bbox.north().fraction();
                const auto dlat  = (north - bbox.south().fraction()) / Tj;

                const auto west = bbox.west().fraction();
                const auto dlon = (bbox.east().fraction() - west) / Ti;

                const Latitude n  = north - j * dlat;
                const Longitude w = west + i * dlon;
                const Latitude s  = n - dlat;
                const Longitude e = w + dlon;

                tiles.emplace_back(i, j, util::BoundingBox{n, w, s, e});
            }
        }

        return tiles;
    }

    size_t Ti;
    size_t Tj;
    size_t i;
    size_t j;
    bool single;
};


std::string get_path(const std::string& path, size_t i, size_t j) {
    const eckit::PathName p(path);

    std::ostringstream s;
    s << p.dirName() / p.baseName(false);
    s << "." << std::setw(4) << std::setfill('0') << i;
    s << "." << std::setw(4) << std::setfill('0') << j;
    s << p.extension();

    return s.str();
};


}  // namespace


size_t Tile::save(const param::MIRParametrisation& param, context::Context& ctx) {
    Tiling tiling(param);

    Log::info() << "Tile::save " << tiling << std::endl;

    auto& field = ctx.field();
    ASSERT(field.dimensions() == 1);

    repres::RepresentationHandle repres(field.representation());
    auto tiles = tiling.make_tiles(repres->domain());
    ASSERT(tiles.size() == tiling.Ti * tiling.Tj);

    if (tiling.single) {
        decltype(tiles) one{tiles[tiling.j * tiling.Ti + tiling.i]};
        ASSERT(one[0].i == tiling.i);
        ASSERT(one[0].j == tiling.j);

        one.swap(tiles);
    }

    std::string kml;
    param.get("tile-kml", kml);

    for (const auto& tile : tiles) {
        // set context
        param::RuntimeParametrisation runtime_user(param.userParametrisation());

        const std::vector<double> area(tile.make_area());
        runtime_user.set("area", area);

        runtime_user.unset("format");
        if (std::string format; param.get("tile-format", format)) {
            runtime_user.set("format", format);
        }

        static const param::DefaultParametrisation defaults;
        param::CombinedParametrisation runtime(runtime_user, param.fieldParametrisation(), defaults);

        // crop
        context::Context local(ctx);
        std::unique_ptr<action::Action>(new action::AreaCropper(runtime))->perform(local);

        // save
        auto path = get_path(path_, tile.j, tile.i);
        std::unique_ptr<output::MIROutput> output(output::MIROutputFactory::build(path, runtime));
        output->save(runtime, local);
    }

    if (!kml.empty()) {
        std::ofstream file(kml);

        file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
             << "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n"
             << "<Document>\n"
             << "  <name>Image Grid Overlay</name>\n";

        for (const auto& tile : tiles) {
            const auto img = get_path(path_, tile.j, tile.i);
            file << "  <GroundOverlay>\n"
                 << "    <name>" << img << "</name>\n"
                 << "    <Icon>\n"
                 << "      <href>" << img << "</href>\n"
                 << "    </Icon>\n"
                 << "    <LatLonBox>\n"
                 << "      <north>" << tile.bbox.north().value() << "</north>\n"
                 << "      <south>" << tile.bbox.south().value() << "</south>\n"
                 << "      <east>" << tile.bbox.east().value() << "</east>\n"
                 << "      <west>" << tile.bbox.west().value() << "</west>\n"
                 << "    </LatLonBox>\n"
                 << "  </GroundOverlay>\n";
        }

        // Write KML footer
        file << "</Document>\n"
             << "</kml>\n";
    }

    return tiles.size();
}


bool Tile::sameAs(const MIROutput& other) const {
    const auto* o = dynamic_cast<const Tile*>(&other);
    return (o != nullptr);
}


bool Tile::sameParametrisation(const param::MIRParametrisation& param1, const param::MIRParametrisation& param2) const {
    return Tiling{param1} == Tiling{param2};
}


bool Tile::printParametrisation(std::ostream& out, const param::MIRParametrisation& param) const {
    out << "tile=" << Tiling{param};
    return true;
}


void Tile::print(std::ostream& out) const {
    out << "Tile[]";
}


static const MIROutputBuilder<Tile> output("tile");


}  // namespace mir::output
