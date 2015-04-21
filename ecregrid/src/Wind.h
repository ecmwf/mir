/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Wind_H
#define Wind_H

// Headers
#ifndef   machine_H
#include "machine.h"
#endif

#ifndef   Field_H
#include "Field.h"
#endif


class Wind {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    Wind();
    Wind(Field *u, Field *v);

    // -- Destructor

    ~Wind(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    bool   isSame() const {
        return !u_.get() && !v_.get();
    }
    Field &getU() const       {
        return *u_;
    }
    Field &getV()  const      {
        return *v_;
    }
    bool isVoDiv() const {
        return u_->number() == 138 && v_->number() == 155;
    }
    bool isRotated() const {
        return u_->isRotated() && v_->isRotated();
    }

    // -- Class members
    auto_ptr<Field> u_;
    auto_ptr<Field> v_;
};

#endif
