/*
* (C) Copyright 1996- ECMWF.
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


#include "eckit/io/StdFile.h"
#include "eckit/io/AutoCloser.h"
#include "eckit/log/Log.h"
#include "eckit/exception/Exceptions.h"

#include "mir/input/GribFileInput.h"
#include "mir/data/MIRField.h"
#include "mir/tools/MIRTool.h"

class MIRMakeLSM : public mir::tools::MIRTool {

    // -- Overridden methods

    void execute(const eckit::option::CmdArgs&);

    void usage(const std::string &tool) const;

    int minimumPositionalArguments() const {
        return 2;
    }

public:

    // -- Contructors

    MIRMakeLSM(int argc, char **argv) : mir::tools::MIRTool(argc, argv) {}

};


void MIRMakeLSM::usage(const std::string &tool) const {
    eckit::Log::info()
            << "\n" << "Usage: " << tool << " file.grib file.lsm"
            << std::endl;
}


void MIRMakeLSM::execute(const eckit::option::CmdArgs&) {

    mir::input::GribFileInput file(argv(1));

    eckit::AutoStdFile out(argv(2), "w");

    while (file.next()) {
        mir::input::MIRInput &input = file;

        const mir::param::MIRParametrisation &parametrisation = input.parametrisation();

        size_t Ni = 0;
        size_t Nj = 0;

        ASSERT(parametrisation.get("Ni", Ni));
        ASSERT(parametrisation.get("Nj", Nj));

        eckit::Log::info() << "Ni=" << Ni << ", Nj=" << Nj << ", size=" << Ni*Nj << std::endl;
        ASSERT(Ni == Nj * 2);


        mir::data::MIRField field(input.field());

        ASSERT(!field.hasMissing());

        unsigned char byte = 0;
        size_t n = 0;
        for (const auto& v : field.values(0)) {
            bool land = v >= 0.5;
            byte <<= 1;
            if (land) {
                byte |= 1;
            }
            n++;
            if (n == 8) {
                ASSERT(fwrite(&byte, 1, 1, out) == 1);
                n = 0;
            }
        }

        ASSERT(n == 0); // Reader will workout Ni Nj from file size

        // if (n) {
        //     byte <<= (8 - n);
        //     ASSERT(fwrite(&byte, 1, 1, out) == 1);

        // }

    }

}


int main(int argc, char **argv) {
    MIRMakeLSM tool(argc, argv);
    return tool.start();
}


