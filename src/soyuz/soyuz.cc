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

#include "soyuz/api/MIRJob.h"
#include "soyuz/input/GribFileInput.h"
#include "soyuz/input/NetcdfFileInput.h"
#include "soyuz/input/DummyInput.h"
#include "soyuz/output/DummyOutput.h"
#include "soyuz/output/GribFileOutput.h"


class MIRDemo : public eckit::Tool {

    virtual void run();

  public:
    MIRDemo(int argc, char **argv) :
        eckit::Tool(argc, argv) {
    }

};


void MIRDemo::run() {

    if(eckit::Context::instance().argc() > 2 ) {

        mir::api::MIRJob job;
        mir::input::GribFileInput input(eckit::Context::instance().argv(1));
        mir::output::GribFileOutput output(eckit::Context::instance().argv(2));

        job.set("grid", "2/2");

        while (input.next()) {
            job.execute(input, output);
        }

        return;
    }

    if (0) {
        mir::api::MIRJob job;

        job.set("area", "80/-20/35/45");
        job.set("bitmap", "test.bitmap");
        job.set("accuracy", "8");
        job.set("frame", "3");

        mir::input::GribFileInput input("test.grib");
        mir::output::GribFileOutput output("result.grib");

        while (input.next()) {
            job.execute(input, output);
        }
    }

    if (1) {
        mir::api::MIRJob job;

        // job.set("grid", "10/10");
        job.set("area", "80/-20/35/45");
        // job.set("frame", "3");
        // job.set("regular", "80");
        // job.set("reduced", "80");

        mir::input::GribFileInput input("/tmp/test.grib");
        mir::output::GribFileOutput output("ll.grib");

        while (input.next()) {
            job.execute(input, output);
        }
    }

    if (0) {
        mir::api::MIRJob job;

        job.set("accuracy", "24");
        job.set("frame", "10");

        mir::input::NetcdfFileInput input("test.nc", "z");
        mir::output::GribFileOutput output("netcdf.grib");

        job.execute(input, output);

    }

    if (0) {
        mir::api::MIRJob job;

        job.set("accuracy", "24");
        job.set("frame", "10");

        mir::input::DummyInput input;
        mir::output::DummyOutput output;

        job.execute(input, output);

    }

    if (0) {
        mir::api::MIRJob job;

        job.set("accuracy", "24");
        job.set("frame", "10");

        mir::input::DummyInput input;
        mir::output::GribFileOutput output("dummy.grib");

        job.execute(input, output);

    }

    if (0) {
        mir::api::MIRJob job;

        job.set("logic", "tool");

        mir::input::DummyInput input;
        mir::output::GribFileOutput output("dummy.grib");

        job.execute(input, output);

    }

}


int main( int argc, char **argv ) {
    MIRDemo tool(argc, argv);
    tool.start();
    return 0;
}

