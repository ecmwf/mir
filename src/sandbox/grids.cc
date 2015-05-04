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
// #include "eckit/runtime/Context.h"
// #include "eckit/parser/Tokenizer.h"

// #include "mir/api/MIRJob.h"
// #include "mir/action/VOD2UVTransform.h"
// #include "mir/action/Sh2ShTransform.h"

// #include "mir/input/GribFileInput.h"
// #include "mir/output/GribFileOutput.h"
// #include "mir/data/MIRField.h"
// #include "mir/repres/Representation.h"


#include "atlas/grids/grids.h"
#include "atlas/grids/rgg/OctahedralRGG.h"
#include "atlas/GridSpec.h"

#include "eckit/utils/MD5.h"
#include "eckit/utils/RLE.h"


class Grids : public eckit::Tool {

    virtual void run();

    void usage(const std::string &tool);
    void grid(const atlas::grids::ReducedGrid &);

  public:
    Grids(int argc, char **argv) :
        eckit::Tool(argc, argv) {
    }

};

void Grids::grid(const atlas::grids::ReducedGrid &grid) {

    const std::vector<int> &points_per_latitudes = grid.npts_per_lat();
    size_t half = points_per_latitudes.size() / 2;

    std::vector<int> diff; diff.reserve(half);
    eckit::DIFFencode(points_per_latitudes.begin(), points_per_latitudes.begin() + half, std::back_inserter(diff));

    std::vector<int> rle;
    eckit::RLEencode2(diff.begin(), diff.end(), std::back_inserter(rle), 1000);

    const atlas::Grid& g = grid;

    eckit::Log::info() << "uid " << g.unique_id() << " hash " << g.hash() << " rle ";
    eckit::RLEprint(eckit::Log::info(), rle.begin(), rle.end());
    eckit::Log::info() << std::endl;
}

void Grids::run() {

    grid(atlas::grids::rgg::N16());
    grid(atlas::grids::rgg::N24());
    grid(atlas::grids::rgg::N32());
    grid(atlas::grids::rgg::N48());
    grid(atlas::grids::rgg::N64());
    grid(atlas::grids::rgg::N80());
    grid(atlas::grids::rgg::N96());
    grid(atlas::grids::rgg::N128());
    grid(atlas::grids::rgg::N160());
    grid(atlas::grids::rgg::N200());
    grid(atlas::grids::rgg::N256());
    grid(atlas::grids::rgg::N320());
    grid(atlas::grids::rgg::N400());
    grid(atlas::grids::rgg::N512());
    grid(atlas::grids::rgg::N576());
    grid(atlas::grids::rgg::N640());
    grid(atlas::grids::rgg::N800());
    grid(atlas::grids::rgg::N1024());
    grid(atlas::grids::rgg::N1280());
    grid(atlas::grids::rgg::N1600());
    grid(atlas::grids::rgg::N2000());
    grid(atlas::grids::rgg::N4000());
    grid(atlas::grids::rgg::N8000());

    grid(atlas::grids::rgg::OctahedralRGG(16));
    grid(atlas::grids::rgg::OctahedralRGG(24));
    grid(atlas::grids::rgg::OctahedralRGG(32));
    grid(atlas::grids::rgg::OctahedralRGG(48));
    grid(atlas::grids::rgg::OctahedralRGG(64));
    grid(atlas::grids::rgg::OctahedralRGG(80));
    grid(atlas::grids::rgg::OctahedralRGG(96));
    grid(atlas::grids::rgg::OctahedralRGG(128));
    grid(atlas::grids::rgg::OctahedralRGG(160));
    grid(atlas::grids::rgg::OctahedralRGG(200));
    grid(atlas::grids::rgg::OctahedralRGG(256));
    grid(atlas::grids::rgg::OctahedralRGG(320));
    grid(atlas::grids::rgg::OctahedralRGG(400));
    grid(atlas::grids::rgg::OctahedralRGG(512));
    grid(atlas::grids::rgg::OctahedralRGG(576));
    grid(atlas::grids::rgg::OctahedralRGG(640));
    grid(atlas::grids::rgg::OctahedralRGG(800));
    grid(atlas::grids::rgg::OctahedralRGG(1024));
    grid(atlas::grids::rgg::OctahedralRGG(1280));
    grid(atlas::grids::rgg::OctahedralRGG(1600));
    grid(atlas::grids::rgg::OctahedralRGG(2000));
    grid(atlas::grids::rgg::OctahedralRGG(4000));
    grid(atlas::grids::rgg::OctahedralRGG(8000));
}


int main( int argc, char **argv ) {
    Grids tool(argc, argv);
    tool.start();
    return 0;
}

