





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
#include "eckit/runtime/Context.h"
#include "eckit/io/StdFile.h"

#include "mir/input/GribFileInput.h"
#include "mir/data/MIRField.h"


class MIRMakeLSM : public eckit::Tool {

    virtual void run();

    void usage(const std::string &tool);

  public:
    MIRMakeLSM(int argc, char **argv) :
        eckit::Tool(argc, argv) {
    }

};

void MIRMakeLSM::usage(const std::string &tool) {

    eckit::Log::info()
            << std::endl << "Usage: " << tool << " file.grib file.lsm" << std::endl
            ;

    ::exit(1);
}

void MIRMakeLSM::run() {

    eckit::Context &ctx = eckit::Context::instance();
    const std::string &tool = ctx.runName();

    size_t argc = ctx.argc();

    if (argc <= 2) {
        usage(tool);
    }

    mir::input::GribFileInput file(ctx.argv(1));
    eckit::StdFile out(ctx.argv(2), "w");

    while (file.next()) {
        mir::input::MIRInput &input = file;

        const mir::param::MIRParametrisation &parametrisation = input.parametrisation();

        size_t Ni = 0;
        size_t Nj = 0;

        ASSERT(parametrisation.get("Ni", Ni));
        ASSERT(parametrisation.get("Nj", Nj));

        eckit::Log::info() << "Ni=" << Ni << ", Nj=" << Nj << ", size=" << Ni*Nj << std::endl;
        ASSERT(Ni == Nj * 2);


        eckit::ScopedPtr<mir::data::MIRField> field(input.field());

        ASSERT(!field->hasMissing());

        const std::vector<double>  &values = field->values(0);

        unsigned char byte = 0;
        size_t n = 0;
        for (std::vector<double>::const_iterator j = values.begin(); j != values.end(); ++j) {
            bool land = (*j) >= 0.5;
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


int main( int argc, char **argv ) {
    MIRMakeLSM tool(argc, argv);
    tool.start();
    return 0;
}

