/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "VerticalTransformerSpectral.h"

#ifndef Field_H
#include "Field.h"
#endif

#ifndef SpectralField_H
#include "SpectralField.h"
#endif

VerticalTransformerSpectral::VerticalTransformerSpectral(const string &typeOfLevel, int level) :
    VerticalTransformer(typeOfLevel, level) {
}

VerticalTransformerSpectral::~VerticalTransformerSpectral() {
}


Field *VerticalTransformerSpectral::transform(const Field &inUp, const Field &inDown) const {
    //const SpectralField& inputUp   = dynamic_cast<const SpectralField&>(inUp);
    //const SpectralField& inputDown = dynamic_cast<const SpectralField&>(inDown);

    return 0;
}
