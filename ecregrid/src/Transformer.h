/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Transformer_H
#define Transformer_H

// Headers
#ifndef   machine_H
#include "machine.h"
#endif

// Forward declarations
class Field;
class Wind;

//

class Transformer {
  public:

// -- Contructors

    Transformer();

// -- Destructor

    virtual ~Transformer(); // Change to virtual if base class

// -- Methods
    virtual Field* transform(const Field& inputField, const Field& outputField) const = 0;

    virtual Wind* transformVector(const Field& inU, const Field& inV, const Field& req) const = 0;

  private:
    Transformer(const Transformer&);
    Transformer& operator=(const Transformer&);
};

#endif
