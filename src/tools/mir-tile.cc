// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/geo/Grid.h"
#include "eckit/geo/grid/reduced/HEALPix.h"
#include "eckit/geo/order/HEALPix.h"
#include "eckit/log/JSON.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/FactoryOption.h"
#include "eckit/option/SimpleOption.h"

#include "mir/action/area/AreaCropper.h"
#include "mir/data/MIRField.h"
#include "mir/input/GridSpecInput.h"
#include "mir/key/Area.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Angles.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Exceptions.h"
#include "mir/util/IndexMapping.h"
#include "mir/util/Log.h"
#include "mir/util/Types.h"


namespace mir::tools {


struct MIRTile : MIRTool {
    MIRTile(int argc, char** argv) : MIRTool(argc, argv) {
        using eckit::option::FactoryOption;
        using eckit::option::SimpleOption;

        options_.push_back(new SimpleOption<std::string>("grid", "Grid"));
        options_.push_back(
            new SimpleOption<std::string>("tile", "Tiling: regular, leopardi (equal-regions) or healpix-nested"));
        options_.push_back(
            new SimpleOption<bool>("coordinates", "Output grid point latitudes/longitudes (default false)"));
        options_.push_back(new SimpleOption<bool>(
            "expand-indices", "Output tile indices as values only, not [first, last] ranges (default false)"));

        // regular
        options_.push_back(new FactoryOption<key::Area>("area", "regular: area to tile (default 90/0/-90/360)"));
        options_.push_back(new SimpleOption<size_t>("ni", "regular: number of tiles West-East (columns)"));
        options_.push_back(new SimpleOption<size_t>("nj", "regular: number of tiles North-South (rows)"));

        // leopardi
        options_.push_back(new SimpleOption<size_t>("regions", "leopardi/equal-regions: number of equal area regions"));

        // healpix-nested
        options_.push_back(new SimpleOption<size_t>("nside", "healpix-nested: tiles Nside (tiles of HEALPix pixels)"));
    }

    int numberOfPositionalArguments() const override { return 0; }

    void usage(const std::string& tool) const override {
        Log::info() << "\nGrid tiling, reporting grid point indices per tile (JSON, values and [first, last] ranges)."
                       "\nEach point belongs to one tile: on shared edges, the tile to the North and West."
                       "\n"
                       "\nUsage:"
                       "\n\t"
                    << tool
                    << " --grid=<gridspec> [--coordinates] [--expand-indices] --tile=<tiling> [tiling options]"
                       "\n"
                       "\nTilings:"
                       "\n\tregular         [--area=n/w/s/e] --ni=<columns> --nj=<rows>"
                       "\n\tleopardi        --regions=<N> (alias: equal-regions)"
                       "\n\thealpix-nested  --nside=<Nside> (HEALPix nested grids only, Nside < grid Nside)"
                       "\n"
                       "\nExamples:"
                       "\n\t"
                    << tool << " --grid='{grid: O320}' --tile=regular --area=60/-10/30/40 --ni=4 --nj=2"
                    << "\n\t" << tool << " --grid='{grid: 1/1}' --tile=leopardi --regions=32"
                    << "\n\t" << tool << " --grid='{grid: H128, order: nested}' --tile=healpix-nested --nside=4"
                    << std::endl;
    }

    void execute(const eckit::option::CmdArgs&) override;
};


using Crop    = std::function<std::vector<size_t>(const util::BoundingBox&)>;
using Indices = std::function<void(eckit::JSON&, const std::vector<size_t>&)>;


struct Tile {
    util::BoundingBox bbox;
    std::vector<size_t> indices;
};


std::vector<Tile> tiles_regular(const Crop& crop, const util::BoundingBox& area, size_t ni, size_t nj) {
    if (ni < 1 || nj < 1) {
        throw exception::UserError("mir-tile: --tile=regular requires positive 'ni' and 'nj'");
    }

    const auto n = area.north().value();
    const auto w = area.west().value();
    const auto s = area.south().value();
    const auto e = area.east().value();

    auto lat = [=](size_t j) { return j == nj ? s : n - (n - s) * static_cast<double>(j) / static_cast<double>(nj); };
    auto lon = [=](size_t i) { return i == ni ? e : w + (e - w) * static_cast<double>(i) / static_cast<double>(ni); };

    std::vector<Tile> tiles;
    tiles.reserve(ni * nj);

    for (size_t j = 0; j < nj; ++j) {
        for (size_t i = 0; i < ni; ++i) {
            util::BoundingBox bbox(lat(j), lon(i), lat(j + 1), lon(i + 1));
            tiles.push_back({bbox, crop(bbox)});
        }
    }

    return tiles;
}


// Recursive zonal equal area (EQ) sphere partitioning, for S^2 (polar caps and collars, no collar offsets)
// @ref P. Leopardi (2006) A partition of the unit sphere into regions of equal area and small diameter, Electronic
// Transactions on Numerical Analysis 25, 309-327
// @ref P. Leopardi, Recursive Zonal Equal Area (EQ) Sphere Partitioning Toolbox, eq_regions
struct EqualRegions {
    static double area_of_cap(double s_cap) { return 4. * M_PI * std::pow(std::sin(0.5 * s_cap), 2); }

