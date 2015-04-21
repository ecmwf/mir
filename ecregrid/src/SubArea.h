/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef SubArea_H
#define SubArea_H

#ifndef Area_H
#include "Area.h"
#endif

// Forward declarations
class GridField;

/*! \class SubArea
    \brief Extract a sub Area values from input Grid Field

	Create a sub Area without Interpolation if the characteristics
	of the grid are the same as input grid
	Area boundaries for output field are adjusted (by shrinking)
	if necessary to fit the grid.
*/

class SubArea {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    SubArea(double north, double west, double south, double east);
    SubArea(const Area &other);

    // -- Destructor

    ~SubArea(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    void extract(const GridField &input, vector<double> &values) const;


  protected:

    // -- Members
    // None

    // -- Methods

    void print(ostream &) const; // Change to virtual if base class

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  private:

    // No copy allowed
    SubArea(const SubArea &);
    SubArea &operator=(const SubArea &);

    // -- Members
    Area area_; ///< sub Area boundaries


    // -- Friends

    friend ostream &operator<<(ostream &s, const SubArea &p) {
        p.print(s);
        return s;
    }

};

#endif
