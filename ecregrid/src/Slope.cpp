/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Slope.h"


Slope::Slope() {
}

Slope::~Slope() {
}

double Slope::calculate(double k, double l, double m) const {
    return sqrt(k + sqrt(l*l + m*m));
}

void Slope::print(ostream& out) const {
    DerivedSubgridParameters::print(out);
    out << "Slope" ;
}
