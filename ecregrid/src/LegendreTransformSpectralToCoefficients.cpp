/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "LegendreTransformSpectralToCoefficients.h"


#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef LegendrePolynomials_H
#include "LegendrePolynomials.h"
#endif

#ifndef Timer_H
#include <eckit/utils/Timer.h>
#endif


LegendreTransformSpectralToCoefficients::LegendreTransformSpectralToCoefficients(const ref_counted_ptr<const LegendrePolynomials>& poly, int truncation, const vector<double>& lats, bool wind, double north, double south) :
    LegendreTransform(poly,truncation,lats),
    north_(north),
    south_(south),
    wind_(wind) {
}

LegendreTransformSpectralToCoefficients::~LegendreTransformSpectralToCoefficients() {
}

int LegendreTransformSpectralToCoefficients::transform(mapLats& mappedCoeff, const vector<comp>& dataComplex, int northIndex, int southIndex) const {

    eckit::Timer time("Legendre Transformation");

    long lsize = (truncation_ + 1) * (truncation_+ 4) / 2;
    int  ilim  = truncation_ + 1;

//		cout << "LegendreTransformSpectralToCoefficients::transform --- lsize --- " << lsize   << " 3*ilim  " << 3*ilim << endl;

    bool memory = false;
    const double* polinoms = polynomials_->getPolynoms();
    if(polinoms)
        memory = true;

    long size = ilim*2;
    vector<double> northLats(size);
    vector<double> southLats(size);
    vector<comp> suma(size);
    vector<comp> sums(size);

    /*
    	 For each latitude, the north and corresponding south latitude row
    	 are calculated at the same time from the same legendre functions.
    */

//	if(DEBUG)
//		cout << "LegendreTransformSpectralToCoefficients::transform --- northIndex --- " << northIndex  << " southIndex  " << southIndex << endl;

    if(memory)
        polinoms += lsize * northIndex;
    int latitudeCount = 0;
    for ( int jlat = northIndex ; jlat <= southIndex ; jlat++ ) {
        double latitude = globalLatitudes_[jlat];

        if(same(latitude,90.0) && wind_) {
            int n = uvAtPole(mappedCoeff,dataComplex);
            latitudeCount += n;
//			if(DEBUG)
//				cout << "LegendreTransformSpectralToCoefficients::transform Pole for Wind found"  << endl;
            continue;
        }

//		if(DEBUG)
//			cout << "LegendreTransformSpectralToCoefficients::transform --- lat --- " << globalLatitudes_[jlat]   << " index  " << jlat << endl;
        /* Calculate Fourier Coefficients */
//		polynomials_->getOneLatitude(polinoms,work,latitude,jlat);
        if(!memory)
            polinoms = polynomials_->getOneLatitude(latitude,jlat);

        int imn = 0, imp = 0;

        for ( int iter = 0 ; iter < ilim ; iter++ ) {
            int ital = truncation_ - iter + 1;
            int itals = (ital + 1) / 2;
            //int itala = ital / 2;
            int italss = 2 * itals;
            comp chold(0,0);
            comp chold1(0,0);
            for ( int k = 0 ; k < italss ; k = k + 2 ) {
                chold = chold + polinoms[k+imp] * dataComplex[k+imn];
                if(k+1 < ital)
                    chold1 = chold1 + polinoms[k+1+imp] * dataComplex[k+1+imn];
            }
            sums[iter] = chold;
            suma[iter] = chold1;

            imp       = imp + ital + 1;
            imn       = imn + ital;
        }
        /*
           For the southern hemisphere row, the legendre functions are
           the complex conjugates of the corresponding northern row -
           hence the juggling with the signs
        */

        double northLatitude = latitude;
        if(north_ < 0)
            northLatitude = -(northLatitude);

        double southLatitude = latitude;
        if(south_ < 0)
            southLatitude = -(southLatitude);

        bool isThereN = north_ > northLatitude || same(north_,northLatitude);
        bool isThereS = south_ < southLatitude || same(south_,southLatitude);

        if(north_ > 0 && (south_ > 0 || same(south_,0)))
            isThereS = false;
        if((north_ < 0 || same(north_,0)) && south_ < 0)
            isThereN = false;

        if(iszero(northLatitude) && iszero(southLatitude))
            isThereS = false;

        int count1 = 0, count2 = 0;
        for ( int i = 0 ; i <= truncation_ ; i++ ) {
            if(isThereN) {
                northLats[count1++] = sums[i].real() + suma[i].real();
                northLats[count1++] = sums[i].imag() + suma[i].imag();
            }
            if(isThereS) {
                southLats[count2++] = sums[i].real() - suma[i].real();
                southLats[count2++] = sums[i].imag() - suma[i].imag();
            }
        }

        if(isThereN) {
            mappedCoeff.insert( pair<double,vector<double> >(northLatitude,northLats) );
            ++latitudeCount;
        }
        if(isThereS) {
            mappedCoeff.insert( pair<double,vector<double> >(southLatitude,southLats) );
            ++latitudeCount;
        }
        if(memory)
            polinoms += lsize;
    }
    if(DEBUG)
        cout << "LegendreTransformSpectralToCoefficients::transform  Latitudes number  " << latitudeCount << endl;


    return latitudeCount;
}

