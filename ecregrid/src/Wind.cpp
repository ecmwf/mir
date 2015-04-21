/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Wind.h"

Wind::Wind() :
    u_(0), v_(0) {
}

Wind::Wind(Field *u, Field *v) :
    u_(u), v_(v) {
}

Wind::~Wind() {
}


/*
void Wind::print(ostream&) const
{
}
*/
