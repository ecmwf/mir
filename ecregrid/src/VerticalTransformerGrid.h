#ifndef VerticalTransformerGrid_H
#define VerticalTransformerGrid_H
/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/


#include "VerticalTransformer.h"


// Forward declarations
class Field;

class VerticalTransformerGrid : public VerticalTransformer {
  public:

// -- Contructors

    VerticalTransformerGrid(const string& typeOfLevel, int level);

// -- Destructor

    ~VerticalTransformerGrid(); // Change to virtual if base class

// -- Overridden methods
    Field* transform(const Field& inputUp, const Field& inputDown) const;

  protected:

// -- Members
    // None

// -- Methods

// -- Class members
    // None

// -- Class methods
    // None

  private:

// No copy allowed

    VerticalTransformerGrid(const VerticalTransformerGrid&);
    VerticalTransformerGrid& operator=(const VerticalTransformerGrid&);

};

#endif
