#ifndef REGULARGRIDCONTEXT_H
#define REGULARGRIDCONTEXT_H
/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "GridContext.h"

struct RegularGridContext : public GridContext {
    int last_i_;
    int last_j_;
    RegularGridContext() : last_i_(0),last_j_(0) {}
};
#endif
