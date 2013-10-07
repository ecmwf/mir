#ifndef MACHINE_H
#define MACHINE_H
/***************************** LICENSE START ***********************************

 Copyright 2012 ECMW. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include <iostream>
#include <string>
#include <iomanip>
#include <math.h>

using namespace std;

#include "mir_config.h"

static const double MISSING_VALUE = 9999.0;
static const long UNDEF = 9999;

/// AREA_FACTOR is added because GRIB has precision for 3 dec places.
/// For instance east for N640 is 359.8593750 intstead of 359.859
static const double AREA_FACTOR = 1.e-3;

static const long MULT = 100000;
static const long MMULT = 10000000;


static const double LSM_FACTOR  = 0.2;
static const double NINT_FACTOR = 0.5;

static const double NORTH_POLE = 90.0;
static const double SOUTH_POLE = -90.0;
static const double EQUATOR    = 360.0;

static const int CENTRAL_PARALEL    = 60;
static const int CENTRAL_MERIDIAN   = 0;

// Comparison
static const double ROUNDING_FACTOR = 1.e-10;
inline bool iszero(const double v) { return fabs(v) < ROUNDING_FACTOR; }
inline bool same(const double a, const double b) { return iszero(a-b); } 

// Macros
#define MIR_VERSION (MIR_MAJOR_VERSION*10000+MIR_MINOR_VERSION*100+MIR_REVISION_VERSION)

#ifndef NUMBER
#define NUMBER(x) (sizeof(x)/sizeof(x[0]))
#endif

// checks if env var is not equal to "0" or ""
#define IS_SET(x) (getenv(x) ? (string(getenv(x)).compare("0")!=0 && string(getenv(x)).length()>0 ) : false)

#define DEBUG IS_SET("MIR_DEBUG")
#define SHARED_DEBUG IS_SET("MIR_SHARED_DEBUG")

#define DUMP_NEAREST IS_SET("MIR_JUST_DUMP_NEAREST_POINTS")
#define CHECK_DUMP_NEAREST IS_SET("MIR_DUMP_NEAREST_POINTS")

#define EMOSLIB_PRECISION IS_SET("MIR_EMOSLIB_PRECISION")
#define EMOSLIB_MARS_AREA IS_SET("MIR_EMOSLIB_MARS_AREA")

#define SMALL_NUMBERS IS_SET("MIR_SMALL_NUMBERS")
#define DISABLE_LSM IS_SET("MIR_DISABLE_LSM")
#define GTOPO_TEMP IS_SET("MIR_GTOPO_TEMP")

#include <map>
#include <vector>
// for some machines is neccesary to include following
// for auto_ptr
#include <memory>
// for getenv
#include <stdio.h>
#include <stdlib.h>
typedef map<double,vector<double>,greater<double> > mapLats;

// Function to retrieve the shared path at runtime from the environment
// and failing that reverting to that defined at compile-time.
//
inline string getShareDir()
{
    char* pPath = getenv("MIR_SHARE_DIR");
    if (pPath)
    	return string(pPath);
    return string(MIR_SHARE_DIR);
}

inline string getDataDir()
{
    char* pPath = getenv("MIR_DATA_DIR");
    if (pPath)
        return string(pPath);
    return string(MIR_DATA_DIR);
}

static const int MAX_INCREMENTS_DISPLAY_PRECISION = 8;
#include <sstream>
inline string format_increments_value(double val)
{
    // single place where latitude increments are formatted such
    // that we don't get e.g. 0.16666 shown as 0.16667 in output streams

    stringstream ss;
    // set the precision such that we hold one more place
    // than we will ever wish to output
    ss.precision(MAX_INCREMENTS_DISPLAY_PRECISION+1);
    ss<<fixed;  // turn off scientific notation and output all values
    ss<< val;

    string all_digits = ss.str();
    // we now have a value e.g. 0.050000000 for input of 0.05
    // or 1.666666667 for input value calculated from e.g. 600/360

    // find the decimal point so we can limit precision 
    int pt = all_digits.find(".");

    // find last digit containing significant figure (non-zero)
    int sig=all_digits.find_last_of("123456789");

    // return substring limited to last significant figure or max display precision
    return all_digits.substr(0, min(sig+1, pt+MAX_INCREMENTS_DISPLAY_PRECISION+1));

}

#endif
