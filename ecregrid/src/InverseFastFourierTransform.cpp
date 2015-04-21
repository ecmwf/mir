/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "InverseFastFourierTransform.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

#ifndef Timer_H
#include <eckit/utils/Timer.h>
#endif

#include <algorithm>


InverseFastFourierTransform::InverseFastFourierTransform(int truncation,int lonNumber,int latsNumProc,int maxBloxSize,double west, double east) :
    FastFourierTransform(truncation,lonNumber,latsNumProc,maxBloxSize), west_(west), east_(east) {
}


InverseFastFourierTransform::InverseFastFourierTransform(int truncation, int lonNumber, int latsNumProc, double west, double east, bool wind) :
    FastFourierTransform(truncation,lonNumber,latsNumProc), west_(west), east_(east), wind_(wind) {
}

InverseFastFourierTransform::~InverseFastFourierTransform() {
}


long InverseFastFourierTransform::transform(vector<double>& values, const mapLats& mapped, const Grid& grid) const {

//	eckit::Timer time("Inverse FFT");

    int multiFactor = 1;
    int needed = adjustNumberOfPointsAlongLatitude(&multiFactor);

    vector<double> trigs;
    setSinesAndCosinesFromZeroToPi(trigs,needed);
    vector<int>    factors;
    factorize(factors,needed);

    int jump = needed + 2;
//	int jump = needed;
    vector<int> offsets;
    vector<double> lats;

//	grid.getOffsets(offsets);

    long groupSize = latsProcess_ * jump;
    vector<double> group(groupSize);
    vector<double> work(groupSize);

    if(DEBUG) {
        cout << "InverseFastFourierTransform::transform latsProcess_ " << latsProcess_ << " jump " << jump << endl;
        cout << "InverseFastFourierTransform::transform groupSize " << groupSize << endl;
    }

    long j = 0;
    long currentPos = 0;
    mapLats::const_iterator iter = mapped.begin(), stop = mapped.end();
    for ( ; iter != stop; ++iter) {
        // ssp could be optimised more
//		offsets.push_back(grid.getLatitudeOffset(iter->first));
        offsets.push_back(grid.getLatitudeOffset(iter->first,currentPos));
        lats.push_back(iter->first);
        vector<double>::const_iterator it = iter->second.begin(), st = iter->second.end();
        int size = iter->second.size();
        for(; it != st; it++)
            group[j++] = *it;
        for(int i = 0 ; i < jump-size; i++)
            group[j++] = 0;
    }
    int nx = needed + 1;
    if (needed % 2)
        nx = needed;

    /*  Calculate number of blocks of maxBloxSize(64 is default) vectors
    	and number of vectors 'left over'. This remainder is transformed first. */

    int nblox = 1 + (latsProcess_ - 1) / maxBloxSize_;
    int nvex  = latsProcess_ - (nblox - 1) * maxBloxSize_;

    /* Loop through the blocks of vectors */
    long istart = 0;
    for (int nb = 0; nb < nblox; nb++) {
        int i = istart;
        if(DEBUG)
            cout << "InverseFastFourierTransform::transform istart: " << istart << " nvex " << nvex << endl;
        for (int j = 0; j < nvex; j++) {
            group[i+1] = 0.5 * group[i];
            i += jump;
        }
        if (needed%2 != 1) {
            i = istart + needed;
            for (int j = 0; j < nvex; j++) {
                group[i] = 0.5 * group[i];
                i += jump;
            }
        }
// do ovde dobro
        int ia = istart + 1;
        int la = 1;
        /* Work through the factors */
        int factorsSize = factors.size();
        for ( int k = 0; k < factorsSize; k++ ) {
            int factor = factors[k];
            if(DEBUG)
                cout << "	**** transform factor: " << factor << " ia: " << ia << " la: " << la << " nx: " << nx << " needed: " << needed << " jump: " << jump << " multi factor: " << multiFactor <<  endl;
            if ( k & 1 )
                passThroughData(&work[0], &work[la], &group[ia], &group[ia+factor*la], trigs, nx, jump, nvex, needed, factor, la);
            else
                passThroughData(&group[ia], &group[ia+la], &work[0], &work[factor*la], trigs, jump, nx, nvex, needed, factor, la);

            la *= factor;
// emos
            ia = istart + 1;
        }

        /* If necessary, copy results back to group */
        if (factorsSize % 2 != 0) {
            int ibase = 0;
            int jbase = ia;
            for ( int jj = 0; jj < nvex; jj++ ) {
                int i = ibase;
                int j = jbase;
                for ( int ii = 0; ii < needed; ii++ )
                    group[j++] = work[i++];
                ibase += nx;
                jbase += jump;
            }
            if(DEBUG)
                cout << "InverseFastFourierTransform::transform => Copy results back" << endl;
        }

        /* Fill in cyclic boundary values (ie repeat the data vector
        end points at opposite end of the vector)
        */
        int ix = istart;
        int iz = istart + needed;
        for (int i = 0; i < nvex; i++) {
            group[ix]     = group[iz];
            group[iz + 1] = group[ix + 1];
            ix += jump;
            iz += jump;
        }
        /* Adjust for next block */
        if(DEBUG)
            cout << "InverseFastFourierTransform::transform istart: " << istart << endl;
        istart += nvex * jump;
        nvex    = maxBloxSize_;
    }


    if(wind_)
        return getUsefulPartWind(values,offsets,group,jump,multiFactor,lats);

    return getUsefulPart(values,offsets,group,jump,multiFactor);

}