int LegendreTransformSpectralToCoefficients::uvAtPole(mapLats& mappedCoeff,const vector<comp>& dataComplex) const {
    /**

    	 PXF(2)   - Single fourier coefficient calculated
    */
    int ilim = truncation_ + 1;

    int count = 0;
    if(north_ == 90.0) {
        vector<double> northLat;
        northLat.reserve(ilim*2);
        for ( int i = 0 ; i <= truncation_ ; i++ )
            northLat.push_back(0);
        comp fh =  firstHarmonic(dataComplex,true);
        northLat[2] =  fh.real();
        northLat[3] =  fh.imag();
        if(DEBUG) {
            cout << "uvAtPole northLat[0] " << northLat[0] << endl;
            cout << "uvAtPole northLat[1] " << northLat[1] << endl;
            cout << "uvAtPole northLat[2] " << northLat[2] << endl;
            cout << "uvAtPole northLat[3] " << northLat[3] << endl;
        }
        mappedCoeff.insert( pair<double,vector<double> >(90.0,northLat) );
        ++count;
    }
    if(south_== -90.0) {
        vector<double> southLat;
        southLat.reserve(ilim*2);
        for ( int i = 0 ; i <= truncation_ ; i++ )
            southLat.push_back(0);
        comp fh =  firstHarmonic(dataComplex,false);
        southLat[2] =  fh.real();
        southLat[3] =  fh.imag();
        if(DEBUG) {
            cout << "uvAtPole southLat[0] " << southLat[0]  << endl;
            cout << "uvAtPole southLat[1] " << southLat[1]  << endl;
            cout << "uvAtPole southLat[2] " << southLat[2]  << endl;
            cout << "uvAtPole southLat[3] " << southLat[3]  << endl;
        }
        mappedCoeff.insert( pair<double,vector<double> >(-90.0,southLat) );
        ++count;
    }

    return count;
}

comp LegendreTransformSpectralToCoefficients::firstHarmonic(const vector<comp>& dataComplex, bool pole) const {
    /**
         Calculates fourier coefficient for first harmonic only
         for U and V wind component at the pole.
    */
    comp jk(0,0);
    int itin1  = truncation_ + 1;
    int itout1 = truncation_;

    double  zpol = -1.0;
    if(pole)
        zpol = 1.0;

    double zp1 = -1.0;
    double zp2 = -3.0 * zpol;

//	int i1 = itin1 + 1;
    int i1 = itin1;

    double znorm = -sqrt(2.0);

    for ( int i = 0 ; i < itout1 ; i += 2 ) {
        int ii = i + 1;
        double z1 = sqrt( (2.0 * ii + 1.0)/(2.0* ii *(ii + 1.0)) );
        double z2 = sqrt( (2.0*( ii + 1.0) + 1.0)/(2.0*(ii +1.0)*(ii +2.0)) );
        if(i == itout1 - 1) {
            z2 = 0;
        }
        jk += (z1 * zp1 * dataComplex[i1] + z2 * zp2 * dataComplex[i1+1]) * znorm;
        //		cout << "LegendreTransformSpectralToCoefficients::firstHarmonic: ||||||| i: " << i << "  i1 " << i1 << jk << endl;

        zp1 = zp1 - 2.0 * (ii + 1.0) - 1.0;
        zp2 = zp2 - (2.0 * (ii + 2.0) + 1.0) * zpol;
        i1 += 2;
    }
    if(DEBUG)
        cout << "LegendreTransformSpectralToCoefficients::firstHarmonic: end " << jk << endl;
    return jk;
}

int LegendreTransformSpectralToCoefficients::transform(double* coeff, const double* data, int latNumber) const {
    long lsize = (truncation_ + 1) * (truncation_+ 4) / 2;
    //int  ilim  = truncation_ + 1;
    const double* poli;
    double *nn, *ss;
    /* Calculate Fourier Coefficients */
    for ( int jlat = 0 ; jlat < latNumber ; jlat++ ) {
        double latitude = globalLatitudes_[jlat];
        poli = polynomials_->getOneLatitude(latitude,jlat);
        poli += lsize;
    }

    for ( int iter = 0 ; iter <= truncation_ ; iter++ ) {
        for ( int ifc = iter ; ifc <= truncation_ ; ifc++ ) {
            double dataN = *data++;
            double dataS = *data++;
            nn = coeff;
            ss = coeff + latNumber;
            for ( int jlat = 0 ; jlat < latNumber; jlat++ ) {
                nn[jlat] += poli[jlat] * dataN;
                ss[jlat] += poli[jlat] * dataS;
            }
            poli += latNumber;
        }
        coeff += 2 * latNumber;
    }

    delete [] poli;

    return latNumber * 2;
}



void LegendreTransformSpectralToCoefficients::print(ostream&) const {
}
