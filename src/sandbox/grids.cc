/*
 * (C) Copyright 1996-2016 ECMWF.
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
#include "atlas/grid/global/gaussian/OctahedralGaussian.h"
#include "atlas/grid/global/gaussian/ClassicGaussian.h"

#include "eckit/utils/MD5.h"
#include "eckit/utils/RLE.h"


class Grids : public eckit::Tool {

    virtual void run();

    void usage(const std::string &tool);
    void grid(const atlas::grid::global::Structured &);

  public:
    Grids(int argc, char **argv) :
        eckit::Tool(argc, argv) {
    }

};


void Grids::grid(const atlas::grid::global::Structured& grid) {

    const std::vector<long>& pl = grid.pl();
    ASSERT(pl.size());

    std::vector<int> points_per_latitudes(pl.size());
    ASSERT(pl.size()==points_per_latitudes.size());

    size_t half = points_per_latitudes.size() / 2;
    ASSERT(half>0);

    std::vector<int> diff;
    diff.reserve(half);
    eckit::DIFFencode(points_per_latitudes.begin(), points_per_latitudes.begin() + half, std::back_inserter(diff));

    std::vector<int> rle;
    eckit::RLEencode2(diff.begin(), diff.end(), std::back_inserter(rle), 1000);

    const atlas::grid::Grid& g = grid;

    eckit::Log::info() << "uid " << g.uniqueId() << " hash " << g.hash() << " rle ";
    eckit::RLEprint(eckit::Log::info(), rle.begin(), rle.end());
    eckit::Log::info() << eckit::newl;
}


void Grids::run() {
    using namespace atlas::grid::global::gaussian;

    grid(ClassicGaussian(16));
    grid(ClassicGaussian(24));
    grid(ClassicGaussian(32));
    grid(ClassicGaussian(48));
    grid(ClassicGaussian(64));
    grid(ClassicGaussian(80));
    grid(ClassicGaussian(96));
    grid(ClassicGaussian(128));
    grid(ClassicGaussian(160));
    grid(ClassicGaussian(200));
    grid(ClassicGaussian(256));
    grid(ClassicGaussian(320));
    grid(ClassicGaussian(400));
    grid(ClassicGaussian(512));
    grid(ClassicGaussian(576));
    grid(ClassicGaussian(640));
    grid(ClassicGaussian(800));
    grid(ClassicGaussian(1024));
    grid(ClassicGaussian(1280));
    grid(ClassicGaussian(1600));
    grid(ClassicGaussian(2000));
    grid(ClassicGaussian(4000));
    grid(ClassicGaussian(8000));

    grid(OctahedralGaussian(16));
    grid(OctahedralGaussian(24));
    grid(OctahedralGaussian(32));
    grid(OctahedralGaussian(48));
    grid(OctahedralGaussian(64));
    grid(OctahedralGaussian(80));
    grid(OctahedralGaussian(96));
    grid(OctahedralGaussian(128));
    grid(OctahedralGaussian(160));
    grid(OctahedralGaussian(200));
    grid(OctahedralGaussian(256));
    grid(OctahedralGaussian(320));
    grid(OctahedralGaussian(400));
    grid(OctahedralGaussian(512));
    grid(OctahedralGaussian(576));
    grid(OctahedralGaussian(640));
    grid(OctahedralGaussian(800));
    grid(OctahedralGaussian(1024));
    grid(OctahedralGaussian(1280));
    grid(OctahedralGaussian(1600));
    grid(OctahedralGaussian(2000));
    grid(OctahedralGaussian(4000));
    grid(OctahedralGaussian(8000));
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