long InverseFastFourierTransform::getUsefulPart(vector<double>& values, const vector<int>& offsets, const vector<double>& group, int jump, int multiFactor) const {
    const size_t valuesLength = values.size();

    double inc  = 360.0 / longitudeNumber_;
    int realLongitudeNumber = int((east_ - west_) / inc + AREA_FACTOR) + 1;

    if(DEBUG) {
        int offsize = offsets.size();
        cout << "InverseFastFourierTransform::getUsefulPart multiFactor: " << multiFactor << " west: " << west_ << " east: " << east_ << endl;
        cout << "InverseFastFourierTransform::getUsefulPart offsize " << offsize << endl;
        cout << "InverseFastFourierTransform::getUsefulPart longitude number " << longitudeNumber_ << endl;
        cout << "InverseFastFourierTransform::getUsefulPart jump " << jump << endl;
        cout << "InverseFastFourierTransform::getUsefulPart increment " << inc << endl;
        cout << "InverseFastFourierTransform::getUsefulPart multi factor " << multiFactor << endl;
        cout << "InverseFastFourierTransform::getUsefulPart multi factor " << multiFactor << endl;
        cout << "InverseFastFourierTransform::getUsefulPart real longitude number " << realLongitudeNumber << endl;
    }

    int longit = int((360.0 + west_) / inc + 0.5);
    int n360   = longitudeNumber_;
    int start  = (longit % n360) * multiFactor;

    if(DEBUG) {
        cout << "InverseFastFourierTransform::getUsefulPart start: " << start <<  endl;
        cout << "InverseFastFourierTransform::getUsefulPart start longitude : " << longit <<  endl;
    }

    vector<int>::const_iterator it = offsets.begin(), st = offsets.end();
    unsigned long count = 0;
    unsigned long countValues = 0;

    long jumpCalculation = jump - 2;

    for(; it != st; it++) {
        int row = 0;
        long rowOffset = jump *(count++);
        for(int i = 0 ; i < realLongitudeNumber ; i++) {
            long next = 1 + ((start + i*multiFactor) % jumpCalculation);
            values[(*it)+row++] = group[rowOffset+next];
            countValues++;
        }
    }
    if(DEBUG)
        cout << "InverseFastFourierTransform::getUsefulPart valuesLength: " << valuesLength << " countValues: " << countValues << endl;
    ASSERT(valuesLength >= countValues);

    return countValues;
}

long InverseFastFourierTransform::getUsefulPartWind(vector<double>& values, const vector<int>& offsets, const vector<double>& group, int jump, int multiFactor, const vector<double>& lats) const {
    const size_t valuesLength = values.size();

    double inc  = 360.0 / longitudeNumber_;
    int realLongitudeNumber = int((east_ - west_) / inc + AREA_FACTOR) + 1;
    if(DEBUG) {
        int offsize = offsets.size();
        cout << "InverseFastFourierTransform::getUsefulPartWind multiFactor: " << multiFactor << " west: " << west_ << " east: " << east_ << endl;
        cout << "InverseFastFourierTransform::getUsefulPartWind offsize " << offsize << endl;
        cout << "InverseFastFourierTransform::getUsefulPartWind longitude number " << longitudeNumber_ << endl;
        cout << "InverseFastFourierTransform::getUsefulPartWind jump " << jump << endl;
        cout << "InverseFastFourierTransform::getUsefulPartWind increment " << inc << endl;
        cout << "InverseFastFourierTransform::getUsefulPartWind multi factor " << multiFactor << endl;
        cout << "InverseFastFourierTransform::getUsefulPartWind multi factor " << multiFactor << endl;
        cout << "InverseFastFourierTransform::getUsefulPartWind real longitude number " << realLongitudeNumber << endl;
    }

    int longit = int((360.0 + west_) / inc + 0.5);
    int n360   = longitudeNumber_;
    int start  = (longit % n360) * multiFactor;

    vector<int>::const_iterator it = offsets.begin(), st = offsets.end();
    unsigned long count = 0;
    unsigned long countValues = 0;

    long jumpCalculation = jump - 2;

    long lcnt = 0;
    double zdegr = M_PI / 180.0;

    for(; it != st; it++) {
        double lati = lats[lcnt++];
        double zcosi = 1.0 / cos(lati * zdegr);
        if( same(lati,90.0) || same(lati,-90.0))
            zcosi = 1.0;

        int row = 0;
        long rowOffset = jump *(count++);
        for(int i = 0 ; i < realLongitudeNumber ; i++) {
            long next = 1 + ((start + i*multiFactor) % jumpCalculation);
            values[(*it)+row++] = group[rowOffset+next] * zcosi;
            countValues++;
        }
    }

    if(DEBUG)
        cout << "InverseFastFourierTransform::getUsefulPartWind valuesLength: " << valuesLength << " countValues: " << countValues << endl;

    ASSERT(valuesLength >= countValues);

    return countValues;
}