    static double sradius_of_cap(double area) { return 2. * std::asin(0.5 * std::sqrt(area / M_PI)); }

    static double area_of_ideal_region(size_t N) { return 4. * M_PI / static_cast<double>(N); }

    static double polar_colat(size_t N) {
        return N == 1 ? M_PI : N == 2 ? 0.5 * M_PI : sradius_of_cap(area_of_ideal_region(N));
    }

    /// Number of regions per zone (caps and collars) and colatitudes of the (increasing) spherical caps
    static void eq_caps(size_t N, std::vector<size_t>& n_regions, std::vector<double>& s_cap) {
        if (N == 1) {
            n_regions = {1};
            s_cap     = {M_PI};
            return;
        }

        const auto c_polar = polar_colat(N);
        const auto a_ideal = std::sqrt(area_of_ideal_region(N));

        const size_t n_collars = N > 2 ? std::max(1L, std::lround((M_PI - 2. * c_polar) / a_ideal)) : 0;

        // ideal (real) number of regions per zone
        std::vector<double> r_regions(n_collars + 2, 1.);
        if (n_collars > 0) {
            const auto a_fitting = (M_PI - 2. * c_polar) / static_cast<double>(n_collars);
            for (size_t c = 0; c < n_collars; ++c) {
                const auto top   = c_polar + static_cast<double>(c) * a_fitting;
                const auto bot   = c_polar + static_cast<double>(c + 1) * a_fitting;
                r_regions[c + 1] = (area_of_cap(bot) - area_of_cap(top)) / area_of_ideal_region(N);
            }
        }

        // natural number of regions per zone
        n_regions.assign(n_collars + 2, 0);
        double discrepancy = 0.;
        for (size_t z = 0; z < n_collars + 2; ++z) {
            n_regions[z] = static_cast<size_t>(std::lround(r_regions[z] + discrepancy));
            discrepancy += r_regions[z] - static_cast<double>(n_regions[z]);
        }

        // colatitudes of caps enclosing the cumulative number of regions
        s_cap.assign(n_collars + 2, M_PI);
        s_cap[0] = c_polar;

        size_t subtotal = 1;
        for (size_t c = 0; c < n_collars; ++c) {
            subtotal += n_regions[c + 1];
            s_cap[c + 1] = sradius_of_cap(static_cast<double>(subtotal) * area_of_ideal_region(N));
        }
    }

