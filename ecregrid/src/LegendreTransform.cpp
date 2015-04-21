/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "LegendreTransform.h"
#include "LegendrePolynomials.h"

#ifndef Exception_H
#include "Exception.h"
#endif

LegendreTransform::LegendreTransform(const ref_counted_ptr<const LegendrePolynomials>& poly, int truncation, const vector<double>& lats):
    truncation_(truncation),
    polynomials_(poly),
    globalLatitudes_(lats) {
}

LegendreTransform::~LegendreTransform() {
}