void InverseFastFourierTransform::passThroughData(double* a, double* b, double* c, double* d, const vector<double>& trigs, int inInc, int outInc, int lot, int nLon, int factor, int la)  const {

    /*
       Performs one pass through data as part;
       of multiple real fft (fourier synthesis) routine;

       a is first real input vector
       b is equivalent to a + la * incos1
       c is first real output vector
       d is equivalent to c + factor * la * 1
       trigs  is a precalculated list of sines & cosines
       inInc is the increment between input vectors a;
       outInc is the increment between output vectors c;
       lot is the number of vectors;
       n is the length of the vectors;
       factor is the current factor of n;
       la is the product of previous factors;
     */


//  double a10, a11, a20, a21;
//  double b10, b11, b20, b21;

    int m = nLon / factor;
    int iink = la;
    int jink = la;
    int jump = (factor - 1) * jink;
    int kstop = (nLon - factor) / (2 * factor);
    int ibase = 0, i = 0, j = 0;
    int jbase = 0;

    switch (factor) {
    case 2: {
        double a01, b01;

        int ia = 0;
        int ja = 0;
        int ib = ia + 1 * (m + m - la);
        int jb = ja + jink;

        if (la != m) {
            for (int l = 0; l < la; ++l) {
                i = ibase;
                j = jbase;
                for (int ijk = 0; ijk < lot; ++ijk) {
                    c[ja + j] = a[ia + i] + a[ib + i];
                    c[jb + j] = a[ia + i] - a[ib + i];
                    i += inInc;
                    j += outInc;
                }
                ++ibase;
                ++jbase;
            }

            ia += iink;
            iink += iink;
            ib -= iink;
            ibase = 0;
            jbase += jump;
            jump += jump + jink;

            if (ia != ib) {
                for (int k = la; k <= kstop; k += la) {
                    int kb = k + k;
                    double cos1 = trigs[kb  ];
                    double sin1 = trigs[kb+1];
                    ibase = 0;
                    for (int l = 0; l < la; ++l) {
                        i = ibase;
                        j = jbase;
                        for (int ijk = 0; ijk < lot; ++ijk) {
                            a01 = a[ia + i] - a[ib + i];
                            b01 = b[ia + i] + b[ib + i];

                            c[ja + j] = a[ia + i] + a[ib + i];
                            d[ja + j] = b[ia + i] - b[ib + i];
                            c[jb + j] = cos1 * a01 - sin1 * b01;
                            d[jb + j] = sin1 * a01 + cos1 * b01;
                            i += inInc;
                            j += outInc;
                        }
                        ++ibase;
                        ++jbase;
                    }
                    ia += iink;
                    ib -= iink;
                    jbase += jump;
                }

                if (ia > ib)
                    return;
            }			/* End (ia != ib) */

            ibase = 0;
            for (int l = 0; l < la; ++l) {
                i = ibase;
                j = jbase;
                for (int ijk = 0; ijk < lot; ++ijk) {
                    c[ja + j] = a[ia + i];
                    c[jb + j] = -b[ia + i];
                    i += inInc;
                    j += outInc;
                }
                ++ibase;
                ++jbase;
            }
        } else {		/* (la != m) */
            for (int l = 0; l < la; ++l) {
                i = ibase;
                j = jbase;
                for (int ijk = 0; ijk < lot; ++ijk) {
                    c[ja + j] = 2.0 * (a[ia + i] + a[ib + i]);
                    c[jb + j] = 2.0 * (a[ia + i] - a[ib + i]);
                    i += inInc;
                    j += outInc;
                }
                ++ibase;
                ++jbase;
            }
        }
    }
    break;

    case 3: {
        double afa1, a1p2, a1m2, a0mm, a0mp;
        double bfa1, b1p2, b1m2, b0mm, b0mp;

        int ia = 0;
        int ja = 0;
        int ib = ia + 1 * (m + m - la);
        int ic = ib;
        int jb = ja + jink;
        int jc = jb + jink;

        double ssin60 = 2.0 * sin60;

        if (la != m) {
            for (int l = 0; l < la; ++l) {
                i = ibase;
                j = jbase;
                for (int ijk = 0; ijk < lot; ++ijk) {
                    afa1 = a[ia + i] - 0.5 * a[ib + i];
                    bfa1 = sin60 * b[ib + i];

                    c[ja + j] = a[ia + i] + a[ib + i];
                    c[jb + j] = afa1 - bfa1;
                    c[jc + j] = afa1 + bfa1;
                    i += inInc;
                    j += outInc;
                }
                ++ibase;
                ++jbase;
            }

            ia += iink;
            iink += iink;
            ib += iink;
            ic -= iink;
            jbase += jump;
            jump += jump + jink;

            if (ia != ic) {
                for (int k = la; k <= kstop; k += la) {
                    int kb = k + k;
                    int kc = kb + kb;
                    double cos1 = trigs[kb  ];
                    double sin1 = trigs[kb+1];
                    double cos2 = trigs[kc  ];
                    double sin2 = trigs[kc+1];
                    ibase = 0;
                    for (int l = 0; l < la; ++l) {
                        i = ibase;
                        j = jbase;
                        for (int ijk = 0; ijk < lot; ++ijk) {
                            a1p2 = a[ia + i] - 0.5 * (a[ib + i] + a[ic + i]);
                            b1m2 = b[ia + i] - 0.5 * (b[ib + i] - b[ic + i]);
                            a1m2 = sin60 * (a[ib + i] - a[ic + i]);
                            b1p2 = sin60 * (b[ib + i] + b[ic + i]);

                            a0mm = a1p2 - b1p2;
                            a0mp = a1p2 + b1p2;
                            b0mm = b1m2 - a1m2;
                            b0mp = b1m2 + a1m2;

                            c[ja + j] = a[ia + i] + a[ib + i] + a[ic + i];
                            d[ja + j] = b[ia + i] + b[ib + i] - b[ic + i];
                            c[jb + j] = cos1 * a0mm - sin1 * b0mp;
                            d[jb + j] = sin1 * a0mm + cos1 * b0mp;
                            c[jc + j] = cos2 * a0mp - sin2 * b0mm;
                            d[jc + j] = sin2 * a0mp + cos2 * b0mm;
                            i += inInc;
                            j += outInc;
                        }
                        ++ibase;
                        ++jbase;
                    }

                    ia += iink;
                    ib += iink;
                    ic -= iink;
                    jbase += jump;
                }

                if (ia > ic)
                    return ;
            }			/* End (ia != ic) */

            ibase = 0;
            for (int l = 0; l < la; ++l) {
                i = ibase;
                j = jbase;
                for (int ijk = 0; ijk < lot; ++ijk) {
                    a0mp = 0.5 * a[ia + i];
                    b0mp = sin60 * b[ia + i];

                    c[ja + j] = a[ia + i] + a[ib + i];
                    c[jb + j] = a0mp - a[ib + i] - b0mp;
                    c[jc + j] = a[ib + i] - a0mp - b0mp;
                    i += inInc;
                    j += outInc;
                }
                ++ibase;
                ++jbase;
            }
        } else {		/* (la != m) */
            for (int l = 0; l < la; ++l) {
                i = ibase;
                j = jbase;
                for (int ijk = 0; ijk < lot; ++ijk) {
                    a0mp = 2.0 * a[ia + i] - a[ib + i];
                    b0mp = ssin60 * b[ib + i];

                    c[ja + j] = 2.0 * (a[ia + i] + a[ib + i]);
                    c[jb + j] = a0mp - b0mp;
                    c[jc + j] = a0mp + b0mp;
                    i += inInc;
                    j += outInc;
                }
                ++ibase;
                ++jbase;
            }
        }
    }
    break;

    case 4: {
        double a01, a0p2, a1p3, a0m2, a1m3, a0p2ma1p3, a0m2pb1p3, a0m2mb1p3;
        double b01, b0p2, b1p3, b0m2, b1m3, b0p2pa1m3, b0p2ma1m3, b0m2mb1m3;

        double sin45 = sqrt(0.5);

        int ia = 0;
        int ja = 0;
        int ib = ia + 1 * (m + m - la);
        int id = ib;
        int ic = ib + 1 * (m + m);
        int jb = ja + jink;
        int jc = jb + jink;
        int jd = jc + jink;

        if (la != m) {
            for (int l = 0; l < la; ++l) {
                i = ibase;
                j = jbase;
                for (int ijk = 0; ijk < lot; ++ijk) {
                    a0p2 = a[ia + i] + a[ic + i];
                    a0m2 = a[ia + i] - a[ic + i];

                    c[ja + j] = a0p2 + a[ib + i];
                    c[jb + j] = a0m2 - b[ib + i];
                    c[jc + j] = a0p2 - a[ib + i];
                    c[jd + j] = a0m2 + b[ib + i];
                    i += inInc;
                    j += outInc;
                }
                ++ibase;
                ++jbase;
            }
            ia += iink;
            iink += iink;
            ib += iink;
            ic -= iink;
            id -= iink;
            jbase += jump;
            jump += jump + jink;

            if (ib != ic) {
                for (int k = la; k <= kstop; k += la) {
                    int kb = k + k;
                    int kc = kb + kb;
                    int kd = kc + kb;
                    double cos1 = trigs[kb  ];
                    double sin1 = trigs[kb+1];
                    double cos2 = trigs[kc  ];
                    double sin2 = trigs[kc+1];
                    double cos3 = trigs[kd  ];
                    double sin3 = trigs[kd+1];

                    ibase = 0;
                    for (int l = 0; l < la; ++l) {
                        i = ibase;
                        j = jbase;
                        for (int ijk = 0; ijk < lot; ++ijk) {
                            a0p2 = a[ia + i] + a[ic + i];
                            a0m2 = a[ia + i] - a[ic + i];
                            a1p3 = a[ib + i] + a[id + i];
                            a1m3 = a[ib + i] - a[id + i];
                            b0p2 = b[ia + i] + b[ic + i];
                            b0m2 = b[ia + i] - b[ic + i];
                            b1p3 = b[ib + i] + b[id + i];
                            b1m3 = b[ib + i] - b[id + i];

                            a0p2ma1p3 = a0p2 - a1p3;
                            a0m2pb1p3 = a0m2 + b1p3;
                            a0m2mb1p3 = a0m2 - b1p3;
                            b0p2pa1m3 = b0p2 + a1m3;
                            b0p2ma1m3 = b0p2 - a1m3;
                            b0m2mb1m3 = b0m2 - b1m3;

                            c[ja + j] = a0p2 + a1p3;
                            d[ja + j] = b0m2 + b1m3;
                            c[jc + j] = cos2 * a0p2ma1p3 - sin2 * b0m2mb1m3;
                            d[jc + j] = sin2 * a0p2ma1p3 + cos2 * b0m2mb1m3;
                            c[jb + j] = cos1 * a0m2mb1p3 - sin1 * b0p2pa1m3;
                            d[jb + j] = sin1 * a0m2mb1p3 + cos1 * b0p2pa1m3;
                            c[jd + j] = cos3 * a0m2pb1p3 - sin3 * b0p2ma1m3;
                            d[jd + j] = sin3 * a0m2pb1p3 + cos3 * b0p2ma1m3;
                            i += inInc;
                            j += outInc;
                        }
                        ++ibase;
                        ++jbase;
                    }
                    ia += iink;
                    ib += iink;
                    ic -= iink;
                    id -= iink;
                    jbase += jump;
                }

                if (ib > ic)
                    return ;
            }			/* End (ib != ic) */
            ibase = 0;
            for (int l = 0; l < la; ++l) {
                i = ibase;
                j = jbase;
                for (int ijk = 0; ijk < lot; ++ijk) {
                    a01 = a[ia + i] - a[ib + i];
                    b01 = b[ia + i] + b[ib + i];

                    c[ja + j] = a[ia + i] + a[ib + i];
                    c[jc + j] = b[ib + i] - b[ia + i];

                    c[jb + j] = sin45 * (a01 - b01);
                    c[jd + j] = -sin45 * (a01 + b01);
                    i += inInc;
                    j += outInc;
                }
                ++ibase;
                ++jbase;
            }
        } else {		/* (la != m) */
            for (int l = 0; l < la; ++l) {
                i = ibase;
                j = jbase;
                for (int ijk = 0; ijk < lot; ++ijk) {
                    a0p2 = a[ia + i] + a[ic + i];
                    a0m2 = a[ia + i] - a[ic + i];

                    c[ja + j] = 2.0 * (a0p2 + a[ib + i]);
                    c[jb + j] = 2.0 * (a0m2 - b[ib + i]);
                    c[jc + j] = 2.0 * (a0p2 - a[ib + i]);
                    c[jd + j] = 2.0 * (a0m2 + b[ib + i]);
                    i += inInc;
                    j += outInc;
                }
                ++ibase;
                ++jbase;
            }
        }
    }
    break;

    case 5: {
        double a1p2, a1m2, a0mm, a0mp, b136, b172, b236, b272;

        int ia = 0;
        int ja = 0;
        int ib = ia + 1 * (m + m - la);
        int ie = ib;
        int ic = ib + 1 * (m + m);
        int id = ic;
        int jb = ja + jink;
        int jc = jb + jink;
        int jd = jc + jink;
        int je = jd + jink;

        if (la != m) {
            for (int l = 0; l < la; ++l) {
                i = ibase;
                j = jbase;
                for (int ijk = 0; ijk < lot; ++ijk) {
                    a1p2 = 0.25 * (a[ib + i] + a[ic + i]);
                    a1m2 = qrt5 * (a[ib + i] - a[ic + i]);

                    a0mp = a[ia + i] - a1p2 + a1m2;
                    a0mm = a[ia + i] - a1p2 - a1m2;

                    b136 = b[ib + i] * sin36;
                    b172 = b[ib + i] * sin72;
                    b236 = b[ic + i] * sin36;
                    b272 = b[ic + i] * sin72;

                    c[ja + j] = a[ia + i] + a[ib + i] + a[ic + i];
                    c[jb + j] = a0mp - b172 - b236;
                    c[jc + j] = a0mm - b136 + b272;
                    c[jd + j] = a0mm + b136 - b272;
                    c[je + j] = a0mp + b172 + b236;
                    i += inInc;
                    j += outInc;
                }
                ++ibase;
                ++jbase;
            }
            ia += iink;
            iink += iink;
            ib += iink;
            ic += iink;
            id -= iink;
            ie -= iink;
            jbase += jump;
            jump += jump + jink;

            if (ib != id) {
                for (int k = la; k <= kstop; k += la) {
                    int kb = k + k;
                    int kc = kb + kb;
                    int kd = kc + kb;
                    int ke = kd + kb;
                    double cos1 = trigs[kb  ];
                    double sin1 = trigs[kb+1];
                    double cos2 = trigs[kc  ];
                    double sin2 = trigs[kc+1];
                    double cos3 = trigs[kd  ];
                    double sin3 = trigs[kd+1];
                    double cos4 = trigs[ke  ];
                    double sin4 = trigs[ke+1];
                    ibase = 0;
                    for (int l = 0; l < la; ++l) {
                        i = ibase;
                        j = jbase;
                        for (int ijk = 0; ijk < lot; ++ijk) {
                            double a10 = (a[ia + i] - 0.25 * ((a[ib + i] + a[ie + i]) + (a[ic + i] + a[id + i]))) + qrt5 * ((a[ib + i] + a[ie + i]) - (a[ic + i] + a[id + i]));
                            double a20 = (a[ia + i] - 0.25 * ((a[ib + i] + a[ie + i]) + (a[ic + i] + a[id + i]))) - qrt5 * ((a[ib + i] + a[ie + i]) - (a[ic + i] + a[id + i]));
                            double b10 = (b[ia + i] - 0.25 * ((b[ib + i] - b[ie + i]) + (b[ic + i] - b[id + i]))) + qrt5 * ((b[ib + i] - b[ie + i]) - (b[ic + i] - b[id + i]));
                            double b20 = (b[ia + i] - 0.25 * ((b[ib + i] - b[ie + i]) + (b[ic + i] - b[id + i]))) - qrt5 * ((b[ib + i] - b[ie + i]) - (b[ic + i] - b[id + i]));

                            double a11 = sin72 * (b[ib + i] + b[ie + i]) + sin36 * (b[ic + i] + b[id + i]);
                            double a21 = sin36 * (b[ib + i] + b[ie + i]) - sin72 * (b[ic + i] + b[id + i]);
                            double b11 = sin72 * (a[ib + i] - a[ie + i]) + sin36 * (a[ic + i] - a[id + i]);
                            double b21 = sin36 * (a[ib + i] - a[ie + i]) - sin72 * (a[ic + i] - a[id + i]);

                            c[ja + j] = a[ia + i] + ((a[ib + i] + a[ie + i]) + (a[ic + i] + a[id + i]));
                            d[ja + j] = b[ia + i] + ((b[ib + i] - b[ie + i]) + (b[ic + i] - b[id + i]));

                            c[jb + j] = cos1 * (a10 - a11) - sin1 * (b10 + b11);
                            d[jb + j] = sin1 * (a10 - a11) + cos1 * (b10 + b11);
                            c[je + j] = cos4 * (a10 + a11) - sin4 * (b10 - b11);
                            d[je + j] = sin4 * (a10 + a11) + cos4 * (b10 - b11);
                            c[jc + j] = cos2 * (a20 - a21) - sin2 * (b20 + b21);
                            d[jc + j] = sin2 * (a20 - a21) + cos2 * (b20 + b21);
                            c[jd + j] = cos3 * (a20 + a21) - sin3 * (b20 - b21);
                            d[jd + j] = sin3 * (a20 + a21) + cos3 * (b20 - b21);
                            i += inInc;
                            j += outInc;
                        }
                        ++ibase;
                        ++jbase;
                    }
                    ia += iink;
                    ib += iink;
                    ic += iink;
                    id -= iink;
                    ie -= iink;
                    jbase += jump;
                }
                if (ib > id)
                    return ;
            }			/* End (ib != id) */

            ibase = 0;
            for (int l = 0; l < la; ++l) {
                i = ibase;
                j = jbase;
                for (int ijk = 0; ijk < lot; ++ijk) {
                    c[ja + j] = a[ia + i] + a[ib + i] + a[ic + i];
                    c[jb + j] =  (qrt5 * (a[ia + i] - a[ib + i]) + (0.25 * (a[ia + i] + a[ib + i]) - a[ic + i])) - (sin36 * b[ia + i] + sin72 * b[ib + i]);
                    c[je + j] = -(qrt5 * (a[ia + i] - a[ib + i]) + (0.25 * (a[ia + i] + a[ib + i]) - a[ic + i])) - (sin36 * b[ia + i] + sin72 * b[ib + i]);
                    c[jc + j] =  (qrt5 * (a[ia + i] - a[ib + i]) - (0.25 * (a[ia + i] + a[ib + i]) - a[ic + i])) - (sin72 * b[ia + i] - sin36 * b[ib + i]);
                    c[jd + j] = -(qrt5 * (a[ia + i] - a[ib + i]) - (0.25 * (a[ia + i] + a[ib + i]) - a[ic + i])) - (sin72 * b[ia + i] - sin36 * b[ib + i]);
                    i += inInc;
                    j += outInc;
                }
                ++ibase;
                ++jbase;
            }
        } else {
            double qqrt5 = 2.0 * qrt5;
            double ssin36 = 2.0 * sin36;
            double ssin72 = 2.0 * sin72;
            for (int l = 0; l < la; ++l) {
                i = ibase;
                j = jbase;
                for (int ijk = 0; ijk < lot; ++ijk) {
                    c[ja + j] = 2.0 * (a[ia + i] + a[ib + i] + a[ic + i]);
                    c[jb + j] =(2.0 * (a[ia + i] - 0.25 * (a[ib + i] + a[ic + i])) + qqrt5 * (a[ib + i] - a[ic + i])) - (ssin72 * b[ib + i] + ssin36 * b[ic + i]);
                    c[jc + j] =(2.0 * (a[ia + i] - 0.25 * (a[ib + i] + a[ic + i])) - qqrt5 * (a[ib + i] - a[ic + i])) - (ssin36 * b[ib + i] - ssin72 * b[ic + i]);
                    c[jd + j] =(2.0 * (a[ia + i] - 0.25 * (a[ib + i] + a[ic + i])) - qqrt5 * (a[ib + i] - a[ic + i])) + (ssin36 * b[ib + i] - ssin72 * b[ic + i]);
                    c[je + j] =(2.0 * (a[ia + i] - 0.25 * (a[ib + i] + a[ic + i])) + qqrt5 * (a[ib + i] - a[ic + i])) + (ssin72 * b[ib + i] + ssin36 * b[ic + i]);
                    i += inInc;
                    j += outInc;
                }
                ++ibase;
                ++jbase;
            }
        }

    }
    break;

    case 6: {
        int ia = 0;
        int ib = ia + (2 * m - la);
        int ic = ib + 2 * m ;
        int id = ic + 2 * m ;
        int ie = ic;
        int iF = ib;
        int ja = 0;
        int jb = ja + jink;
        int jc = jb + jink;
        int jd = jc + jink;
        int je = jd + jink;
        int jf = je + jink;

        if (la != m) {	/* go to 690 */
            for (int l = 0; l < la; ++l) {
                i = ibase;
                j = jbase;
                for (int ijk = 0; ijk < lot; ++ijk) {
                    c[ja + j] = (a[ia + i] + a[id + i]) + (a[ib + i] + a[ic + i]);
                    c[jd + j] = (a[ia + i] - a[id + i]) - (a[ib + i] - a[ic + i]);
                    c[jb + j] =((a[ia + i] - a[id + i]) + 0.5 * (a[ib + i] - a[ic + i])) - sin60 * (b[ib + i] + b[ic + i]);
                    c[jf + j] =((a[ia + i] - a[id + i]) + 0.5 * (a[ib + i] - a[ic + i])) + sin60 * (b[ib + i] + b[ic + i]);
                    c[jc + j] =((a[ia + i] + a[id + i]) - 0.5 * (a[ib + i] + a[ic + i])) - sin60 * (b[ib + i] - b[ic + i]);
                    c[je + j] =((a[ia + i] + a[id + i]) - 0.5 * (a[ib + i] + a[ic + i])) + sin60 * (b[ib + i] - b[ic + i]);
                    i += inInc;
                    j += outInc;
                }
                ++ibase;
                ++jbase;
            }
            ia += iink;
            iink += iink;
            ib += iink;
            ic += iink;
            id -= iink;
            ie -= iink;
            iF -= iink;
            jbase += jump;
            jump += jump + jink;

            if (ic != id) {
                for (int k = la; k <= kstop; k += la) {
                    int kb = k + k;
                    int kc = kb + kb;
                    int kd = kc + kb;
                    int ke = kd + kb;
                    int kf = ke + kb;
                    double cos1 = trigs[kb  ];
                    double sin1 = trigs[kb+1];
                    double cos2 = trigs[kc  ];
                    double sin2 = trigs[kc+1];
                    double cos3 = trigs[kd  ];
                    double sin3 = trigs[kd+1];
                    double cos4 = trigs[ke  ];
                    double sin4 = trigs[ke+1];
                    double cos5 = trigs[kf  ];
                    double sin5 = trigs[kf+1];
                    ibase = 0;
                    for (int l = 0; l < la; ++l) {
                        i = ibase;
                        j = jbase;
                        for (int ijk = 0; ijk < lot; ++ijk) {
                            double a11 = a[ie + i] + a[ib + i] + a[ic + i] + a[iF + i];
                            double a20 = a[ia + i] + a[id + i] - 0.5 * a11;
                            double a21 = sin60 * ((a[ie + i] + a[ib + i]) - (a[ic + i] + a[iF + i]));
                            double b11 = b[ib + i] - b[ie + i] + b[ic + i] - b[iF + i];
                            double b20 = b[ia + i] - b[id + i] - 0.5 * b11;
                            double b21 = sin60 * ((b[ib + i] - b[ie + i]) - (b[ic + i] - b[iF + i]));

                            c[ja + j] = a[ia + i] + a[id + i] + a11;
                            d[ja + j] = b[ia + i] - b[id + i] + b11;
                            c[jc + j] = cos2 * (a20 - b21) - sin2 * (b20 + a21);
                            d[jc + j] = sin2 * (a20 - b21) + cos2 * (b20 + a21);
                            c[je + j] = cos4 * (a20 + b21) - sin4 * (b20 - a21);
                            d[je + j] = sin4 * (a20 + b21) + cos4 * (b20 - a21);

                            a11 = (a[ie + i] - a[ib + i]) + (a[ic + i] - a[iF + i]);
                            b11 = (b[ie + i] + b[ib + i]) - (b[ic + i] + b[iF + i]);
                            a20 = (a[ia + i] - a[id + i]) - 0.5 * a11;
                            a21 = sin60 * ((a[ie + i] - a[ib + i]) - (a[ic + i] - a[iF + i]));
                            b20 = (b[ia + i] + b[id + i]) + 0.5 * b11;
                            b21 = sin60 * ((b[ie + i] + b[ib + i]) + (b[ic + i] + b[iF + i]));

                            c[jd + j] = cos3 * (a[ia + i] - a[id + i] + a11) - sin3 * (b[ia + i] + b[id + i] - b11);
                            d[jd + j] = sin3 * (a[ia + i] - a[id + i] + a11) + cos3 * (b[ia + i] + b[id + i] - b11);
                            c[jb + j] = cos1 * (a20 - b21) - sin1 * (b20 - a21);
                            d[jb + j] = sin1 * (a20 - b21) + cos1 * (b20 - a21);



                            c[jf + j] = cos5 * (a20 + b21) - sin5 * (b20 + a21);
                            c[jf+j] = cos5 * (a20 + b21) - sin5 * (b20 + a21);


                            d[jf + j] = sin5 * (a20 + b21) + cos5 * (b20 + a21);
                            i += inInc;
                            j += outInc;
                        }
                        ++ibase;
                        ++jbase;
                    }
                    ia += iink;
                    ib += iink;
                    ic += iink;
                    id -= iink;
                    ie -= iink;
                    iF -= iink;
                    jbase += jump;
                }
                if (ic > id)
                    return ;
            }

            ibase = 0;
            for (int l = 0; l < la; ++l) {
                i = ibase;
                j = jbase;
                for (int ijk = 0; ijk < lot; ++ijk) {
                    c[ja + j] = a[ib + i] + (a[ia + i] + a[ic + i]);
                    c[jd + j] = b[ib + i] - (b[ia + i] + b[ic + i]);
                    c[jb + j] = (sin60 * (a[ia + i] - a[ic + i])) - (0.5 * (b[ia + i] + b[ic + i]) + b[ib + i]);
                    c[jf + j] =-(sin60 * (a[ia + i] - a[ic + i])) - (0.5 * (b[ia + i] + b[ic + i]) + b[ib + i]);
                    c[jc + j] =  sin60 * (b[ic + i] - b[ia + i]) + (0.5 * (a[ia + i] + a[ic + i]) - a[ib + i]);
                    c[je + j] =  sin60 * (b[ic + i] - b[ia + i]) - (0.5 * (a[ia + i] + a[ic + i]) - a[ib + i]);
                    i += inInc;
                    j += outInc;
                }
                ++ibase;
                ++jbase;
            }
        } else {
            double ssin60 = 2.0 * sin60;
            for (int l = 0; l < la; ++l) {
                i = ibase;
                j = jbase;
                for (int ijk = 0; ijk < lot; ++ijk) {
                    c[ja + j] = (2.0 * (a[ia + i] + a[id + i])) + (2.0 * (a[ib + i] + a[ic + i]));
                    c[jd + j] = (2.0 * (a[ia + i] - a[id + i])) - (2.0 * (a[ib + i] - a[ic + i]));
                    c[jb + j] = (2.0 * (a[ia + i] - a[id + i]) + (a[ib + i] - a[ic + i])) - (ssin60 * (b[ib + i] + b[ic + i]));
                    c[jf + j] = (2.0 * (a[ia + i] - a[id + i]) + (a[ib + i] - a[ic + i])) + (ssin60 * (b[ib + i] + b[ic + i]));
                    c[jc + j] = (2.0 * (a[ia + i] + a[id + i]) - (a[ib + i] + a[ic + i])) - (ssin60 * (b[ib + i] - b[ic + i]));
                    c[je + j] = (2.0 * (a[ia + i] + a[id + i]) - (a[ib + i] + a[ic + i])) + (ssin60 * (b[ib + i] - b[ic + i]));
                    i += inInc;
                    j += outInc;
                }
                ++ibase;
                ++jbase;
            }
        }
    }
    break;

    case 8: {
        double a0p7, a1p5, a2p6, p073, p074, p152;
        double a0m7, a1m5, a2m6, m073, m074, m152;

        double m_sqrt2 = 1.41421356237309504880;
// ssp
        if (la != m)
            throw FactorHandled(la,factor,m);

        int ia = 0;
        int ib = ia + iink;
        int ic = ib + iink;
        int id = ic + iink;
        int ie = id + iink;
        int iF = ie + iink;
        int ig = iF + iink;
        int ih = ig + iink;
        int ja = 0;
        int jb = ja + jink;
        int jc = jb + jink;
        int jd = jc + jink;
        int je = jd + jink;
        int jf = je + jink;
        int jg = jf + jink;
        int jh = jg + jink;

        for (int l = 0; l < la; ++l) {
            i = ibase;
            j = jbase;
            for (int ijk = 0; ijk < lot; ++ijk) {
                a0p7 = a[ia + i] + a[ih + i];
                a0m7 = a[ia + i] - a[ih + i];
                a1p5 = a[ib + i] + a[iF + i];
                a1m5 = a[ib + i] - a[iF + i];

                a2p6 = a[ic + i] + a[ig + i];
                a2m6 = a[ic + i] - a[ig + i];

                p073 = a0p7 + a[id + i];
                m073 = a0p7 - a[id + i];

                p074 = 2.0 * (a0m7 + a[ie + i]);
                m074 = 2.0 * (a0m7 - a[ie + i]);

                p152 = m_sqrt2 * (a1m5 + a2p6);
                m152 = m_sqrt2 * (a1m5 - a2p6);

                c[ja + j] = 2.0 * (p073 + a1p5);
                c[je + j] = 2.0 * (p073 - a1p5);
                c[jc + j] = 2.0 * (m073 - a2m6);
                c[jg + j] = 2.0 * (m073 + a2m6);

                c[jb + j] = m074 + m152;
                c[jf + j] = m074 - m152;
                c[jd + j] = p074 - p152;
                c[jh + j] = p074 + p152;
                i += inInc;
                j += outInc;
            }
            ++ibase;
            ++jbase;
        }
    }
//eeeeeeeeeeeeeeeeeeeeeeeeeeeeee
        /*
          int ia = 0;
          int ib = m;
          int ic = ib + 2 * m;
          int id = ic + 2 * m;
          int ie = id + 2 * m;

          int ja = 0;
          int jb = 1;
          int jc = jb + 1;
          int jd = jc + 1;
          int je = jd + 1;
          int jf = je + 1;
          int jg = jf + 1;
          int jh = jg + 1;

          double ssin45 = sqrt(2.0);


          for(int k = 0; k < la; k++) {
              i = ibase;
              j = jbase;
              for (int kk = 0; kk < lot; kk++) {
        //              double a01 = a[ia+i] + a[ie+i];
        //              double a02 = a[ia+i] - a[ie+i];
        //              double a03 = a[ib+i] + a[id+i];
        //              double a04 = a[ib+i] - a[id+i];
        //              double b01 = b[ib+i] + b[id+i];
        //              double b02 = b[ib+i] - b[id+i];

        //              double a11 = 2.0 * (a01 + a[ic+i]);
        //              double a12 = 2.0 * (a01 - a[ic+i]);
        //              double a21 = 2.0 * (a02 - b[ic+i]);
        //              double a22 = 2.0 * (a02 + b[ic+i]);
        //              double b11 = ssin45 * (a04 - b01);
        //              double b12 = ssin45 * (a04 + b01);
        //
        //              c[ja+j] = a11 + a03;
        //              c[je+j] = a11 - a03;
        //              c[jc+j] = a12 - b02;
        //              c[jg+j] = a12 + b02;
        //              c[jb+j] = a21 + b11;
        //              c[jf+j] = a21 - b11;
        //              c[jd+j] = a22 - b12;
        //              c[jh+j] = a22 + b12;
                  c[ja+j] = 2.0 * (((a[ia+i] + a[ie+i]) + a[ic+i]) + (a[ib+i] + a[id+i]));
                  c[je+j] = 2.0 * (((a[ia+i] + a[ie+i]) + a[ic+i]) - (a[ib+i] + a[id+i]));
                  c[jc+j] = 2.0 * (((a[ia+i] + a[ie+i]) - a[ic+i]) - (b[ib+i] - b[id+i]));
                  c[jg+j] = 2.0 * (((a[ia+i] + a[ie+i]) - a[ic+i]) + (b[ib+i] - b[id+i]));
                  c[jb+j] = 2.0 * ((a[ia+i] - a[ie+i]) - b[ic+i]) + ssin45 * ((a[ib+i] - a[id
        +i]) - (b[ib+i] + b[id+i]));
                  c[jf+j] = 2.0 * ((a[ia+i] - a[ie+i]) - b[ic+i]) - ssin45 * ((a[ib+i] - a[id
        +i]) - (b[ib+i] + b[id+i]));
                  c[jd+j] = 2.0 * ((a[ia+i] - a[ie+i]) + b[ic+i]) - ssin45 * ((a[ib+i] - a[id
        +i]) + (b[ib+i] + b[id+i]));
                  c[jh+j] = 2.0 * ((a[ia+i] - a[ie+i]) + b[ic+i]) + ssin45 * ((a[ib+i] - a[id
        +i]) + (b[ib+i] + b[id+i]));
                  i += inInc;
                  j += outInc;
              }
              ++ibase;
              ++jbase;

        }
        }
        */
    break;
    }
}