    static std::vector<util::BoundingBox> regions(size_t N) {
        ASSERT(N > 0);

        std::vector<size_t> n_regions;
        std::vector<double> s_cap;
        eq_caps(N, n_regions, s_cap);

        // zones (polar caps and collars) from North to South, each divided in equal longitude intervals from Greenwich
        std::vector<util::BoundingBox> regions;
        regions.reserve(N);

        auto latitude = [&s_cap](size_t z) {  // 90 to -90 (exact)
            return z == 0 ? 90. : z == s_cap.size() ? -90. : 90. - util::radian_to_degree(s_cap[z - 1]);
        };

        for (size_t z = 0; z < n_regions.size(); ++z) {
            const auto north = latitude(z);
            const auto south = latitude(z + 1);
            const auto n     = static_cast<double>(n_regions[z]);

            for (size_t r = 0; r < n_regions[z]; ++r) {
                regions.emplace_back(north, 360. * static_cast<double>(r) / n, south,
                                     360. * static_cast<double>(r + 1) / n);
            }
        }

        ASSERT(regions.size() == N);
        return regions;
    }
};


std::vector<Tile> tiles_leopardi(const Crop& crop, size_t N) {
    if (N < 1) {
        throw exception::UserError("mir-tile: --tile=leopardi requires positive 'regions'");
    }

    std::vector<Tile> tiles;
    tiles.reserve(N);

    for (const auto& bbox : EqualRegions::regions(N)) {
        tiles.push_back({bbox, crop(bbox)});
    }

    return tiles;
}


// Bounding box of a pixel (nested order)
util::BoundingBox healpix_nested_pixel_bbox(size_t Nside, size_t pixel) {
    // from eckit::geo
    static constexpr int jrll[] = {2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4};
    static constexpr int jpll[] = {1, 3, 5, 7, 0, 2, 4, 6, 1, 3, 5, 7};

    const auto face = pixel / (Nside * Nside);
    auto p          = pixel % (Nside * Nside);

    // de-interleave bits (x: even bits, y: odd bits)
    size_t ix = 0;
    size_t iy = 0;
    for (size_t bit = 0; p != 0; ++bit, p >>= 2) {
        ix |= (p & 1) << bit;
        iy |= ((p >> 1) & 1) << bit;
    }

    double n = -90.;
    double s = 90.;
    std::vector<double> lons;

    for (size_t dx = 0; dx < 2; ++dx) {
        for (size_t dy = 0; dy < 2; ++dy) {
            const auto x = static_cast<double>(ix + dx) / static_cast<double>(Nside);
            const auto y = static_cast<double>(iy + dy) / static_cast<double>(Nside);

            // face-local coordinates to (z, phi)
            const auto jr = jrll[face] - x - y;
            const auto nr = jr < 1 ? jr : jr > 3 ? 4 - jr : 1.;
            const auto z  = jr < 1 ? 1. - nr * nr / 3. : jr > 3 ? nr * nr / 3. - 1. : (2. - jr) * 2. / 3.;

            const auto lat = util::radian_to_degree(std::asin(std::max(-1., std::min(1., z))));
            n              = std::max(n, lat);
            s              = std::min(s, lat);

            if (nr > 1e-15) {
                // longitude is undefined at the poles
                lons.push_back(45. * (jpll[face] * nr + x - y) / nr);
            }
        }
    }

    ASSERT(!lons.empty());

    // unwrap longitudes around the first vertex
    for (auto& lon : lons) {
        lon -= 360. * std::round((lon - lons.front()) / 360.);
    }

    const auto [w, e] = std::minmax_element(lons.begin(), lons.end());
    return {n, *w, s, *e};
}


std::vector<Tile> tiles_healpix_nested(const eckit::geo::Grid& grid, size_t nside) {
    const auto* hp = dynamic_cast<const eckit::geo::grid::reduced::HEALPix*>(&grid);
    if (hp == nullptr || hp->order() != eckit::geo::order::HEALPix::NESTED) {
        throw exception::UserError("mir-tile: --tile=healpix-nested requires a HEALPix grid in nested order");
    }

    const auto Nside = hp->Nside();
    const auto ratio = nside > 0 ? Nside / nside : 0;

    if (ratio < 2 || Nside % nside != 0 || (ratio & (ratio - 1)) != 0) {
        throw exception::UserError(
            "mir-tile: --tile=healpix-nested requires 'nside' such that grid Nside/nside"
            " is a power of 2 greater than 1 (grid Nside=" +
            std::to_string(Nside) + ")");
    }

    // in nested order, each tile (pixel at nside) is a contiguous group of ratio^2 points
    const auto group = ratio * ratio;
    const auto N     = 12 * nside * nside;
    ASSERT(grid.size() == N * group);

    std::vector<Tile> tiles(N);
    for (size_t t = 0; t < N; ++t) {
        tiles[t].bbox = healpix_nested_pixel_bbox(nside, t);
        tiles[t].indices.resize(group);
        std::iota(tiles[t].indices.begin(), tiles[t].indices.end(), t * group);
    }

    return tiles;
}


void MIRTile::execute(const eckit::option::CmdArgs& args) {
    std::string gridspec;
    if (!args.get("grid", gridspec)) {
        throw exception::UserError("mir-tile: option 'grid' is required");
    }

    std::string tiling;
    if (!args.get("tile", tiling)) {
        throw exception::UserError("mir-tile: option 'tile' is required");
    }

    if (args.has("area") && tiling != "regular") {
        throw exception::UserError("mir-tile: option 'area' only applies to --tile=regular");
    }

    bool expand = false;
    args.get("expand-indices", expand);

    std::unique_ptr<const eckit::geo::Grid> grid(eckit::geo::GridFactory::make_from_string(gridspec));
    ASSERT(grid);

    // grid representation (for area cropping)
    std::unique_ptr<input::MIRInput> in(new input::GridSpecInput(gridspec));
    ASSERT(in && in->next());

    repres::RepresentationHandle repres(in->field().representation());
    ASSERT(repres->numberOfPoints() == grid->size());

    Crop crop = [&repres](const util::BoundingBox& bbox) -> std::vector<size_t> {
        util::BoundingBox cropped(bbox);
        util::IndexMapping mapping;

        try {
            action::AreaCropper::crop(*repres, cropped, mapping);
        }
        catch (const eckit::AssertionFailed&) {
            return {};  // empty area crop
        }

        return mapping;
    };

    eckit::JSON j(Log::info());
    j.startObject();

    j << "grid" << grid->spec_str();
    j << "size" << grid->size();
    j << "tile" << tiling;

    std::vector<Tile> tiles;

    if (tiling == "regular") {
        util::BoundingBox area;
        key::Area::get(param::ConfigurationWrapper(args), area);

        size_t ni = 1;
        size_t nj = 1;
        args.get("ni", ni);
        args.get("nj", nj);

        j << "area"
          << std::vector<double>{area.north().value(), area.west().value(), area.south().value(), area.east().value()};
        j << "ni" << ni;
        j << "nj" << nj;

        tiles = tiles_regular(crop, area, ni, nj);
    }
    else if (tiling == "leopardi" || tiling == "equal-regions") {
        size_t regions = 0;
        args.get("regions", regions);
        j << "regions" << regions;

        tiles = tiles_leopardi(crop, regions);
    }
    else if (tiling == "healpix-nested") {
        size_t nside = 0;
        args.get("nside", nside);
        j << "nside" << nside;

        tiles = tiles_healpix_nested(*grid, nside);
    }
    else {
        throw exception::UserError("mir-tile: unknown tiling '" + tiling +
                                   "', choices are: regular, leopardi (equal-regions), healpix-nested");
    }

    // grid points (non-projected, as used for cropping)
    std::vector<PointLatLon> points(grid->size());
    for (const std::unique_ptr<repres::Iterator> it(repres->iterator()); it->next();) {
        points[it->index()] = it->pointUnrotated();
    }

    // each point belongs to one tile: on shared edges, the first tile (from North to South) not having it on its
    // western edge, consistently across the 0/360 meridian (exact longitude comparison, as in cropping)
    auto west_edge = [&points, &tiles](size_t t, size_t i) {
        const auto& b = tiles[t].bbox;
        return !b.isPeriodicWestEast() && points[i].lon().normalise(b.west()) == b.west();
    };

    const auto none = tiles.size();
    std::vector<size_t> owner(points.size(), none);

    for (size_t t = 0; t < tiles.size(); ++t) {
        for (auto i : tiles[t].indices) {
            if (owner[i] == none || (west_edge(owner[i], i) && !west_edge(t, i))) {
                owner[i] = t;
            }
        }
    }

    // tile indices, sorted (and unique)
    for (auto& tile : tiles) {
        tile.indices.clear();
    }

    for (size_t i = 0; i < owner.size(); ++i) {
        if (owner[i] != none) {
            tiles[owner[i]].indices.push_back(i);
        }
    }

    const auto indices =
        expand ? Indices([](eckit::JSON& j, const std::vector<size_t>& v) { j << v; })
               : Indices([](eckit::JSON& j, const std::vector<size_t>& v) {
                     // sorted indices as values and [first, last] ranges of consecutive values
                     j.startList();
                     for (auto first = v.begin(); first != v.end();) {
                         auto last = std::adjacent_find(first, v.end(), [](auto a, auto b) { return b != a + 1; });
                         if (last == v.end()) {
                             --last;
                         }

                         if (first == last) {
                             j << *first;
                         }
                         else {
                             j.startList();
                             j << *first;
                             j << *last;
                             j.endList();
                         }

                         first = std::next(last);
                     }
                     j.endList();
                 });

    j << "tiles";
    j.startList();
    for (size_t t = 0; t < tiles.size(); ++t) {
        const auto& tile = tiles[t];
        const auto& b    = tile.bbox;
        j.startObject();
        j << "index" << t;
        j << "bounding-box"
          << std::vector<double>{b.north().value(), b.west().value(), b.south().value(), b.east().value()};
        j << "size" << tile.indices.size();
        j << "indices";
        indices(j, tile.indices);
        j.endObject();
    }
    j.endList();

    if (bool coordinates = false; args.get("coordinates", coordinates) && coordinates) {
        std::vector<double> lats(points.size());
        std::vector<double> lons(points.size());

        std::transform(points.begin(), points.end(), lats.begin(), [](const auto& p) { return p.lat().value(); });
        std::transform(points.begin(), points.end(), lons.begin(), [](const auto& p) { return p.lon().value(); });

        j << "latitudes" << lats;
        j << "longitudes" << lons;
    }

    j.endObject();
    Log::info() << std::endl;
}


}  // namespace mir::tools


int main(int argc, char** argv) {
    mir::tools::MIRTile tool(argc, argv);
    return tool.start();
}
