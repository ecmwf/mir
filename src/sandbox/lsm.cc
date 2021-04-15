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


#include "eckit/io/StdFile.h"
#include "eckit/runtime/Tool.h"

#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/util/Log.h"
#include "mir/util/Types.h"


using namespace mir;


struct LSM : eckit::Tool {
    using Tool::Tool;
    void run() override;
};


void LSM::run() {

    input::GribFileInput file("/tmp/lsm.grib");
    const input::MIRInput& input = file;

    while (file.next()) {

        input.parametrisation();  //
        data::MIRField field(input.field());

        const MIRValuesVector& v = field.values(0);
        std::vector<int32_t> p(v.size());

        eckit::AutoStdFile f("zzzzz", "w");

        unsigned char c = 0;
        size_t n        = 0;
        for (size_t i = 0; i < v.size(); i++) {

            p[i] = v[i] >= 0.5 ? 1 : 0;

            c <<= 1;
            c |= p[i];

            n++;

            if (n == 8) {
                fwrite(&c, 1, 1, f);
                c = 0;
                n = 0;
            }
        }

        while (n < 8) {
            c <<= 1;
            n++;
        }
        fwrite(&c, 1, 1, f);


        Log::info() << std::endl;

        // std::vector<int32_t> q;
        // q.push_back(p[0]);
        // for (size_t i = 0; i < v.size(); i++) {
        //     q.push_back(p[i] - p[i - 1]);
        // }

        // std::vector<int32_t> diff;
        // eckit::RLEencode2(q.begin(), q.end(), std::back_inserter(diff), 10000);


        // Log::info() << diff.size() << std::endl;


        // std::vector<int32_t> rle;
        // eckit::RLEdecode2(a.begin(), a.end(), std::back_inserter(rle));
        // Log::info() << a.size() << std::endl;

        // Log::info() << "};" << std::endl;
    }
}


int main(int argc, char** argv) {
    LSM tool(argc, argv);
    return tool.start();
}
