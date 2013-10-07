/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Anisotropy.h"


Anisotropy::Anisotropy() 
{
}

Anisotropy::~Anisotropy()
{
}

double Anisotropy::calculate(double k, double l, double m) const
{
	double x = sqrt(l*l + m*m);
    if ( k + x == 0)
          return 0.01;

    if ( (k - x) < 1.e-8 )
          return 0;

    if( fabs(k + x) < 1.e-8 )
        return sqrt(1.e8 * (k - x));
    else
        return sqrt((k - x)/(k + x)); 	
}

void Anisotropy::print(ostream& out) const
{
	DerivedSubgridParameters::print(out);
	out << "Anisotropy" ;
}
