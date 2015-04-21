/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Orientation.h"


Orientation::Orientation() {
}

Orientation::~Orientation() {
}

double Orientation::calculate(double k, double l, double m) const {
    /*
    if(atan2(m,l) == 0)
    	return 0;
     */
    //if (fabs(m)  <=  1.e-10) return 0.5*atan2((1.e-10*(m>=0 ? 1 : -1)),l);

    return 0.5 * atan2(m,l);
}

void Orientation::print(ostream& out) const {
    DerivedSubgridParameters::print(out);
    out << "Orientation" ;
}
