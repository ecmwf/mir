/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Slope_H
#define Slope_H

// namespace src;

// Headers
#ifndef   DerivedSubgridParameters_H
#include "DerivedSubgridParameters.h"
#endif

// 

class Slope : public DerivedSubgridParameters {
public:

// -- Contructors
	Slope();

// -- Destructor
	virtual ~Slope();

// -- Methods


protected:
    virtual void print(ostream&) const;

private:

// -- Methods
	double calculate(double k, double l, double m)  const;

// -- Friends
    friend ostream& operator<<(ostream& s,const Slope& p)
            { p.print(s); return s; }


};


#endif

