/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef SubGrid_H
#define SubGrid_H

#ifndef   Extraction_H
#include "Extraction.h"
#endif

// Forward declarations
class GridField;

/*! \class SubGrid
    \brief Extract a subgrid values from input Grid Field

	Create a subgrid of an input regular latitude/longitude
	grid by selecting points (dissemination style)
	Area boundaries for output field are adjusted (by shrinking)
	if necessary to fit the grid.
*/

class SubGrid : public Extraction {
  public:

// -- Exceptions
    // None

// -- Contructors

    SubGrid(double ns, double we);

// -- Destructor

    ~SubGrid();


// -- Methods

// -- Overridden methods
    GridField* extract(GridField& g) const;

  protected:

// -- Methods
    void print(ostream&) const;

  private:

// No copy allowed
    SubGrid(const SubGrid&);
    SubGrid& operator=(const SubGrid&);

    double ns_; ///< Nort-South Increment of Subgrid
    double we_; ///< West-East  Increment of Subgrid

// -- Friends

    friend ostream& operator<<(ostream& s,const SubGrid& p) {
        p.print(s);
        return s;
    }

};

#endif
