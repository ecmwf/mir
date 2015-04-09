#include "eckit/runtime/Tool.h"

#include "MIRJob.h"
#include "GribFileInput.h"
#include "GribFileOutput.h"

//------------------------------------------------------------------------------------------------------

class Outline : public eckit::Tool {

	virtual void run();

public:
	Outline(int argc,char **argv) :
		eckit::Tool(argc,argv)
    {
    }

};


void Outline::run() {

    MIRJob job;

    job.set("grid", "1/1");

    GribFileInput input("in.grib");
    GribFileOutput output("out.grib");

    job.execute(input, output);
}

//------------------------------------------------------------------------------------------------------

int main( int argc, char **argv )
{
    Outline tool(argc,argv);
    tool.start();
    return 0;
}

