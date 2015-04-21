/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef FieldDescriptionGribApi_H
#define FieldDescriptionGribApi_H

#ifndef FieldDescription_H
#include "FieldDescription.h"
#endif

#include "grib_api.h"

struct FieldDescriptionGribApi : public FieldDescription {

    // -- Contructors
    FieldDescriptionGribApi();
    FieldDescriptionGribApi(grib_handle *h);

    // -- Destructor
    ~FieldDescriptionGribApi(); // Change to virtual if base class

    FieldDescriptionGribApi &operator=(const FieldDescriptionGribApi &);

    void extractListOfPoints(grib_handle *inHandle, vector<Point> &points, const FieldDescription &output, vector<double> &outValues) const;

};

#endif
