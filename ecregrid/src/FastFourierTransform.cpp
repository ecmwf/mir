/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "FastFourierTransform.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#include <algorithm>


FastFourierTransform::FastFourierTransform(int truncation, int lonNumber, int latsNumProc, int maxBloxSize) :
    truncation_(truncation), longitudeNumber_(lonNumber), latsProcess_(latsNumProc), maxBloxSize_(maxBloxSize) {
}

FastFourierTransform::~FastFourierTransform() {
}

int FastFourierTransform::adjustNumberOfPointsAlongLatitude(int *factor) const {
    /*
        For calculation purposes,the number of longitude points,
        has to be greater than 2*(output truncation) to ensure that the
        fourier transform is exact.
         Later, output longitude points have to be taken selectively to avoid the intermediate generated
         points, picking up values only at the required longitudes.
     */
    int numberOfPoints = longitudeNumber_;

    //  cout << "FastFourierTransform::adjustNumberOfPointsAlongLatitude  longitudeNumber_ " << longitudeNumber_ << " truncation_ " << truncation_ << endl;

    while ( numberOfPoints < 2 * truncation_) {
        *factor *= 2;
        numberOfPoints *= 2;
    }
    //  cout << "FastFourierTransform::adjustNumberOfPointsAlongLatitude  numberOfPoints " << numberOfPoints << " factor " << *factor << endl;

    return numberOfPoints;
}

void FastFourierTransform::setSinesAndCosinesFromZeroToPi(vector<double> &trigs, int points) const {
    /* Sinus and cosines for angles 0 to pi in N steps (sin,cos,sin,cos,...)
       Compute list of rotated twiddle factors */

    trigs.clear();
    double  del   = (2.0 * M_PI) / points ;
    int     nhl   = points / 2;

    for (int k = 0; k < nhl; k++) {
        double angle = k * del;
        trigs.push_back(cos(angle));
        trigs.push_back(sin(angle));
    }
}

/*
long FastFourierTransform::getCoefficientsOffset(double latitude) const
{
    // IOFF = NINT( (90.0 - ZLAT)/PBUILD )*(KTRUNC+1)*(KTRUNC+4)/2
    return  ( (90.0 - latitude));
}
*/
/*
void FastFourierTransform::factorize(vector<int>& f, int points) const
{
// Find allowed factors of N (8,6,5,4,3,2; only one 8 allowed)
//  Look for sixes first, store factors in order: 8,6,5,4,3,2

    f.clear();
    const int factors[7] = { 6,8,5,4,3,2,1 };

    int k = 0;
    bool eight = false;

    for(int j = 0; j < 7; j++) {

        while ( points % factors[j] == 0){
            if (factors[j] == 1)
                break;
            points /= factors[j];

            if(factors[j] == 8) {
            // Only one 8 allowed as a factor
                if (eight)
                    continue;
                eight = true;
            }
            f.push_back(factors[j]);
            k++;
            ASSERT(k <= 8);
        }
    }

    std::sort(f.begin(),f.end());

        // All allowed factors tried but some factors still left
//  if (points > 1)
//      throw UserError("FastFourierTransform::factorise -- Illegal factors found factorising");
}
*/

void FastFourierTransform::factorize(vector<int> &f, int n) const {
    while (n % 6 == 0)  {
        f.push_back(6);
        n /= 6;
    }
    if    (n % 8 == 0)  {
        f.push_back(8);
        n /= 8;
    }
    while (n % 5 == 0)  {
        f.push_back(5);
        n /= 5;
    }
    while (n % 4 == 0)  {
        f.push_back(4);
        n /= 4;
    }
    while (n % 3 == 0)  {
        f.push_back(3);
        n /= 3;
    }
    while    (n % 2 == 0)  {
        f.push_back(2);
        n /= 2;
    }
    //  if    (n % 2 == 0)  { f.push_back(2); n /= 2; }
    sort(f.begin(), f.end());
}

/*
void FastFourierTransform::print(ostream&) const
{
}
*/
