/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "LegendreTransformCoefficientsToSpectral.h"

#ifndef LegendrePolynomials_H
#include "LegendrePolynomials.h"
#endif

LegendreTransformCoefficientsToSpectral::LegendreTransformCoefficientsToSpectral(const ref_counted_ptr<const LegendrePolynomials>& poly, int truncation, const vector<double>& lats) :
    LegendreTransform(poly,truncation,lats) {
}


LegendreTransformCoefficientsToSpectral::~LegendreTransformCoefficientsToSpectral() {
}

int LegendreTransformCoefficientsToSpectral::transform(double* data, double* coeff, int latNumber) const {
    long lsize = (truncation_ + 1) * (truncation_+ 4) / 2;
    //int  ilim  = truncation_ + 1;

    double *nn, *ss;
    const double* poli;


    /* Calculate Fourier Coefficients */
    for ( int jlat = 0 ; jlat < latNumber ; jlat++ ) {
        double latitude = globalLatitudes_[jlat];
//		polynomials_->jspleg1(poli,latitude,work);
        poli = polynomials_->getOneLatitude(latitude,jlat);
        poli += lsize;
    }


    for ( int iter = 0 ; iter <= truncation_ ; iter++ ) {
        for ( int ifc = iter ; ifc <= truncation_ ; ifc++ ) {
            double data1 = 0;
            double data2 = 0;
            nn = coeff;
            ss = coeff + latNumber;
            for ( int jlat = 0 ; jlat < latNumber; jlat++ ) {
                data1 += poli[jlat] * nn[jlat];
                data2 += poli[jlat] * ss[jlat];
            }
            *data++ = data1;
            *data++ = data2;
            poli += latNumber;
        }
        coeff += 2 * latNumber;
    }

    delete [] poli;

//ssp
    return latNumber;
}

/*
void LegendreTransformCoefficientsToSpectral::print(ostream&) const
{
}
*/
