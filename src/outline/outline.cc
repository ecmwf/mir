#include "eckit/runtime/Tool.h"

//------------------------------------------------------------------------------------------------------

class MirOutline : public eckit::Tool {

	void run()
	{
	}

public:
	MirOutline(int argc,char **argv) :
		eckit::Tool(argc,argv)
    {
    }

};

//------------------------------------------------------------------------------------------------------

int main( int argc, char **argv )
{
    MirOutline tool(argc,argv);
    tool.start();
    return 0;
}

