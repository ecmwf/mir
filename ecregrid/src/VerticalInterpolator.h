/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef VerticalInterpolator_H
#define VerticalInterpolator_H

// namespace src;

// Headers
#ifndef   machine_H
#include "machine.h"
#endif

// Forward declarations
class GridField;

//

class VerticalInterpolator {
  public:

// -- Contructors
    VerticalInterpolator();

// -- Destructor
    virtual ~VerticalInterpolator();

// -- Methods

    virtual void  interpolate(const GridField& inputUp, const GridField& inputDown, double* values, unsigned long valuesSize) const = 0;

  protected:
// -- Methods
//	virtual void print(ostream&) const;

// Members

  private:

// -- Friends
//   friend ostream& operator<<(ostream& s,const VerticalInterpolator& p)
//	        { p.print(s); return s; }
};

#endif
