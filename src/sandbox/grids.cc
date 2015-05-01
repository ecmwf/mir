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

#include "mir/api/emoslib.h"
#include "eckit/io/StdFile.h"
#include "mir/util/Grib.h"
#include "eckit/io/Buffer.h"

#include "atlas/grids/grids.h"
#include "atlas/grids/rgg/OctahedralRGG.h"
#include "eckit/util/MD5.h"
#include "eckit/util/RLE.h"


class VOD2UVTool : public eckit::Tool {

    virtual void run();

    void usage(const std::string &tool);

  public:
    VOD2UVTool(int argc, char **argv) :
        eckit::Tool(argc, argv) {
    }

};


void VOD2UVTool::run() {


}


int main( int argc, char **argv ) {
    VOD2UVTool tool(argc, argv);
    tool.start();
    return 0;
}

