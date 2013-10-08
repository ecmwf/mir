/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "LegendrePolynomials.h"

#ifndef Exception_H
#include "Exception.h"
#endif


inline double deg2rad(double x) { return x/180.0*M_PI; }
inline double rad2deg(double x) { return x*180.0/M_PI; }

LegendrePolynomials::LegendrePolynomials(int truncation):
	truncation_(truncation)
{
}

LegendrePolynomials::~LegendrePolynomials()
{
}

void LegendrePolynomials::calculateLegendrePoly(double* pleg, double* work, double lat) const
{

/* Calculate the legendre functions for one latitude(but not their derivatives)
   Recurrence relation with explicit relations for P(m,m) and P(m,m+1) */

  int itout1, i1m, ilm, jm, jcn, im2;
  double zsin, zcos, zf1m, zre1, zf2m, zn, ze1, ze2;
  double *zhlp1, *zhlp2, *zhlp3;

  /* Initialization */
	lat = deg2rad(lat);
	zsin = sin(lat);

  itout1 = truncation_ + 1;
  /*  zsin   = sin(plat); */
//  zsin   = plat;
  zcos   = sqrt(1.-zsin*zsin);

  zhlp1 = work;
  zhlp2 = work + itout1;
  zhlp3 = work + itout1 + itout1;

  /*  Step 1.        M = 0, N = 0 and N = 1 */

  ilm     = 1;
  pleg[0] = 1.0;
  zf1m    = sqrt(3.0);
  pleg[1] = zf1m*zsin;

  /*  Step 2.       Sum for M = 0 to T (T = truncation) */

  for ( jm = 1; jm < itout1; jm++ )
    {
      zhlp1[jm] = sqrt(2.*jm+3.);
      zhlp2[jm] = 1./sqrt(2.*jm);
    }

  zhlp1[0] = sqrt(3.);

  for ( jm = 0; jm < itout1; jm++ )
    {
      i1m  = jm - 1;
      zre1 = zhlp1[jm];
      ze1  = 1./zre1;

      /*   Step 3.       M > 0 only */

      if ( i1m != -1 )
	{
          zf2m = zf1m*zcos*zhlp2[jm];
          zf1m = zf2m*zre1;

	  /*  Step 4.       N = M and N = M+1 */

          ilm       = ilm+1;
          pleg[ilm] = zf2m;
          ilm       = ilm+1;
          pleg[ilm] = zf1m*zsin;

	  /* When output truncation is reached, return to calling program */

          if ( jm == (itout1-1) ) break;
	}

      /*  Step 5.       Sum for N = M+2 to T+1 */

      im2 = i1m+2;

      for ( jcn = im2; jcn < itout1; jcn++ )
	{
          zn         = jcn + 1;
	  zhlp3[jcn] = sqrt((4.*zn*zn-1.)/(zn*zn-jm*jm));
	}

      for ( jcn = im2; jcn < itout1; jcn++ )
	{
          ze2        = zhlp3[jcn];
          ilm        = ilm+1;
          pleg[ilm]  = ze2*(zsin*pleg[ilm-1]-ze1*pleg[ilm-2]);
          ze1        = 1./ze2;
	}
    }
}

void LegendrePolynomials::print(ostream& out) const
{
    out << "Legendre Polynomials Method is: ";
}

