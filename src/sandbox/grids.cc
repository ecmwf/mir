/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include "eckit/runtime/Tool.h"


#include "atlas/grid/grids.h"
#include "atlas/grid/OctahedralRGG.h"

#include "eckit/utils/MD5.h"
#include "eckit/utils/RLE.h"


class Grids : public eckit::Tool {

    virtual void run();

    void usage(const std::string &tool);
    void grid(const atlas::grid::ReducedGrid &);

  public:
    Grids(int argc, char **argv) :
        eckit::Tool(argc, argv) {
    }

};

void Grids::grid(const atlas::grid::ReducedGrid &grid) {

    const std::vector<int> &points_per_latitudes = grid.npts_per_lat();
    size_t half = points_per_latitudes.size() / 2;

    std::vector<int> diff;
    diff.reserve(half);
    eckit::DIFFencode(points_per_latitudes.begin(), points_per_latitudes.begin() + half, std::back_inserter(diff));

    std::vector<int> rle;
    eckit::RLEencode2(diff.begin(), diff.end(), std::back_inserter(rle), 1000);

    const atlas::grid::Grid& g = grid;

    eckit::Log::info() << "uid " << g.uniqueId() << " hash " << g.hash() << " rle ";
    eckit::RLEprint(eckit::Log::info(), rle.begin(), rle.end());
    eckit::Log::info() << std::endl;
}

void Grids::run() {

    grid(atlas::grid::predefined::rgg::N16());
    grid(atlas::grid::predefined::rgg::N24());
    grid(atlas::grid::predefined::rgg::N32());
    grid(atlas::grid::predefined::rgg::N48());
    grid(atlas::grid::predefined::rgg::N64());
    grid(atlas::grid::predefined::rgg::N80());
    grid(atlas::grid::predefined::rgg::N96());
    grid(atlas::grid::predefined::rgg::N128());
    grid(atlas::grid::predefined::rgg::N160());
    grid(atlas::grid::predefined::rgg::N200());
    grid(atlas::grid::predefined::rgg::N256());
    grid(atlas::grid::predefined::rgg::N320());
    grid(atlas::grid::predefined::rgg::N400());
    grid(atlas::grid::predefined::rgg::N512());
    grid(atlas::grid::predefined::rgg::N576());
    grid(atlas::grid::predefined::rgg::N640());
    grid(atlas::grid::predefined::rgg::N800());
    grid(atlas::grid::predefined::rgg::N1024());
    grid(atlas::grid::predefined::rgg::N1280());
    grid(atlas::grid::predefined::rgg::N1600());
    grid(atlas::grid::predefined::rgg::N2000());
    grid(atlas::grid::predefined::rgg::N4000());
    grid(atlas::grid::predefined::rgg::N8000());

    grid(atlas::grid::OctahedralRGG(16));
    grid(atlas::grid::OctahedralRGG(24));
    grid(atlas::grid::OctahedralRGG(32));
    grid(atlas::grid::OctahedralRGG(48));
    grid(atlas::grid::OctahedralRGG(64));
    grid(atlas::grid::OctahedralRGG(80));
    grid(atlas::grid::OctahedralRGG(96));
    grid(atlas::grid::OctahedralRGG(128));
    grid(atlas::grid::OctahedralRGG(160));
    grid(atlas::grid::OctahedralRGG(200));
    grid(atlas::grid::OctahedralRGG(256));
    grid(atlas::grid::OctahedralRGG(320));
    grid(atlas::grid::OctahedralRGG(400));
    grid(atlas::grid::OctahedralRGG(512));
    grid(atlas::grid::OctahedralRGG(576));
    grid(atlas::grid::OctahedralRGG(640));
    grid(atlas::grid::OctahedralRGG(800));
    grid(atlas::grid::OctahedralRGG(1024));
    grid(atlas::grid::OctahedralRGG(1280));
    grid(atlas::grid::OctahedralRGG(1600));
    grid(atlas::grid::OctahedralRGG(2000));
    grid(atlas::grid::OctahedralRGG(4000));
    grid(atlas::grid::OctahedralRGG(8000));
}


int main( int argc, char **argv ) {
    Grids tool(argc, argv);
#if (ECKIT_MAJOR_VERSION == 0) && (ECKIT_MINOR_VERSION <= 10)
    tool.start();
    return 0;
#else
    return tool.start();
#endif
}

