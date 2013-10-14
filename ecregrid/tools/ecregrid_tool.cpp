/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef HandleRequest_H
#include "HandleRequest.h"
#endif

//#include <eckit/utils/Timer.h>

int main (int argc, char* argv[]) {

#ifdef ECREGRID_TIMER
    //eckit::Timer timer("time");
#endif

	try{
		HandleRequest request;

		int s = request.request(argc,argv);
		if(s)
			return 1;
		request.processing();
		cout << " - || - || - || - || - || -" << endl;
		return 0;	
	}
	catch(Exception& e){
		cout << e ;
		cout << "-> ecRegrid exception" << endl;
	}
	catch(exception& e){
		cout << "-> Standard exception " << e.what() << endl;
	}
	catch(...){
		cout << "-> Unknown exception" << endl;
	}
	return 1;
}

