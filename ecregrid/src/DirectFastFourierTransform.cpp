/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "DirectFastFourierTransform.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef  M_SQRT2
#define  M_SQRT2     1.41421356237309504880
#endif

#define  QUA  0.25
#define  QT5  0.559016994374947

#define  S36  0.587785252292473
#define  S60  0.866025403784437
#define  S72  0.951056516295154

#define  SQ2  0.707106781186547524401

#define  D60  (S60+S60)

DirectFastFourierTransform::DirectFastFourierTransform(int truncation, int lonNumber, int latsProcess, int index1, int index2) :
    FastFourierTransform(truncation,lonNumber,latsProcess),index1_(index1),index2_(index2) {
}

DirectFastFourierTransform::~DirectFastFourierTransform() {
}


void DirectFastFourierTransform::transform(const vector<int>& offsets, vector<double>& values, const vector<double>& coeff) const {
    int multiFactor = 1;
    int needed = adjustNumberOfPointsAlongLatitude(&multiFactor);

    int nLat = offsets.size();

    vector<double> trigs;
    setSinesAndCosinesFromZeroToPi(trigs,needed);
    vector<int>    factors;
    factorize(factors,needed);

    int jump = needed + 2;
    long groupSize = latsProcess_ * jump;
    double* group  = new double[groupSize];
    double* work   = new double[groupSize];

    long j1 = 0;
    long j2 = 0;

    for (int l = 0 ; l < nLat; ++l) {
        for (int i = 0 ; i < needed; ++i) {
            group[j1 + i] = coeff[j2 + i];
            for(int k = needed ; k < jump; k++)
                group[j1 + k] = 0;
        }
        j1 += jump;
        j2 += needed;
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
//		cout << "InverseFastFourierTransform::transform istart: " << istart << " nvex " << nvex << endl;
        int ia = istart + 1;
        int la = needed;
        /* Work through the factors */
        int factorsSize = factors.size();
        for ( int k = 0; k < factorsSize; k++ ) {
            int factor = factors[k];
            la /= factor;
            if ( k & 1 )
                passThroughData(work, work+la*factor, group+ia, group+ia+la, trigs, nx, jump, nvex, needed, factor, la);
            else
                passThroughData(group+ia, group+ia+la*factor, work, work+la, trigs, jump, nx, nvex, needed, factor, la);
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
        }
        /* Shift A(0) and fill in zero imaginary parts */
        int ix = istart;
        for (int i = 0; i < nvex; i++) {
            group[ix]      = group[ix + 1];
            group[ix + 1]  = 0.0;
            ix += jump;
        }

        if ( needed%2 != 1 ) {
            long iz = istart + (needed + 1);
            for ( int j = 0; j < nvex; j++ ) {
                group[iz] = 0.0;
                iz += jump;
            }
        }
        /* Adjust for next block */
        cout << " istart istart istart " << istart << endl;
        istart += nvex * jump;
        nvex    = maxBloxSize_;
    }

    int size = 2 * (truncation_ + 1);

    for( int lat = index1_; lat <= index2_; lat++) {
        int rix = jump * lat;
        int wix = lat + size * latsProcess_;
        values[wix] = group[rix];
        values[wix + latsProcess_] = 0.0;
        for(int jj = 0; jj < size; jj++)
            values[wix + jj * latsProcess_] = group[rix + jj];
    }
    delete [] group;
    delete [] work;
}


