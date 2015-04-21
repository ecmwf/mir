/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "VerticalTransformerGrid.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Field_H
#include "Field.h"
#endif

#ifndef GridField_H
#include "GridField.h"
#endif

VerticalTransformerGrid::VerticalTransformerGrid(const string& typeOfLevel, int level) :
    VerticalTransformer(typeOfLevel,level) {
}

VerticalTransformerGrid::~VerticalTransformerGrid() {
}


Field* VerticalTransformerGrid::transform(const Field& inUp, const Field& inDown) const {
    /*
    const GridField& inputUp   = dynamic_cast<const GridField&>(inUp);

    static double* values = NULL;
    static long preserveOutSize = 0;
    long valuesSize = inputUp.dataLength();
    if(valuesSize > preserveOutSize){
        if(values)
            delete [] values;
        values = new double[valuesSize];
        ASSERT(values);
        preserveOutSize = valuesSize;
    }
    */
    return 0;
}
