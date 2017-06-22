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

#include "eckit/utils/MD5.h"
#include "eckit/utils/RLE.h"

#include "eckit/memory/ScopedPtr.h"
#include "eckit/runtime/Tool.h"
#include "eckit/io/StdFile.h"

// #include "eckit/runtime/Context.h"
// #include "eckit/parser/Tokenizer.h"

// #include "mir/api/MIRJob.h"
// #include "mir/action/VOD2UVTransform.h"
// #include "mir/action/Sh2ShTransform.h"

// #include "mir/input/GribFileInput.h"
// #include "mir/output/GribFileOutput.h"
// #include "mir/data/MIRField.h"
// #include "mir/repres/Representation.h"


// #include "mir/api/MIRJob.h"
#include "mir/input/GribFileInput.h"
// #include "mir/output/GribFileOutput.h"
#include "mir/data/MIRField.h"


class LSM : public eckit::Tool {

    virtual void run();

    void usage(const std::string &tool);

  public:
    LSM(int argc, char **argv) :
        eckit::Tool(argc, argv) {
    }

};

// void LSM::grid(const atlas::grid::ReducedGrid &grid) {

//     const std::vector<int> &points_per_latitudes = grid.npts_per_lat();
//     size_t half = points_per_latitudes.size() / 2;

//     std::vector<int> diff; diff.reserve(half);
//     eckit::DIFFencode(points_per_latitudes.begin(), points_per_latitudes.begin() + half, std::back_inserter(diff));

//     std::vector<int> rle;
//     eckit::RLEencode2(diff.begin(), diff.end(), std::back_inserter(rle), 1000);

//     const atlas::Grid& g = grid;

//     eckit::Log::debug<LibMir>() << "uid " << g.uniqueID() << " hash " << g.hash() << " rle ";
//     eckit::RLEprint(eckit::Log::debug<LibMir>(), rle.begin(), rle.end());
//     eckit::Log::debug<LibMir>() << std::endl;
// }

void LSM::run() {

    mir::input::GribFileInput file("/tmp/lsm.grib");
    mir::input::MIRInput &input = file;

    while (file.next()) {

        input.parametrisation(); //
        mir::data::MIRField field(input.field());

        const std::vector<double> &v = field.values(0);
        std::vector<int32_t> p(v.size());

        eckit::StdFile f("zzzzz", "w");
        unsigned char c = 0;
        size_t n = 0;
        for (size_t i = 0; i < v.size(); i++) {

            p[i] = v[i] >= 0.5 ? 1 : 0;

            c <<=  1;
            c |= p[i];

            n++;

            if(n == 8) {
                fwrite(&c, 1, 1, f);
                c = 0;
                n = 0;
            }

        }

        while(n < 8) {
            c <<=  1;
            n++;
        }
        fwrite(&c, 1, 1, f);


        eckit::Log::info() << std::endl;

        // std::vector<int32_t> q;
        // q.push_back(p[0]);
        // for (size_t i = 0; i < v.size(); i++) {
        //     q.push_back(p[i] - p[i - 1]);
        // }

        // std::vector<int32_t> diff;
        // eckit::RLEencode2(q.begin(), q.end(), std::back_inserter(diff), 10000);


        // eckit::Log::info() << diff.size() << std::endl;



        // std::vector<int32_t> rle;
        // eckit::RLEdecode2(a.begin(), a.end(), std::back_inserter(rle));
        // eckit::Log::info() << a.size() << std::endl;

        // eckit::Log::info() << "};" << std::endl;
    }

}

int main( int argc, char **argv ) {
    LSM tool(argc, argv);
#if (ECKIT_MAJOR_VERSION == 0) && (ECKIT_MINOR_VERSION <= 10)
    tool.start();
    return 0;
#else
    return tool.start();
#endif
}

