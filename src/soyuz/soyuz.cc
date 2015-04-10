#include "eckit/runtime/Tool.h"
#include "eckit/runtime/Context.h"

#include "soyuz/api/MIRJob.h"
#include "soyuz/inout/GribFileInput.h"
#include "soyuz/inout/NetcdfFileInput.h"
#include "soyuz/inout/DummyInput.h"
#include "soyuz/inout/DummyOutput.h"

#include "soyuz/inout/GribFileOutput.h"

//------------------------------------------------------------------------------------------------------

class MIRDemo : public eckit::Tool {

    virtual void run();

  public:
    MIRDemo(int argc, char **argv) :
        eckit::Tool(argc, argv) {
    }

};


void MIRDemo::run() {

    if(eckit::Context::instance().argc() > 2 ) {

        MIRJob job;
        GribFileInput input(eckit::Context::instance().argv(1));
        GribFileOutput output(eckit::Context::instance().argv(2));

        job.set("grid", "2/2");

        while (input.next()) {
            job.execute(input, output);
        }

        return;
    }

    if (1) {
        MIRJob job;

        job.set("area", "80/0/35/45");
        job.set("bitmap", "test.bitmap");
        job.set("accuracy", "8");
        job.set("frame", "3");

        GribFileInput input("test.grib");
        GribFileOutput output("result.grib");

        while (input.next()) {
            job.execute(input, output);
        }
    }

    if (1) {
        MIRJob job;

        job.set("accuracy", "24");
        job.set("frame", "10");

        NetcdfFileInput input("test.nc", "z");
        GribFileOutput output("netcdf.grib");

        job.execute(input, output);

    }

    if (1) {
        MIRJob job;

        job.set("accuracy", "24");
        job.set("frame", "10");

        DummyInput input;
        DummyOutput output;

        job.execute(input, output);

    }

    if (1) {
        MIRJob job;

        job.set("accuracy", "24");
        job.set("frame", "10");

        DummyInput input;
        GribFileOutput output("dummy.grib");

        job.execute(input, output);

    }

}

//------------------------------------------------------------------------------------------------------

int main( int argc, char **argv ) {
    MIRDemo tool(argc, argv);
    tool.start();
    return 0;
}