void DirectFastFourierTransform::passThroughData(double *a, double *b, double *c, double *d, const vector<double>& trigs, int inc3, int inc4, int lot, int n, int factor, int la) const {
    /*
       qpassc - performs one pass through data as part
       of multiple real fft (fourier analysis) routine.

       a      is first real input vector
       b      is equivalent to a + ifac * la * inc1
       c      is first real output vector;
       d      is equivalent to c + la * 1
       trigs  is a precalculated list of sines & cosines
       inc3   is the increment between input vectors a
       inc4   is the increment between output vectors c
       lot    is the number of vectors
       n      is the length of the vectors
       ifac   is the current factor of n
       la     is the product of previous factors
     */

    int i0, i1, i2, i3, i4, i5, i6, i7;
    int j0, j1, j2, j3, j4, j5, j6, j7;
    int ia, ib, ic;
    int ja, jb, jc;
    int i, j, k, l, m, ijk;
    int ibase, jbase;
    int iink, jink;
    int jump;
    int kstop;
    int kb, kc, kd, ke, kf;

    double a0, a1, a2, a3;
    double b0, b1, b2, b3;
    double c1, c2, c3, c4, c5;
    double s1, s2, s3, s4, s5;
    double w, x, y, z;

    m = n / factor;
    iink = la * 1;
    jink = la * 1;
    jump = (factor - 1) * iink;
    kstop = (n - factor) / (2 * factor);
    ibase = 0;
    jbase = 0;

    switch (factor) {
    case 2: {
        i0 = j0 = 0;
        i1 = i0 + iink;
        j1 = j0 + 1 * (m + m - la);
        if (la != m) {
            for (l = 0; l < la; ++l) {
                i = ibase;
                j = jbase;
#if defined (SX)
#pragma vdir nodep
#endif
                for (ijk = 0; ijk < lot; ++ijk) {
                    c[j0 + j] = a[i0 + i] + a[i1 + i];
                    c[j1 + j] = a[i0 + i] - a[i1 + i];
                    i += inc3;
                    j += inc4;
                }
                ibase += 1;
                jbase += 1;
            }
            j0    += jink;
            jink  += jink;
            j1    -= jink;
            ibase += jump;
            jump  += jump + iink;

            if (j0 != j1) {
                for (k = la; k <= kstop; k += la) {
                    kb = k + k;
                    c1 = trigs[kb  ];
                    s1 = trigs[kb+1];
                    jbase = 0;
                    for (l = 0; l < la; ++l) {
                        i = ibase;
                        j = jbase;
#if defined (SX)
#pragma vdir nodep
#endif
                        for (ijk = 0; ijk < lot; ++ijk) {
                            c[j0 + j] = a[i0 + i] + c1 * a[i1 + i] + s1 * b[i1 + i];
                            c[j1 + j] = a[i0 + i] - c1 * a[i1 + i] - s1 * b[i1 + i];
                            d[j0 + j] = c1 * b[i1 + i] - s1 * a[i1 + i] + b[i0 + i];
                            d[j1 + j] = c1 * b[i1 + i] - s1 * a[i1 + i] - b[i0 + i];
                            i += inc3;
                            j += inc4;
                        }
                        ibase += 1;
                        jbase += 1;
                    }
                    j0 += jink;
                    j1 -= jink;
                    ibase += jump;
                }		/* End FORK */
                if (j0 > j1)
//		  return 0;
                    break;
            }			/* End (i0 != i1) */
            jbase = 0;
            for (l = 0; l < la; ++l) {
                i = ibase;
                j = jbase;
#if defined (SX)
#pragma vdir nodep
#endif
                for (ijk = 0; ijk < lot; ++ijk) {
                    c[j0 + j] = a[i0 + i];
                    d[j1 + j] = -a[i1 + i];
                    i += inc3;
                    j += inc4;
                }
                ibase += 1;
                jbase += 1;
            }
        } else {		/* (la != m) */
            z = 1.0 / n;
            for (l = 0; l < la; ++l) {
                i = ibase;
                j = jbase;
#if defined (SX)
#pragma vdir nodep
#endif
                for (ijk = 0; ijk < lot; ++ijk) {
                    c[j0 + j] = z * (a[i0 + i] + a[i1 + i]);
                    c[j1 + j] = z * (a[i0 + i] - a[i1 + i]);
                    i += inc3;
                    j += inc4;
                }
                ibase += 1;
                jbase += 1;
            }
        }
        break;
    }

    case 3: {
        ia = 0;
        ib = ia + iink;
        ic = ib + iink;

        ja = 0;
        jb = ja + 1 * (m + m - la);
        jc = jb;

        if (la != m) {	/* else 390 */
            for (l = 0; l < la; ++l) {
                i = ibase;
                j = jbase;
#if defined (SX)
#pragma vdir nodep
#endif
                for (ijk = 0; ijk < lot; ++ijk) {
                    c[ja + j] = a[ia + i] + a[ib + i] + a[ic + i];
                    c[jb + j] = a[ia + i] - 0.5 * (a[ib + i] + a[ic + i]);
                    d[jb + j] = S60 * (a[ic + i] - a[ib + i]);
                    i += inc3;
                    j += inc4;
                }
                ibase += 1;
                jbase += 1;
            }
            ja += jink;
            jink += jink;
            jb += jink;
            jc -= jink;
            ibase += jump;
            jump += jump + iink;

            if (ja != jc) {	/* else  360 */
                for (k = la; k <= kstop; k += la) {
                    kb = k + k;
                    kc = kb + kb;
                    c1 = trigs[kb  ];
                    s1 = trigs[kb+1];
                    c2 = trigs[kc  ];
                    s2 = trigs[kc+1];
                    jbase = 0;
                    for (l = 0; l < la; ++l) {
                        i = ibase;
                        j = jbase;
#if defined (SX)
#pragma vdir nodep
#endif
                        for (ijk = 0; ijk < lot; ++ijk) {
                            a1 = c1 * a[ib + i] + s1 * b[ib + i] +
                                 c2 * a[ic + i] + s2 * b[ic + i];
                            b1 = c1 * b[ib + i] - s1 * a[ib + i] +
                                 c2 * b[ic + i] - s2 * a[ic + i];
                            a2 = a[ia + i] - 0.5 * a1;
                            b2 = b[ia + i] - 0.5 * b1;
                            a3 = S60 * (c1 * a[ib + i] + s1 * b[ib + i] -
                                        c2 * a[ic + i] - s2 * b[ic + i]);
                            b3 = S60 * (c1 * b[ib + i] - s1 * a[ib + i] -
                                        c2 * b[ic + i] + s2 * a[ic + i]);

                            c[ja + j] = a[ia + i] + a1;
                            d[ja + j] = b[ia + i] + b1;
                            c[jb + j] = a2 + b3;
                            d[jb + j] = b2 - a3;
                            c[jc + j] = a2 - b3;
                            d[jc + j] = -b2 - a3;
                            i += inc3;
                            j += inc4;
                        }
                        ibase += 1;
                        jbase += 1;
                    }
                    ja += jink;
                    jb += jink;
                    jc -= jink;
                    ibase += jump;
                }		/* End FORK */
                if (ja > jc)
                    break;
//		  return 0;
            }			/* End (ia != ic) */
            jbase = 0;
            for (l = 0; l < la; ++l) {
                i = ibase;
                j = jbase;
#if defined (SX)
#pragma vdir nodep
#endif
                for (ijk = 0; ijk < lot; ++ijk) {
                    /* soweit */
                    c[ja + j] = a[ia + i] + 0.5 * (a[ib + i] - a[ic + i]);
                    d[ja + j] = -S60 * (a[ib + i] + a[ic + i]);
                    c[jb + j] = a[ia + i] - a[ib + i] + a[ic + i];
                    i += inc3;
                    j += inc4;
                }
                ibase += 1;
                jbase += 1;
            }
        } else {		/* (la != m) */
            z = 1.0 / n;
            y = S60 / n;
            for (l = 0; l < la; ++l) {
                i = ibase;
                j = jbase;
#if defined (SX)
#pragma vdir nodep
#endif
                for (ijk = 0; ijk < lot; ++ijk) {
                    c[ja + j] = z * (a[ia + i] + a[ib + i] + a[ic + i]);
                    c[jb + j] = z * (a[ia + i] - 0.5 * (a[ib + i] + a[ic + i]));
                    d[jb + j] = y * (a[ic + i] - a[ib + i]);
                    i += inc3;
                    j += inc4;
                }
                ibase += 1;
                jbase += 1;
            }
        }
        break;
//	return 0;
    }

    case 4: {
        double a0p2, a1p3;

        i0 = 0;
        i1 = i0 + iink;
        i2 = i1 + iink;
        i3 = i2 + iink;
        j0 = 0;
        j1 = j0 + 1 * (m + m - la);
        j2 = j1 + 1 * (m + m);
        j3 = j1;

        if (la != m) {	/*else go to 490 */
            for (l = 0; l < la; ++l) {
                i = ibase;
                j = jbase;
#if defined (SX)
#pragma vdir nodep
#endif
                for (ijk = 0; ijk < lot; ++ijk) {
                    a0p2 = a[i0 + i] + a[i2 + i];
                    a1p3 = a[i1 + i] + a[i3 + i];

                    c[j0 + j] = a0p2 + a1p3;
                    c[j2 + j] = a0p2 - a1p3;

                    c[j1 + j] = a[i0 + i] - a[i2 + i];
                    d[j1 + j] = a[i3 + i] - a[i1 + i];
                    i += inc3;
                    j += inc4;
                }
                ibase += 1;
                jbase += 1;
            }
            j0 += jink;
            jink += jink;
            j1 += jink;
            j2 -= jink;
            j3 -= jink;
            ibase += jump;
            jump += jump + iink;

            if (j1 != j2) {	/* else go to 460; */
                for (k = la; k <= kstop; k += la) {
                    kb = k + k;
                    kc = kb + kb;
                    kd = kc + kb;
                    c1 = trigs[kb  ];
                    s1 = trigs[kb+1];
                    c2 = trigs[kc  ];
                    s2 = trigs[kc+1];
                    c3 = trigs[kd  ];
                    s3 = trigs[kd+1];
                    jbase = 0;
                    for (l = 0; l < la; ++l) {
                        i = ibase;
                        j = jbase;
#if defined (SX)
#pragma vdir nodep
#endif
                        for (ijk = 0; ijk < lot; ++ijk) {
                            a0 = a[i0 + i] + c2 * a[i2 + i] + s2 * b[i2 + i];
                            a2 = a[i0 + i] - c2 * a[i2 + i] - s2 * b[i2 + i];
                            b0 = b[i0 + i] + c2 * b[i2 + i] - s2 * a[i2 + i];
                            b2 = b[i0 + i] - c2 * b[i2 + i] + s2 * a[i2 + i];

                            a1 = c1 * a[i1 + i] + s1 * b[i1 + i] +
                                 c3 * a[i3 + i] + s3 * b[i3 + i];
                            a3 = c1 * a[i1 + i] + s1 * b[i1 + i] -
                                 c3 * a[i3 + i] - s3 * b[i3 + i];
                            b1 = c1 * b[i1 + i] - s1 * a[i1 + i] +
                                 c3 * b[i3 + i] - s3 * a[i3 + i];
                            b3 = c1 * b[i1 + i] - s1 * a[i1 + i] -
                                 c3 * b[i3 + i] + s3 * a[i3 + i];

                            c[j0 + j] = a0 + a1;
                            c[j2 + j] = a0 - a1;
                            d[j0 + j] = b0 + b1;
                            d[j2 + j] = b1 - b0;
                            c[j1 + j] = a2 + b3;
                            c[j3 + j] = a2 - b3;
                            d[j1 + j] = b2 - a3;
                            d[j3 + j] = -b2 - a3;
                            i += inc3;
                            j += inc4;
                        }
                        ibase += 1;
                        jbase += 1;
                    }
                    j0 += jink;
                    j1 += jink;
                    j2 -= jink;
                    j3 -= jink;
                    ibase += jump;
                }		/* End FORK */
                if (j1 > j2)
//		  return 0;
                    break;
            }			/* End (i1 != i2) */
            jbase = 0;
            for (l = 0; l < la; ++l) {
                i = ibase;
                j = jbase;
#if defined (SX)
#pragma vdir nodep
#endif
                for (ijk = 0; ijk < lot; ++ijk) {
                    c[j0 + j] =  a[i0 + i] + SQ2 * (a[i1 + i] - a[i3 + i]);
                    c[j1 + j] =  a[i0 + i] - SQ2 * (a[i1 + i] - a[i3 + i]);
                    d[j0 + j] = -a[i2 + i] - SQ2 * (a[i1 + i] + a[i3 + i]);
                    d[j1 + j] =  a[i2 + i] - SQ2 * (a[i1 + i] + a[i3 + i]);
                    i += inc3;
                    j += inc4;
                }
                ibase += 1;
                jbase += 1;
            }
        } else {		/* (la != m) */
            z = 1.0 / n;
            for (l = 0; l < la; ++l) {
                i = ibase;
                j = jbase;
#if defined (SX)
#pragma vdir nodep
#endif
                for (ijk = 0; ijk < lot; ++ijk) {
                    a0p2 = a[i0 + i] + a[i2 + i];
                    a1p3 = a[i1 + i] + a[i3 + i];

                    c[j0 + j] = z * (a0p2 + a1p3);
                    c[j2 + j] = z * (a0p2 - a1p3);
                    c[j1 + j] = z * (a[i0 + i] - a[i2 + i]);
                    d[j1 + j] = z * (a[i3 + i] - a[i1 + i]);
                    i += inc3;
                    j += inc4;
                }
                ibase += 1;
                jbase += 1;
            }
        }
//	return 0;
        break;
    }

    case 5: {
        double a1p4, a2p3, b1p4, b2p3, a025, b025, asps, bsps, a0pq, b0pq;
        double a1m4, a2m3, b1m4, b2m3, aqrt, bqrt, asms, bsms, a0mq, b0mq;

        i0 = 0;
        i1 = i0 + iink;
        i2 = i1 + iink;
        i3 = i2 + iink;
        i4 = i3 + iink;
        j0 = 0;
        j1 = j0 + 1 * (m + m - la);
        j2 = j1 + 1 * (m + m);
        j3 = j2;
        j4 = j1;

        if (la != m) {	/* else go to 590; */
            for (l = 0; l < la; ++l) {
                i = ibase;
                j = jbase;
#if defined (SX)
#pragma vdir nodep
#endif
                for (ijk = 0; ijk < lot; ++ijk) {
                    a1p4 = a[i1 + i] + a[i4 + i];
                    a1m4 = a[i1 + i] - a[i4 + i];
                    a2p3 = a[i2 + i] + a[i3 + i];
                    a2m3 = a[i2 + i] - a[i3 + i];

                    a025 = a[i0 + i] - 0.25 * (a1p4 + a2p3);
                    aqrt = QT5 * (a1p4 - a2p3);

                    c[j0 + j] = a[i0 + i] + a1p4 + a2p3;
                    c[j1 + j] = a025 + aqrt;
                    c[j2 + j] = a025 - aqrt;
                    d[j1 + j] = -S72 * a1m4 - S36 * a2m3;
                    d[j2 + j] = -S36 * a1m4 + S72 * a2m3;
                    i += inc3;
                    j += inc4;
                }
                ibase += 1;
                jbase += 1;
            }
            j0 += jink;
            jink += jink;
            j1 += jink;
            j2 += jink;
            j3 -= jink;
            j4 -= jink;
            ibase += jump;
            jump += jump + iink;

            if (j1 != j3) {
                for (k = la; k <= kstop; k += la) {
                    kb = k + k;
                    kc = kb + kb;
                    kd = kc + kb;
                    ke = kd + kb;
                    c1 = trigs[kb  ];
                    s1 = trigs[kb+1];
                    c2 = trigs[kc  ];
                    s2 = trigs[kc+1];
                    c3 = trigs[kd  ];
                    s3 = trigs[kd+1];
                    c4 = trigs[ke  ];
                    s4 = trigs[ke+1];
                    jbase = 0;
                    for (l = 0; l < la; ++l) {
                        i = ibase;
                        j = jbase;
#if defined (SX)
#pragma vdir nodep
#endif
                        for (ijk = 0; ijk < lot; ++ijk) {
                            a1p4 = c1 * a[i1 + i] + s1 * b[i1 + i] +
                                   c4 * a[i4 + i] + s4 * b[i4 + i];
                            a1m4 = c1 * a[i1 + i] + s1 * b[i1 + i] -
                                   c4 * a[i4 + i] - s4 * b[i4 + i];
                            a2p3 = c2 * a[i2 + i] + s2 * b[i2 + i] +
                                   c3 * a[i3 + i] + s3 * b[i3 + i];
                            a2m3 = c2 * a[i2 + i] + s2 * b[i2 + i] -
                                   c3 * a[i3 + i] - s3 * b[i3 + i];
                            b1p4 = c1 * b[i1 + i] - s1 * a[i1 + i] +
                                   c4 * b[i4 + i] - s4 * a[i4 + i];
                            b1m4 = c1 * b[i1 + i] - s1 * a[i1 + i] -
                                   c4 * b[i4 + i] + s4 * a[i4 + i];
                            b2p3 = c2 * b[i2 + i] - s2 * a[i2 + i] +
                                   c3 * b[i3 + i] - s3 * a[i3 + i];
                            b2m3 = c2 * b[i2 + i] - s2 * a[i2 + i] -
                                   c3 * b[i3 + i] + s3 * a[i3 + i];

                            a025 = a[i0 + i] - 0.25 * (a1p4 + a2p3);
                            aqrt = QT5 * (a1p4 - a2p3);
                            b025 = b[i0 + i] - 0.25 * (b1p4 + b2p3);
                            bqrt = QT5 * (b1p4 - b2p3);

                            a0pq = a025 + aqrt;
                            a0mq = a025 - aqrt;
                            b0pq = b025 + bqrt;
                            b0mq = b025 - bqrt;

                            asps = S72 * a1m4 + S36 * a2m3;
                            asms = S36 * a1m4 - S72 * a2m3;
                            bsps = S72 * b1m4 + S36 * b2m3;
                            bsms = S36 * b1m4 - S72 * b2m3;

                            c[j0 + j] = a[i0 + i] + a1p4 + a2p3;
                            c[j1 + j] = a0pq + bsps;
                            c[j2 + j] = a0mq + bsms;
                            c[j3 + j] = a0mq - bsms;
                            c[j4 + j] = a0pq - bsps;
                            d[j0 + j] = b[i0 + i] + b1p4 + b2p3;
                            d[j1 + j] = b0pq - asps;
                            d[j2 + j] = b0mq - asms;
                            d[j3 + j] = -b0mq - asms;
                            d[j4 + j] = -b0pq - asps;
                            i += inc3;
                            j += inc4;
                        }
                        ibase += 1;
                        jbase += 1;
                    }
                    j0 += jink;
                    j1 += jink;
                    j2 += jink;
                    j3 -= jink;
                    j4 -= jink;
                    ibase += jump;
                }		/* End FORK */
                if (j1 > j3)
//		  return 0;
                    break;
            }			/* End (jb != jd) */
            jbase = 0;
            for (l = 0; l < la; ++l) {
                i = ibase;
                j = jbase;
#if defined (SX)
#pragma vdir nodep
#endif
                for (ijk = 0; ijk < lot; ++ijk) {
                    a1p4 = a[i1 + i] + a[i4 + i];
                    a1m4 = a[i1 + i] - a[i4 + i];
                    a2p3 = a[i2 + i] + a[i3 + i];
                    a2m3 = a[i2 + i] - a[i3 + i];

                    a025 = a[i0 + i] + 0.25 * (a1m4 - a2m3);
                    aqrt = QT5 * (a1m4 + a2m3);

                    c[j0 + j] = a025 + aqrt;
                    c[j1 + j] = a025 - aqrt;
                    c[j2 + j] = a[i0 + i] - a1m4 + a2m3;
                    d[j0 + j] = -S36 * a1p4 - S72 * a2p3;
                    d[j1 + j] = -S72 * a1p4 + S36 * a2p3;

                    i += inc3;
                    j += inc4;
                }
                ibase += 1;
                jbase += 1;
            }
        } else {
            z = 1.0 / n;
            y = QT5 / n;
            x = S36 / n;
            w = S72 / n;

            for (l = 0; l < la; ++l) {
                i = ibase;
                j = jbase;
#if defined (SX)
#pragma vdir nodep
#endif
                for (ijk = 0; ijk < lot; ++ijk) {
                    a1p4 = a[i1 + i] + a[i4 + i];
                    a1m4 = a[i1 + i] - a[i4 + i];
                    a2p3 = a[i2 + i] + a[i3 + i];
                    a2m3 = a[i2 + i] - a[i3 + i];

                    a025 = z * (a[i0 + i] - 0.25 * (a1p4 + a2p3));
                    aqrt = y * (a1p4 - a2p3);

                    c[j0 + j] = z * (a[i0 + i] + a1p4 + a2p3);
                    c[j1 + j] = a025 + aqrt;
                    c[j2 + j] = a025 - aqrt;
                    d[j1 + j] = -w * a1m4 - x * a2m3;
                    d[j2 + j] = w * a2m3 - x * a1m4;
                    i += inc3;
                    j += inc4;
                }
                ibase += 1;
                jbase += 1;
            }
        }
//	return 0;
        break;
    }

    case 6: {
        double ab1a, ab2a, ab3a, ab4a, ab5a;
        double ab1b, ab2b, ab3b, ab4b, ab5b;
        double a0p3, a1p4, a1p5, a2p4, a2p5;
        double a0m3, a1m4, a1m5, a2m4, a2m5;
        double b1p4, b2p5;
        double b1m4, b2m5;
        double ap05, bp05, ap60, bp60;
        double am05, bm05, am60, bm60;

        i0 = 0;
        i1 = i0 + iink;
        i2 = i1 + iink;
        i3 = i2 + iink;
        i4 = i3 + iink;
        i5 = i4 + iink;
        j0 = 0;
        j1 = j0 + 1 * (m + m - la);
        j2 = j1 + 1 * (m + m);
        j3 = j2 + 1 * (m + m);
        j4 = j2;
        j5 = j1;

        if (la != m) {
            for (l = 0; l < la; ++l) {
                i = ibase;
                j = jbase;
#if defined (SX)
#pragma vdir nodep
#endif
                for (ijk = 0; ijk < lot; ++ijk) {
                    a0p3 = a[i0 + i] + a[i3 + i];
                    a0m3 = a[i0 + i] - a[i3 + i];
                    a1p4 = a[i1 + i] + a[i4 + i];
                    a1m4 = a[i1 + i] - a[i4 + i];
                    a2p5 = a[i2 + i] + a[i5 + i];
                    a2m5 = a[i2 + i] - a[i5 + i];

                    c[j0 + j] = a0p3 + a1p4 + a2p5;
                    c[j3 + j] = a0m3 + a2m5 - a1m4;

                    c[j1 + j] = a0m3 - 0.5 * (a2m5 - a1m4);
                    c[j2 + j] = a0p3 - 0.5 * (a1p4 + a2p5);

                    d[j1 + j] = S60 * (-a2m5 - a1m4);
                    d[j2 + j] = S60 * (a2p5 - a1p4);
                    i += inc3;
                    j += inc4;
                }
                ibase += 1;
                jbase += 1;
            }
            j0 += jink;
            jink += jink;
            j1 += jink;
            j2 += jink;
            j3 -= jink;
            j4 -= jink;
            j5 -= jink;
            ibase += jump;
            jump += jump + iink;

            if (j2 != j3) {
                for (k = la; k <= kstop; k += la) {
                    kb = k + k;
                    kc = kb + kb;
                    kd = kc + kb;
                    ke = kd + kb;
                    kf = ke + kb;
                    c1 = trigs[kb  ];
                    s1 = trigs[kb+1];
                    c2 = trigs[kc  ];
                    s2 = trigs[kc+1];
                    c3 = trigs[kd  ];
                    s3 = trigs[kd+1];
                    c4 = trigs[ke  ];
                    s4 = trigs[ke+1];
                    c5 = trigs[kf  ];
                    s5 = trigs[kf+1];
                    jbase = 0;
                    for (l = 0; l < la; ++l) {
                        i = ibase;
                        j = jbase;
#if defined (SX)
#pragma vdir nodep
#endif
                        for (ijk = 0; ijk < lot; ++ijk) {
                            ab1a = c1 * a[i1 + i] + s1 * b[i1 + i];
                            ab1b = c1 * b[i1 + i] - s1 * a[i1 + i];
                            ab2a = c2 * a[i2 + i] + s2 * b[i2 + i];
                            ab2b = c2 * b[i2 + i] - s2 * a[i2 + i];
                            ab3a = c3 * a[i3 + i] + s3 * b[i3 + i];
                            ab3b = c3 * b[i3 + i] - s3 * a[i3 + i];
                            ab4a = c4 * a[i4 + i] + s4 * b[i4 + i];
                            ab4b = c4 * b[i4 + i] - s4 * a[i4 + i];
                            ab5a = c5 * a[i5 + i] + s5 * b[i5 + i];
                            ab5b = c5 * b[i5 + i] - s5 * a[i5 + i];

                            a1p4 = ab1a + ab4a;
                            a1m4 = ab1a - ab4a;
                            a2p5 = ab2a + ab5a;
                            a2m5 = ab2a - ab5a;

                            b1p4 = ab1b + ab4b;
                            b1m4 = ab1b - ab4b;
                            b2p5 = ab2b + ab5b;
                            b2m5 = ab2b - ab5b;

                            ap05 = a[i0 + i] + ab3a - 0.5 * (a1p4 + a2p5);
                            bp05 = b[i0 + i] + ab3b - 0.5 * (b1p4 + b2p5);
                            am05 = a[i0 + i] - ab3a - 0.5 * (a2m5 - a1m4);
                            bm05 = -b[i0 + i] + ab3b - 0.5 * (b1m4 - b2m5);

                            ap60 = S60 * (a2p5 - a1p4);
                            bp60 = S60 * (b2p5 - b1p4);
                            am60 = S60 * (-a2m5 - a1m4);
                            bm60 = S60 * (-b2m5 - b1m4);

                            c[j0 + j] = a[i0 + i] + ab3a + a1p4 + a2p5;
                            d[j0 + j] = b[i0 + i] + ab3b + b1p4 + b2p5;
                            c[j1 + j] = am05 - bm60;
                            d[j1 + j] = am60 - bm05;
                            c[j2 + j] = ap05 - bp60;
                            d[j2 + j] = ap60 + bp05;
                            c[j3 + j] = a[i0 + i] - ab3a - a1m4 + a2m5;
                            d[j3 + j] = -b[i0 + i] + ab3b + b1m4 - b2m5;
                            c[j4 + j] = ap05 + bp60;
                            d[j4 + j] = ap60 - bp05;
                            c[j5 + j] = am05 + bm60;
                            d[j5 + j] = am60 + bm05;
                            i += inc3;
                            j += inc4;
                        }
                        ibase += 1;
                        jbase += 1;
                    }
                    j0 += jink;
                    j1 += jink;
                    j2 += jink;
                    j3 -= jink;
                    j4 -= jink;
                    j5 -= jink;
                    ibase += jump;
                }
                if (j2 > j3)
//		  return 0;
                    break;
            }
            jbase = 0;
            for (l = 0; l < la; ++l) {
                i = ibase;
                j = jbase;
#if defined (SX)
#pragma vdir nodep
#endif
                for (ijk = 0; ijk < lot; ++ijk) {
                    a1p5 = a[i1 + i] + a[i5 + i];
                    a1m5 = a[i1 + i] - a[i5 + i];
                    a2p4 = a[i2 + i] + a[i4 + i];
                    a2m4 = a[i2 + i] - a[i4 + i];

                    c[j0 + j] =  a[i0 + i] + 0.5 * a2m4 + S60 * a1m5;
                    d[j0 + j] = -a[i3 + i] - 0.5 * a1p5 - S60 * a2p4;
                    c[j1 + j] =  a[i0 + i] - a2m4;
                    d[j1 + j] =  a[i3 + i] - a1p5;
                    c[j2 + j] =  a[i0 + i] + 0.5 * a2m4 - S60 * a1m5;
                    d[j2 + j] = -a[i3 + i] - 0.5 * a1p5 + S60 * a2p4;
                    i += inc3;
                    j += inc4;
                }
                ibase += 1;
                jbase += 1;
            }
        } else {
            z = 1.0 / n;
            y = S60 / n;
            for (l = 0; l < la; ++l) {
                i = ibase;
                j = jbase;
#if defined (SX)
#pragma vdir nodep
#endif
                for (ijk = 0; ijk < lot; ++ijk) {
                    a0p3 = a[i0 + i] + a[i3 + i];
                    a0m3 = a[i0 + i] - a[i3 + i];
                    a1p4 = a[i1 + i] + a[i4 + i];
                    a1m4 = a[i1 + i] - a[i4 + i];
                    a2p5 = a[i2 + i] + a[i5 + i];
                    a2m5 = a[i2 + i] - a[i5 + i];

                    c[j0 + j] = z * (a0p3 + a1p4 + a2p5);
                    c[j3 + j] = z * (a0m3 + a2m5 - a1m4);

                    c[j1 + j] = z * (a0m3 - 0.5 * (a2m5 - a1m4));
                    c[j2 + j] = z * (a0p3 - 0.5 * (a1p4 + a2p5));

                    d[j1 + j] = y * (-a2m5 - a1m4);
                    d[j2 + j] = y * (a2p5 - a1p4);
                    i += inc3;
                    j += inc4;
                }
                ibase += 1;
                jbase += 1;
            }
        }
//	return 0;
        break;
    }

    case 8: {
        double a0p4, a1p5, a2p6, a3p7;
        double a0m4, a1m5, a2m6, a3m7;

        if (la != m)
//	  return 3;
            throw FactorHandled(la,factor,m);
        i0 = 0;
        i1 = i0 + iink;
        i2 = i1 + iink;
        i3 = i2 + iink;
        i4 = i3 + iink;
        i5 = i4 + iink;
        i6 = i5 + iink;
        i7 = i6 + iink;
        j0 = 0;
        j1 = j0 + jink;
        j2 = j1 + jink;
        j3 = j2 + jink;
        j4 = j3 + jink;
        j5 = j4 + jink;
        j6 = j5 + jink;
        j7 = j6 + jink;
        z = 1.0 / n;
        y = SQ2 / n;

        for (l = 0; l < la; ++l) {
            i = ibase;
            j = jbase;
#if defined (SX)
#pragma vdir nodep
#endif
            for (ijk = 0; ijk < lot; ++ijk) {
                a0p4 = a[i0 + i] + a[i4 + i];
                a0m4 = a[i0 + i] - a[i4 + i];
                a1p5 = a[i1 + i] + a[i5 + i];
                a1m5 = a[i1 + i] - a[i5 + i];
                a2p6 = a[i2 + i] + a[i6 + i];
                a2m6 = a[i2 + i] - a[i6 + i];
                a3p7 = a[i3 + i] + a[i7 + i];
                a3m7 = a[i3 + i] - a[i7 + i];

                c[j0 + j] =  z * (a0p4 + a1p5 + a2p6 + a3p7);
                c[j7 + j] =  z * (a0p4 - a1p5 + a2p6 - a3p7);

                c[j3 + j] =  z * (a0p4 - a2p6);
                c[j4 + j] =  z * (a3p7 - a1p5);

                c[j1 + j] =  z * a0m4 + y * (a1m5 - a3m7);
                c[j5 + j] =  z * a0m4 - y * (a1m5 - a3m7);
                c[j2 + j] = -z * a2m6 - y * (a1m5 + a3m7);
                c[j6 + j] =  z * a2m6 - y * (a1m5 + a3m7);
                i += inc3;
                j += inc4;
            }
            ibase++;
            jbase++;
        }
    }
    }
}


