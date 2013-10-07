/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef DerivedSubgridParameters_H
#define DerivedSubgridParameters_H

// namespace src;

// Headers
#ifndef   machine_H
#include "machine.h"
#endif

// Forward declarations

class Interpolator;
class GridField;
class Grid;

// 

class DerivedSubgridParameters {
public:

// -- Contructors
	DerivedSubgridParameters();

// -- Destructor
	virtual ~DerivedSubgridParameters();

// -- Methods
	virtual double calculate(double k, double l, double m) const = 0;

protected:
    virtual void print(ostream&) const;

private:
// -- Friends
    friend ostream& operator<<(ostream& s,const DerivedSubgridParameters& p)
            { p.print(s); return s; }

};


#endif

